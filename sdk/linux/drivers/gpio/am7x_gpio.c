/**
 * drivers/gpio/am7x_gpio.c
 *
 *this driver is for gpio op in user space
 *
 *author: yekai
 *date:2010-01-28
 *version:0.1
 *----------------------
 *modified on 2010-07-13
 *note: add dma test code because dma can only be tested in kernel mode 
 *and it has no interface for usr. So forgive these dirty codes :)
 *----------------------
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/major.h>
#include <actions_io.h>
#include <am7x_gpio.h>
#include <sys_gpio.h>
#include <am7x_board.h>

#define DBG_AM_GPIO		0
#define SUPPORT_DMA_TEST	0

#if DBG_AM_GPIO
#define DBG_MSG(format,args...)   printk(format,##args)
#else
#define DBG_MSG(format,args...)   do {} while (0)
#endif

struct cdev  *gpio_cdev=NULL;
dev_t  gpio_dev;

void am_set_gpio(loff_t num,INT8U value)
{
	volatile ssize_t*  reg=0;

	reg = (volatile ssize_t*)(GPIO_31_0INEN+(ssize_t)(num/32)*12);
	*reg = *reg&~(1<<(num%32));   // disable input
	reg = (volatile ssize_t*)(GPIO_31_0OUTEN+(ssize_t)(num/32)*12);
	*reg = *reg|(1<<(num%32));   // enable output

	reg += 2;  // shift to data reg
	if(value)
		*reg = *reg|(1<<(num%32));
	else
		*reg = *reg&~(1<<(num%32));
}

INT8U am_get_gpio(loff_t num)
{
	volatile ssize_t*  reg=0;

	reg = (volatile ssize_t*)(GPIO_31_0OUTEN+(ssize_t)(num/32)*12);
	*reg = *reg&~(1<<(num%32));   // disable output
	reg = (volatile ssize_t*)(GPIO_31_0INEN+(ssize_t)(num/32)*12);
	*reg = *reg|(1<<(num%32));   // enable input

	reg += 1;  // shift to data reg
	return (*reg&(1<<(num%32))?1:0);
}

static int am_gpio_init(void)
{
	int ret=0,i=0;
	struct am_gpio_config gpio_config;

	DBG_MSG("gpio init\n");
	ret = am_get_config(GPIO_CONFIG_FILE,(char *)&gpio_config,0,sizeof(struct am_gpio_config));
	/*for(i=0;i<GPIO_MFL_MAX;i++){
		printk("gpio_mfl%d = %8x\n",i,gpio_config.gpio_mfl[i]);
	}*/
	if(ret == sizeof(struct am_gpio_config)){
		ret = 0;
		for(i=0;i<GPIO_MFL_MAX;i++){
			//printk("gpio_mfl%d = %8x\n",i,am7x_readl(GPIO_MFCTL0+i*GPIO_MFL_INTER));
			am7x_writel(gpio_config.gpio_mfl[i],GPIO_MFCTL0+i*GPIO_MFL_INTER);
		}
		for(i=0;i<GPIO_IEN_MAX;i++){
			//printk("gpio_inen%d = %8x\n",i,am7x_readl(GPIO_31_0INEN+i*GPIO_IEN_INTER));
			am7x_writel(gpio_config.gpio_in_en[i],GPIO_31_0INEN+i*GPIO_IEN_INTER);
		}
		for(i=0;i<GPIO_OEN_MAX;i++){
			//printk("gpio_outen%d = %8x\n",i,am7x_readl(GPIO_31_0OUTEN+i*GPIO_OEN_INTER));
			am7x_writel(gpio_config.gpio_out_en[i],GPIO_31_0OUTEN+i*GPIO_OEN_INTER);
		}
	}

	ret = am_get_config(GPIO_CONFIG_FILE,(char*)&(get_sys_info()->sys_gpio_cfg),sizeof(struct am_gpio_config),sizeof(struct gpio_cfg));
	//printk("sd det=%d\n",get_sys_info()->sys_gpio_cfg.sd_det);
	//printk("usb det=%d\n",get_sys_info()->sys_gpio_cfg.usb_det);
	
	return ret;
}

static loff_t am_gpio_llseek(struct file *pfile, loff_t offset, int origin)
{
	loff_t pos=GPIO_BASE_N;

	switch(origin){
	case SEEK_SET:
		pos += offset;
		break;
	case SEEK_CUR: 
		pos = pfile->f_pos + offset;
		break;
	default:
		break;
		}
	pfile->f_pos = pos;
	
	return pos;
}

static ssize_t am_gpio_read(struct file *pfile, char __user *buf, size_t size, loff_t *offset)
{	
#ifdef sys_sem
	if(gpio_sem.flag)
		down(&(gpio_sem.sem));
#endif
	
	*buf = am_get_gpio(*offset);

	DBG_MSG("gpio%llu is read %d to %p\n",*offset, *buf,buf);
	//copy_to_user(buf,&value,1);
	
	return 1;
}

static ssize_t  am_gpio_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)
{
	//copy_from_user(&value,buf,1);
	am_set_gpio(*offset,*buf);

	DBG_MSG("gpio%llu is set %d\n",*offset,*buf);
	
	return 1;
}

#if SUPPORT_DMA_TEST
#include <sys_gpio.h>
#include <dma.h>

#define TEST_LOOP	1000
#define DMA_TEST_LEN	(1024*10)
static int dma_no=-1,int_count=0;
irqreturn_t dma_isr(unsigned int irq,struct irq_desc *desc)
{
	volatile unsigned int reg;

	reg = act_readl(INTC_PD);
	if(!(reg&(1<<irq)))
		printk("irq=%d,pending=%x,msk=%x\n",irq,reg,act_readl(INTC_MSK));
	reg = act_readl(DMA_IRQPD);
	if(!reg)
		printk("dma pending=%x,irqen=%x\n",reg,act_readl(DMA_IRQEN));

	am_clear_dma_irq(dma_no,1,0);
	int_count++;
	am_set_dma_irq(dma_no,1,0);
	return IRQ_HANDLED;
}

static int dma_test_entry(unsigned int type)
{
	int mode,ret=0,time_count=0,i=0;
	char *src, *des;

	dma_no = am_request_dma(type,NULL,(irq_handler_t)dma_isr,0,"dma_test");
	if(dma_no<0){
		printk("dma busy:%d\n",dma_no);
		return -EBUSY;
	}else{
		src = (char *)kmalloc(DMA_TEST_LEN,GFP_DMA);
		des = (char *)kmalloc(DMA_TEST_LEN,GFP_DMA);
		if((!src)||(!des)){
			printk("kmalloc fail\n");
			ret = -ENOSPC;
		}else{
			int_count = 0;
			for(i=0;i<TEST_LOOP;i++){
				am_dma_cmd(dma_no,DMA_CMD_RESET);
				mode = 0xa080a080;
				//printk("mode=%x,src=%x,des=%x\n",mode,(unsigned int)src,(unsigned int)des);
				am_set_dma_addr(dma_no, CPHYSADDR(src), CPHYSADDR(des));
				am_dma_config(dma_no, mode, DMA_TEST_LEN);
				am_set_dma_irq(dma_no,1,0);
				am_dma_start(dma_no, DMA_START_DEMOND);
				while(am_dma_cmd(dma_no,DMA_CMD_QUERY)){
					if(DMA_TIME_OUT<=time_count++)
						break;
				}
				if(time_count<DMA_TIME_OUT)
					time_count=0;
				else{
					am_dma_cmd(dma_no,DMA_CMD_RESET);
					ret = -EIO;
					break;
				}
			}
			printk("int nr %d in %d\n",int_count,TEST_LOOP);
		}
		am_free_dma(dma_no);
	}
	
	return ret;
}
#endif

static int am_soc_get_prio(struct am_bus_priority *pri)
{
#if CONFIG_AM_CHIP_ID == 1213

	volatile unsigned int value;

	if(pri == NULL){
		return -1;
	}

	value = act_readl(SDR_PRIORITY);
	pri->special_dma = (value>>28)&0xf;
	pri->mac = (value>>24)&0xf;
	pri->ahb_bus = (value>>20)&0xf;
	pri->graph_2d = (value>>16)&0xf;
	pri->de = (value>>12)&0xf;
	pri->dac = (value>>8)&0xf;
	pri->vdc = (value>>4)&0xf;
	pri->axi_bus = value&0xf;

	value = act_readl(SDR_PRIORITY2);
	pri->vec = value & 0xf;
	
	return 0;
	
#else

	return -1;

#endif	
}

static int am_soc_set_prio(struct am_bus_priority pri)
{
#if CONFIG_AM_CHIP_ID == 1213

	volatile unsigned int value;
	value = ((pri.special_dma & 0xf)<<28)| \
			((pri.mac & 0xf)<<24)| \
			((pri.ahb_bus & 0xf)<<20)| \
			((pri.graph_2d & 0xf)<<16)| \
			((pri.de & 0xf)<<12)| \
			((pri.dac & 0xf)<<8)| \
			((pri.vdc & 0xf)<<4)| \
			(pri.axi_bus & 0xf);
	act_writel(value, SDR_PRIORITY);

	value = pri.vec & 0xf;
	act_writel(value, SDR_PRIORITY2);

	return 0;
	
#else

	return -1;

#endif
}

static int am_gpio_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long bits)
{
	int ret=0;
	struct am_bus_priority pri;
	
#if SUPPORT_DMA_TEST
	ssize_t mask=0;
	
	mask =( ~mask)&bits ;
	switch(cmd){
	case GPIO_BITS_READ: 
		break;
	case GPIO_BITS_WRITE_AND:
		break;
	case GPIO_BITS_WRITE_OR:
		break;
	case 0x100:
		printk("start dma test,type = %lu\n",bits);
		ret = dma_test_entry((unsigned int)bits);
		break;
	default:
		break;
		}
#else
	switch(cmd){
	case GPIO_PAD_INIT:
		ret = am_gpio_init();
		break;

	case SOC_GET_PRIORITY:
		
		if(am_soc_get_prio(&pri)!=0){
			return -1;
		}

		if(copy_to_user((void*)bits, (void *)&pri, sizeof(struct am_bus_priority))){
			return -1;
		}

		return 0;
		
		break;

	case SOC_SET_PRIORITY:
		
		if(copy_from_user((void *)&pri, (void*)bits, sizeof(struct am_bus_priority))){
			return -1;
		}

		return am_soc_set_prio(pri);
		
		break;
		
	default:
		break;
	}
#endif
	return ret;
}

static int am_gpio_open(struct inode *pinode, struct file *pfile)
{
	DBG_MSG("am_gpio_open\n");
	if(!pfile)
		return -ENOENT;

	pfile->f_pos = 0;
	
#ifdef sys_sem
	if(gpio_sem.flag==0){
		sema_init(&(gpio_sem.sem),1);
		gpio_sem.flag = 1;
	}
#endif

	return 0;
}

static int am_gpio_release(struct inode *pinode, struct file *pfile)
{
	DBG_MSG("am_gpio_release\n");

	return 0;
}

static struct file_operations gpiodrv_fops=
{
	.owner  = THIS_MODULE,
	.llseek = am_gpio_llseek,
	.read = am_gpio_read,
	.write = am_gpio_write,
	.ioctl = am_gpio_ioctl,
	.open = am_gpio_open,
	.release = am_gpio_release,
};

static INT32S  __init am7x_gpio_init(void)
{
	INT32S result=0;

	DBG_MSG("*********am7x_gpio_init\n");

	//result = alloc_chrdev_region(&gpio_dev, 0, GPIO_MAX_DEVS, "gpiodrv");
	gpio_dev =MKDEV(AM_GPIO_MAJOR,0);
	result = register_chrdev_region(gpio_dev,GPIO_MAX_DEVS,"gpiodrv");
	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for gpio\n");
		return -EIO;
	}
	DBG_MSG("gpio major=%d, minor=%d\n",MAJOR(gpio_dev),MINOR(gpio_dev));

	gpio_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!gpio_cdev){
		printk(KERN_ERR "malloc memory  fails for gpio device\n");
		unregister_chrdev_region(gpio_dev,GPIO_MAX_DEVS);
		return -ENOMEM;
	}
  	cdev_init(gpio_cdev, &gpiodrv_fops);
	if(cdev_add(gpio_cdev, gpio_dev, 1))
		goto out_err;
	
	return 0;
out_err:
	printk(KERN_ERR "register failed  for gpio device\n");
	kfree(gpio_cdev);
	unregister_chrdev_region(gpio_dev,GPIO_MAX_DEVS);
	return -ENODEV;

}

static void __exit am7x_gpio_exit(void)
{
	if(gpio_cdev)
	{
		cdev_del(gpio_cdev);
		kfree(gpio_cdev);
	}
	unregister_chrdev_region(gpio_dev,GPIO_MAX_DEVS);
}

#if 0
INT32S am7x_gpio_mknod(void)
{
	if(sys_access("/dev",0)!=0)
		printk("access /dev error!\n");
	else
		printk("access /dev ok!\n");

	if(sys_mknod((const char __user *)"/dev/gpio",0666|S_IFCHR, new_encode_dev(gpio_dev)))
		printk("mknod for gpio fail!\n");
}
#endif

module_init(am7x_gpio_init);
module_exit(am7x_gpio_exit);

EXPORT_SYMBOL(am_get_gpio);
EXPORT_SYMBOL(am_set_gpio);

MODULE_AUTHOR("Ye Kai");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Actions-micro GPIO");
