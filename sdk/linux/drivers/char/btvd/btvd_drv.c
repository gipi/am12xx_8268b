/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract : x170 Decoder device driver (kernel module)
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: hx170dec.c,v $
--  $Date: 2007/04/03 10:33:47 $
--  $Revision: 1.4 $
--
------------------------------------------------------------------------------*/

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_pfn_range
	SetPageReserved
	ClearPageReserved
*/
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>
/*for cdev_alloc()*/
#include <linux/cdev.h>
#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
//#include <linux/sched.h>
#include <linux/interrupt.h>
/* needed for virt_to_phys() */
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/major.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include "btvd_api.h"
#include "dma.h"

/* module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("AM MICRO Oy");
MODULE_DESCRIPTION("driver module for btvd");

/* here's all the must remember stuff */
dev_t dev;
char btvd_name[]="btvd";
int dev_count=1;
char* io_mem;
struct cdev *dev_cdev;
/* Logic module base address */
#define MODULE_BASE   0x100d0000	
#define MODULE_SIZE   0x1000


int dma_no=-1;
int frame_bytes=0;//%32==0
int frame_no=0;
#define irq_no                   30
#define FIFO_DEPTH	8
static BTVD_INFO btvd_info; 
static DECLARE_WAIT_QUEUE_HEAD(btvd_wq);
static char buf_ok = 0;




//0:special dma,1:cpu
#define MODE 0
//#define INQUIRE
#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))        

extern int btvd_i2c_write_reg(__u8 *val, __u16 len);
extern int btvd_i2c_read_reg(__u8 *val, __u16 len);


static void RegBitSet(int val,int reg,int msb,int lsb)                                            
{                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
}                                                                                                                                                                                                                                                                               
static unsigned int RegBitRead(int reg,int msb,int lsb)
{                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
}


static void btvd_config(BTVD_INFO * info)
{
	//RegBitSet(0x0,0xb0010084,8,8);
	//RegBitSet(0x1,0xb0010084,8,8);

	
	RegBitSet(0x0,0xb0010084,16,16);
	RegBitSet(0x1,0xb0010084,16,16);
	switch (info->video_format)
	{
	case BT656_480I:
		//act_writel(0x1020011,BTVD_VPEODD);	
		//act_writel(0x2090118,BTVD_VPEEVCS);
		act_writel( 0x01020011,BTVD_VPEODD);	
		act_writel(0x02090118,BTVD_VPEEVCS);
		//act_writel(0x1070017,BTVD_VPEODD);	
		//act_writel(0x271011e,BTVD_VPEEVCS);
		//act_writel(0x59f008a,BTVD_HDE);	
		act_writel(0x059f0000 ,BTVD_HDE);
		break;
		
	case BT656_576I:
//		act_writel(0x1320011,BTVD_VPEODD);	
//		act_writel(0x2390118,BTVD_VPEEVCS);
		act_writel(0x1370017,BTVD_VPEODD);	
		act_writel(0x2700150,BTVD_VPEEVCS);
		act_writel(0x59f0090,BTVD_HDE);		
		break;
	}
	#if(MODE==0)	
		RegBitSet(0xc8,BTVD_FIFOCTL,7,0);
	#else
	#ifdef INQUIRE
		RegBitSet(0x64,BTVD_FIFOCTL,7,0);
	#else
		//RegBitSet(0x74,BTVD_FIFOCTL,7,0);
		RegBitSet(0x28,BTVD_FIFOCTL,7,0);
	#endif
	#endif
	RegBitSet(FIFO_DEPTH,BTVD_FIFOCTL,13,8);
	act_writel(0xf,BTVD_IRQ);
	//act_writel(0x01,BTVD_CTL);
}
static void dma_config(BTVD_INFO * info,int dma_no)
{
	am_dma_cmd(dma_no, DMA_CMD_PAUSE);
	am_dma_cmd(dma_no,DMA_CMD_RESET);
	am_dma_config(dma_no,0x708061a8,frame_bytes);
	am_set_dma_weight(dma_no,am_get_dma_weight(dma_no)|0x10000000);	
	am_set_dma_irq(dma_no,1,0);
}


static void PadConfig(void)
{
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
//BTVD_CLKO
RegBitSet(1,0xb01c004c,11,10);
//BTVD_CLKI
RegBitSet(1,0xb01c004c,29,28);
//BTVD_VSYNC
RegBitSet(1,0xb01c004c,14,12);
//BTVD_HSYNC
RegBitSet(1,0xb01c004c,17,15);
//BTVD_D7
RegBitSet(1,0xb01c004c,20,18);
//BTVD_D6
RegBitSet(1,0xb01c004c,23,21);
//BTVD_D5
RegBitSet(1,0xb01c004c,25,24);
//BTVD_D4
RegBitSet(1,0xb01c004c,27,26);
//BTVD_D3/BTVD_D2/BTVD_D1/BTVD_D0
RegBitSet(1,0xb01c004c,29,28);

#else

//BTVD_CLKO[1]
RegBitSet(9,0xb01c0048,11,8);
//BTVD_CLKO[2]
RegBitSet(2,0xb01c0048,22,20);
//BTVD_CLKI
RegBitSet(2,0xb01c0058,22,20);
//BTVD_VSYNC
RegBitSet(2,0xb01c0058,13,12);
//BTVD_HSYNC
RegBitSet(2,0xb01c0058,18,16);

//BTVD_D7
RegBitSet(6,0xb01c0054,14,12);
//BTVD_D3
RegBitSet(6,0xb01c0054,18,16);
//BTVD_D1
RegBitSet(6,0xb01c0054,22,20);
//BTVD_D0
RegBitSet(6,0xb01c0054,26,24);
//BTVD_D5
RegBitSet(6,0xb01c005c,2,0);
//BTVD_D4
RegBitSet(6,0xb01c005c,6,4);
//BTVD_D6
RegBitSet(6,0xb01c0060,10,8);
//BTVD_D2
RegBitSet(6,0xb01c0060,14,12);
#endif
}
//#if(MODE==0)
#ifndef INQUIRE
static irqreturn_t btvd_isr(int irq, void *dev_id)
{
//	unsigned int handled = 0;
	int i;
	
	//printk("\n###btvd_isr##\n");
	for(i=0;i<8;i++)
		am_clear_dma_irq(dma_no, 1, 0);
	
#if 1
	
	if(BUF_BUSY == btvd_info.buf1.state){
		btvd_info.buf1.state = BUF_FULL;
		btvd_info.buf2.state = BUF_BUSY;
		am_set_dma_addr(dma_no,BTVD_FIFODAT,btvd_info.buf2.phy_buf&0x1fffffff);
	}else{
		btvd_info.buf2.state = BUF_FULL;
		btvd_info.buf1.state = BUF_BUSY;
		am_set_dma_addr(dma_no,BTVD_FIFODAT,btvd_info.buf1.phy_buf&0x1fffffff);
	}
	am_set_dma_irq(dma_no,1,0);
	buf_ok = 1;
	wake_up_interruptible(&btvd_wq);
#endif
	//dma_config(&btvd_info,dma_no);
	//am_set_dma_addr(dma_no,BTVD_FIFODAT,btvd_info.buf1.phy_buf&0x1fffffff);
	//RegBitSet(0x0,BTVD_FIFOCTL,3,3);
	//RegBitSet(0x1,BTVD_FIFOCTL,3,3);
	//am_dma_start(dma_no,0);	
		
	//btvd_config(&btvd_info);
		
	
//	printk(KERN_ERR"\n#########btvd_isr###\n");
	//am_dma_cmd(dma_no, DMA_CMD_START);

	return IRQ_RETVAL(1);
}


static int open_btvd(BTVD_INFO * info)
{
	
	PadConfig();
	switch(info->video_format)
	{
		case BT656_480I:
			frame_bytes=720*487*2;
			printk("\n NTSC \n");
			break;
		case BT656_576I:
			frame_bytes=720*576*2;
			break;
	}
	#if(MODE==0)
	dma_no = am_request_dma(DMA_CHAN_TYPE_SPECIAL, btvd_name,btvd_isr,0,(void *)0);
	#else
	dma_no = am_request_dma(DMA_CHAN_TYPE_BUS, btvd_name,btvd_isr,0,(void *)0);
	#endif
	if(dma_no <0 )
	{
		printk("btvd:request DMA failed\n");
		return -1;
	}

	dma_config(info,dma_no);
	info->buf1.state = BUF_BUSY;
	info->buf2.state = BUF_EMPTY;
	am_set_dma_addr(dma_no,BTVD_FIFODAT,info->buf1.phy_buf&0x1fffffff);
	am_dma_start(dma_no,0);		
	btvd_config(info);
	act_writel(0x1801,BTVD_CTL);

	return 0;
}
static int close_btvd(void)
{
	act_writel(0x0,BTVD_CTL);
	am_free_dma(dma_no);
	
	return 0;
}
#else

static irqreturn_t btvd_isr(int irq, void *dev_id)
{
	int i;
	static int count=0;
	int addr;
	int val;
	int offset=0;
	
	//printk("\n#########btvd_isr###\ncount=%d\n",count);
	for(i=0;i<8;i++)
		RegBitSet(0x4,BTVD_IRQ,3,0);
	
	addr=(btvd_info.buf1.phy_buf)|0xa0000000+count;
	for(i=0;i<FIFO_DEPTH-offset;i++)
	{
		val=act_readl(BTVD_FIFODAT);
		act_writel(val,addr+4*i);
	}
	count=count+(FIFO_DEPTH-offset)*4;
	if(count==frame_bytes)
	{
		printk("\n one field end\n");
		count=0;
		frame_no++;
	}

	
/*	if(BUF_BUSY == btvd_info.buf1.state){
		btvd_info.buf1.state = BUF_FULL;
		btvd_info.buf2.state = BUF_BUSY;
		am_set_dma_addr(dma_no,BTVD_FIFODAT,btvd_info.buf2.phy_buf&0x1fffffff);
	}else{
		btvd_info.buf2.state = BUF_FULL;
		btvd_info.buf1.state = BUF_BUSY;
		am_set_dma_addr(dma_no,BTVD_FIFODAT,btvd_info.buf1.phy_buf&0x1fffffff);
	}*/		
		
	buf_ok = 1;
	//wake_up_interruptible(&btvd_wq);

	return IRQ_RETVAL(1);
}

static int open_btvd(BTVD_INFO * info)
{
	PadConfig();
	switch(info->video_format)
	{
		case BT656_480I:
			frame_bytes=720*480*2;
			break;
		case BT656_576I:
			frame_bytes=720*576*2;
			break;
	}
	info->buf1.state = BUF_BUSY;
	info->buf2.state = BUF_EMPTY;
	btvd_config(info);
	act_writel(0x1801,BTVD_CTL);
#ifdef INQUIRE
	while(1)
	{
		if(RegBitRead(BTVD_IRQ,2,2)==1)
			btvd_isr(0,NULL);
		if(frame_no==2)
			break;
	}
#endif
	return 0;
}
static int close_btvd(void)
{
	act_writel(0x0,BTVD_CTL);
	return 0;
}
#endif

/*------------------------------------------------------------------------------
    Function name   : hx170dec_ioctl
    Description     : communication method to/from the user space

    Return type     : int
------------------------------------------------------------------------------*/
static int btvd_ioctl(struct inode *inode, struct file *filp,
                          unsigned int cmd, unsigned long arg)
{
    int err = 0;
	int rtn = 0;
	struct btvd_data data;
	__u8 value[2];
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if(_IOC_TYPE(cmd) != BTVD_IOC_MAGIC)
        return -ENOTTY;

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if(_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));
    if(err)
        return -EFAULT;
    switch (cmd)
    {
    case BTVD_IOCSOPEN:
		buf_ok = 0;
		copy_from_user(&btvd_info, (void *)arg, sizeof(BTVD_INFO));
		open_btvd(&btvd_info);
        break;
    case BTVD_IOCTCLOSE:
		close_btvd();
        break;

	case BTVD_GET_IMG:
		WAIT_BUF:
		//wait_event_interruptible(btvd_wq, buf_ok == 1);
		if(buf_ok != 1){
			rtn = wait_event_interruptible_timeout(btvd_wq, buf_ok == 1, HZ/5);//200ms
			if(rtn < 0){
				if(buf_ok != 1)
					goto WAIT_BUF;
			}
		}
		buf_ok = 0;
		copy_to_user((void *)arg, &btvd_info, sizeof(BTVD_INFO));
		break;
		
	case BTVD_IO_R_REG:
			copy_from_user(&data, (void *)arg, sizeof(struct btvd_data));
			value[0] = data.reg;
			err= btvd_i2c_read_reg(value,1);
			data.val = value[1];
			copy_to_user((void *)arg, &data, sizeof(struct btvd_data));
			break;
	case BTVD_IO_W_REG:	
			copy_from_user(&data, (void *)arg, sizeof(struct btvd_data));
			value[0] = data.reg;
			value[1] = data.val;
			err= btvd_i2c_write_reg(value,2);
			break;
	
	default:
		break;
    }
    return err;
}



/*------------------------------------------------------------------------------
    Function name   : hx170dec_open
    Description     : open method

    Return type     : int
------------------------------------------------------------------------------*/

static int btvd_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/* VFS methods */
static struct file_operations btvd_fops = {
	open:btvd_open,
	ioctl:btvd_ioctl,
};

/*------------------------------------------------------------------------------
    Function name   : releaseIO
    Description     : release

    Return type     : void
------------------------------------------------------------------------------*/

static void ReleaseIO(void)
{
    if(io_mem!=NULL)
        iounmap((void *) io_mem);
    release_mem_region(MODULE_BASE, MODULE_SIZE);
}


/*------------------------------------------------------------------------------
    Function name   : ReserveIO
    Description     : IO reserve

    Return type     : int
------------------------------------------------------------------------------*/
static int ReserveIO(void)
{

    if(request_mem_region(MODULE_BASE, MODULE_SIZE, btvd_name)==NULL)
    {
        printk("btvd: failed to reserve HW regs\n");
        return -EBUSY;
    }
     io_mem =(char*) ioremap_nocache(MODULE_BASE,MODULE_SIZE);
     if(io_mem==NULL)
     {
     		ReleaseIO();
     		printk("btvd: failed to ioremap HW regs\n");
     }
    return 0;
}
/*------------------------------------------------------------------------------
    Function name   : hx170dec_init
    Description     : Initialize the driver

    Return type     : int
------------------------------------------------------------------------------*/
static int init_btvd(void)
{
	int result;
	/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/
	int btvd_major=AM_BTVD_MAJOR;
	int btvd_minor=0;

	/* if you want to test the module, you obviously need to "mknod". */
	if(btvd_major)
	{
		printk("btvd_major=%d\n",btvd_major);
		dev=MKDEV(btvd_major,btvd_minor);
		result=register_chrdev_region(dev,dev_count,btvd_name);
	}
	else
	{
		result=alloc_chrdev_region(&dev,btvd_minor,dev_count,btvd_name);
		btvd_major=MAJOR(dev);
	}
	
	if(result<0)
	{
		printk("can't get btvd major\n");
		return result;
	}
	result = ReserveIO();
	if(result<0)
	{
		printk("can't get btvd reg mem\n");
		return result;
	}

	dev_cdev=cdev_alloc();
	cdev_init(dev_cdev,&btvd_fops);
	dev_cdev->owner=THIS_MODULE;
	dev_cdev->ops=&btvd_fops;
	result = cdev_add(dev_cdev,dev,dev_count);
	if(result < 0)
	{
		printk("can't add btvd dev");
		unregister_chrdev_region(dev,dev_count);
		return result;
	}
	RegBitSet(1,0xb0010080,30,30);
	#if(MODE==1)
	result = request_irq(irq_no, btvd_isr,IRQF_DISABLED, "btvd",NULL);
	if(result == -EINVAL)
	{
		printk(KERN_ERR "graph: Bad irq number or handler\n");
		return result;
	}
	else if(result == -EBUSY)
	{
		printk(KERN_ERR "graph: IRQ %d busy, change your config\n",irq_no);
		return result;
	}
	#endif
	return 0;
}


/*------------------------------------------------------------------------------
    Function name   : hx170dec_cleanup
    Description     : clean up

    Return type     : int
------------------------------------------------------------------------------*/
static int release_btvd(void)
{
	unregister_chrdev_region(dev,dev_count);		
	ReleaseIO();
	/* free the encoder IRQ */
	cdev_del(dev_cdev);
	#if(MODE==1)
	free_irq(irq_no, (void *) 0);
	#endif
	printk( "btvd_cleanup\n");
	
	return 0;
}


/**
* Register  module to platform bus to do power management.
*/

#define AM7X_BTVD_MODULE_NAME "am7x-btvd"


int am7x_btvd_probe(struct platform_device * pdev)
{
	int result;

	result = init_btvd();

	return result;
	
}


int am7x_btvd_remove(struct platform_device * pdev)
{
	release_btvd();
	return 0;
}

int am7x_btvd_suspend(struct platform_device *pdev, pm_message_t state)
{
	/**
	* Please fix me.
	*/
	RegBitSet(0,0xb0010080,30,30);
	return 0;
}

int am7x_btvd_resume(struct platform_device *pdev)
{
	/**
	* Please fix me.
	*/
	RegBitSet(1,0xb0010080,30,30);
	return 0;
}


static struct platform_driver am7x_driver_btvd = {
	.probe = am7x_btvd_probe,
	.remove = am7x_btvd_remove,
	.suspend = am7x_btvd_suspend,
	.resume = am7x_btvd_resume,
	.driver = {
		.name = AM7X_BTVD_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static struct resource btvd_resource[] = {
	[0]	= {
		.start	= KVA_TO_PA(MODULE_BASE),
		.end	= KVA_TO_PA(MODULE_BASE) + MODULE_SIZE,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device am7x_device_btvd = {
	.name		= AM7X_BTVD_MODULE_NAME,
	.id		= 1,
	.num_resources	= ARRAY_SIZE(btvd_resource),
	.resource	= btvd_resource,
};

int __init btvd_init(void)
{
	int err;
	
	err = platform_driver_register(&am7x_driver_btvd);
	if(!err){
		err = platform_device_register(&am7x_device_btvd);
	}

	return err;
}

void __exit btvd_cleanup(void)
{
	platform_driver_unregister(&am7x_driver_btvd);
	platform_device_unregister(&am7x_device_btvd);
	
	return;
}

module_init(btvd_init);
module_exit(btvd_cleanup);








