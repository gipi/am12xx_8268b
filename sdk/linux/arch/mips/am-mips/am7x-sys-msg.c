/**
 * arch/mips/am-mips/am7x-sys-msg.c
 *
 *this driver is for system message op
 *
 *author: yekai
 *date:2010-03-12
 *version:0.1
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/major.h>
#include <linux/poll.h>
#include <sys_msg.h>

#define DBG_AM_MSG		0
#if  DBG_AM_MSG
#define DBG_SYS_MSG(format,args...)   printk(format,##args)
#else
#define DBG_SYS_MSG(format,args...)   do {} while (0)
#endif

#define SYSMSG_INV 		0
#define SYSMSG_INIT		1
#define SYSMSG_OPEN	2

struct am_sysmsg{
	INT16U	rp;				/**<read pointer*/
	INT16U	wp;				/**<write pointer*/
	INT16U	dep;				/**<message depth*/
	INT16S	flag;				/**<flag for msg device*/
	//struct semaphore sem;		/**<sem for msg device*/
	wait_queue_head_t wait;	/**<used for file->poll*/
};

struct device_notify_t{
	INT32U msg_mask;						/**<system message mask*/
	void (*msg_notify)(struct am_sys_msg *,void *);		/**<message handler*/
	void *private;								/**<private data for device*/
	struct list_head msg_device;						/**<device list*/
};


static struct am_sysmsg sysmsg={
	.rp = 0,
	.wp = 0,
	.dep = 0,
	.flag = 0,
};

static struct cdev  *sys_msg_cdev=NULL;
static dev_t  sys_msg_dev;
static struct am_sys_msg sys_msg[MAX_MSG_NUM];

static LIST_HEAD(dhead);

INT32S am_register_sysmsg(void (*p)(struct am_sys_msg*,void *), INT32U mask,void *pdata)
{
	struct device_notify_t *new = NULL;
	
	if(p==NULL||mask==0)
		return -EINVAL;

	/*kmalloc is suitable? any waste of memory?*/
	new = (struct device_notify_t *)kmalloc(sizeof(struct device_notify_t),GFP_ATOMIC);
	if(!new){
		printk(KERN_ERR"register device fail,no more space!\n");
		return -ENOSPC;
	}else{
		new->msg_mask = mask;
		new->msg_notify = p;
		new->private = pdata;
		list_add_tail(&(new->msg_device),&dhead);
	}
	
	return 0;
}

INT32S am_unregister_sysmsg(void (*handler)(struct am_sys_msg*,void *))
{
	struct device_notify_t *pos = NULL;

	if(!handler)
		return -EINVAL;
	list_for_each_entry(pos,&dhead,msg_device){
		if(pos->msg_notify == handler){
			list_del(&(pos->msg_device));
			kfree(pos);
			return 0;
		}
	}

	return -ENODEV;
}

INT32S am_put_sysmsg(struct am_sys_msg msg)
{
	if(sysmsg.dep<MAX_MSG_NUM){
		if(sysmsg.wp == MAX_MSG_NUM)
			sysmsg.wp = 0;
		sys_msg[sysmsg.wp++] = msg;
		sysmsg.dep++;
		DBG_SYS_MSG("put sysmsg %lx\n",msg.type);
		//up(&(sysmsg.sem));
	}else{
		return -ENOSPC;
	}

	if(sysmsg.flag==SYSMSG_OPEN){
		wake_up_interruptible(&sysmsg.wait);	//wake up poll
	}
	return 0;
}

static INT32S am_get_sysmsg(struct am_sys_msg *msgp )
{
	if(sysmsg.dep){
		if(sysmsg.rp == MAX_MSG_NUM)
			sysmsg.rp = 0;
		*msgp = sys_msg[sysmsg.rp++];
		sysmsg.dep--;
	}else{
		return -ENOMSG;
	}
	DBG_SYS_MSG("get sysmsg %lx\n",msgp->type);	
	return 0;
}

static ssize_t am_sysmsg_read(struct file *pfile, char __user *buf, size_t size, loff_t *offset)
{
	struct am_sys_msg msg;
	struct am_sysmsg* pmsg = pfile->private_data;
	
	if(size<sizeof(struct am_sys_msg))
		return 0;

	if(pfile->f_flags&O_NONBLOCK){
		if(!pmsg->dep)
			return -EAGAIN;
	}
	//down_interruptible(&pmsg->sem);
	wait_event_interruptible(pmsg->wait, pmsg->dep);
	am_get_sysmsg(&msg);

	if(copy_to_user(buf, &msg, sizeof(struct am_sys_msg)))
		return -EFAULT;
	
	return sizeof(struct am_sys_msg);
}

static ssize_t  am_sysmsg_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)
{
	struct am_sys_msg msg;
	struct device_notify_t *dev = NULL;

	if(copy_from_user(&msg,buf,sizeof(struct am_sys_msg)))
		return -EFAULT;

	list_for_each_entry(dev,&dhead,msg_device){
		if(dev->msg_mask&(msg.type-SYSMSG_MAGIC_NUMBER))
			dev->msg_notify(&msg,dev->private);
	}
	
	return 0;
}

static unsigned int am_sysmsg_poll(struct file *pfile, struct poll_table_struct *ptable)
{
	struct am_sysmsg* pmsg = pfile->private_data;

	DBG_SYS_MSG("[sysmsg] poll\n");
	poll_wait(pfile,&pmsg->wait,ptable);

	if(pmsg->dep)
		return POLLIN|POLLRDNORM;
	else
		return 0;
}

static int am_sysmsg_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long bits)
{
	return 0;
}

static int am_sysmsg_open(struct inode *pinode, struct file *pfile)
{
	DBG_SYS_MSG("am_sysmsg_open\n");
	if(sysmsg.flag==SYSMSG_INIT){
		init_waitqueue_head(&sysmsg.wait); 
		pfile->private_data = &sysmsg;
		sysmsg.flag = SYSMSG_OPEN;
	}else{
		return -EBUSY;
	}
	
	return 0;
}

static int am_sysmsg_release(struct inode *pinode, struct file *pfile)
{
	DBG_SYS_MSG("am_sysmsg_release\n");
	if(sysmsg.flag==SYSMSG_OPEN){
		sysmsg.flag = SYSMSG_INIT;
		pfile->private_data = NULL;
	}
	
	return 0;
}

static struct file_operations sysmsg_drv_fops=
{
	.owner  = THIS_MODULE,
	.read = am_sysmsg_read,
	.write = am_sysmsg_write,
	.poll = am_sysmsg_poll,
	.ioctl = am_sysmsg_ioctl,
	.open = am_sysmsg_open,
	.release = am_sysmsg_release,
};

static INT32S  __init am7x_sysmsg_init(void)
{
	INT32S result=0;

	DBG_SYS_MSG("*********am7x_sysmsg_init\n");

	#if 0
	result = alloc_chrdev_region(&sys_msg_dev, 0, SYSMSG_MAX_DEVS, "sysmsgdrv");
	#else
	sys_msg_dev =MKDEV(AM_SYSMSG_MAJOR,0);
	result = register_chrdev_region(sys_msg_dev,SYSMSG_MAX_DEVS,"sysmsgdrv");
	#endif
	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for sys msg\n");
		return -EIO;
	}
	DBG_SYS_MSG("sysmsg major=%d, minor=%d\n",MAJOR(sys_msg_dev),MINOR(sys_msg_dev));

	sys_msg_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!sys_msg_cdev){
		printk(KERN_ERR "malloc memory  fails for sys msg device\n");
		unregister_chrdev_region(sys_msg_dev,SYSMSG_MAX_DEVS);
		return -ENOMEM;
	}
  	cdev_init(sys_msg_cdev, &sysmsg_drv_fops);
	if(cdev_add(sys_msg_cdev, sys_msg_dev, 1))
		goto out_err;

	if(sysmsg.flag==SYSMSG_INV){
		//sema_init(&(sysmsg.sem),0);
		sysmsg.flag = SYSMSG_INIT;
	}
	
	return 0;
out_err:
	printk(KERN_ERR "register failed  for sysmsg device\n");
	kfree(sys_msg_cdev);
	unregister_chrdev_region(sys_msg_dev,SYSMSG_MAX_DEVS);
	return -ENODEV;

}

static void __exit am7x_sysmsg_exit(void)
{
	if(sys_msg_cdev){
		cdev_del(sys_msg_cdev);
		kfree(sys_msg_cdev);
	}
	unregister_chrdev_region(sys_msg_dev,SYSMSG_MAX_DEVS);
}

module_init(am7x_sysmsg_init);
module_exit(am7x_sysmsg_exit);

EXPORT_SYMBOL(am_put_sysmsg);
EXPORT_SYMBOL(am_register_sysmsg);
EXPORT_SYMBOL(am_unregister_sysmsg);

MODULE_AUTHOR("Ye Kai");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Actions-micro system message");
