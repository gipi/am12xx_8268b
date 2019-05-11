/*
 *  WM8988
 *@file I2c_wm8988.c
 *  I2c client Driver for send to wm8988 or read data form wm8988
 ****************************
 * Actions-micro DAC IC
 *
 * author: LiangRao
 * date: 2015-01-04
 * version: 0.1
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <sys_gpio.h>
#include <linux/delay.h>

#include <asm/delay.h>
#include <am7x_i2c.h>
#include <am7x_pm.h>
#include <am7x_dev.h>
#include <sys_pmu.h>
#include "i2c_wm8988.h"//<linux/i2c/i2c_wm8988.h.h>

/*wm8988 is not allow to read ,read will not get ack*/
/*wm8988 have two possible device address  CSB  state 0 low 0x34,1 high 0x36*/
#define CSB_STA 0

/* wm8988 device address */
#if CSB_STA 
#define WM8988_DEV_ADDR		0X1B	
#else 
#define WM8988_DEV_ADDR		0X1A
#endif

#define CONFIG_WM_PROC_FS 
#define MAX_WM_CURRENT	((__u16)3000)

/* wm8988  registers */
/* here's all the must remember stuff */
dev_t wm_dev;
int dev_count=1;
char *wm_name = "wm8988";
struct cdev *dev_cdev = NULL;
static struct i2c_client *wm8988_client;

#ifdef CONFIG_WM_PROC_FS
static struct proc_dir_entry *g_proc_entry = NULL;
#endif


struct class *g_wm_class = NULL;
/*am8253 ezmusci write wm8988 i2c to init it */
//extern int am_get_i2c_bus_id(void);
static int wm8988_write_reg(__u8 *val, __u16 len);

/*wm8988 R8(08h) 7bit address register for sample Rate
bit [8:7]default 00;
BCLK Frequency 
00 = BCM function disabled 
01 = MCLK/4 
10 = MCLK/8 
11 = MCLK/16 
bit[6] CLKDIV2 default:0:
1 = MCLK is divided by 2 
0 = MCLK is not divided 

bit[5:1]SR[4:0] sample Rate control
bit[0] usb default 0
Clocking Mode Select 
1 = USB Mode 
0 = ‘Normal’ Mode

*/
/*
 int am8253_set_samplerate(__u16 sample)
{
	int returnval;
	__u8 value[2];
	__u16 wm8988_sample,wm8988_sample_reg;
	wm8988_sample_reg=0x10;
	switch(sample)
	{
		//reg 01 0 00
	 	case ACT_SAMPLE_96K:
			//0 01 011100
			wm8988_sample=0x5C;
			printk("am8253 Sample[96K],set wm8988 R8 reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_48K:
			//0 10 000000
			wm8988_sample=0x80;
			printk("am8253 Sample[48K],set wm8988 R8  reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_44K:
			//0 10 100000
			wm8988_sample=0x80;
			printk("am8253 Sample[44K],set wm8988 R8 reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_32K:
			//010 011000
			wm8988_sample=0x98;
			printk("am8253 Sample[32K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_24K:
			//010 111000
			wm8988_sample=0xB8;
			printk("am8253 Sample[24K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_22K:
			//010 110100
			wm8988_sample=0xB4;
			printk("am8253 Sample[22K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_16K:
			//010 010100
			wm8988_sample=0x94;
			printk("am8253 Sample[16K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_12K:
			//010 010000
			wm8988_sample=0x90;
			printk("am8253 Sample[12K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_11K:
			//010 110000
			wm8988_sample=0xB0;
			printk("am8253 Sample[11K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		case ACT_SAMPLE_8K:
			//010 001100
			wm8988_sample=0x8C;
			printk("am8253 Sample[8K],set wm8988 R8reg=%X value=%x",wm8988_sample_reg,wm8988_sample);
			break;
		default:
			printk("unknow sample Rate!!!");

	}


	
	value[0] = wm8988_sample_reg;
	value[1]=wm8988_sample;
	returnval=wm8988_write_reg(value,2);
	return returnval;

}
*/
	void am_set_gpio(loff_t num,INT8U value)
	{
		volatile ssize_t*  reg=0;
	
		reg = (volatile ssize_t*)(GPIO_31_0INEN+(ssize_t)(num/32)*12);
		*reg = *reg&~(1<<(num%32));   // disable input
		reg = (volatile ssize_t*)(GPIO_31_0OUTEN+(ssize_t)(num/32)*12);
		*reg = *reg|(1<<(num%32));	 // enable output
	
		reg += 2;  // shift to data reg
		if(value)
			*reg = *reg|(1<<(num%32));
		else
			*reg = *reg&~(1<<(num%32));
	}

static void am8253init_wm8988()
{			

	int retval=0;
	__u8 value[6];

	printk("set i2S GPIO 1");
	/*
	1.set 55 gpio as pin  MFP2
	2.set output enable and value
	3.confirm Output data 
	*/
	
	//set bit [23~21]101
	printk("0.reg_readl(0xb01c0000+0x4c) =0X%X\n",am7x_readl(0xb01c0000+0x4c));	
	am7x_writel((am7x_readl(0xb01c0000+0x4c)|0x00a00000)&0xffbfffff,(0xb01c0000+0x4c));
	printk("1.reg_readl(0xb01c0000+0x4c) =0X%X",am7x_readl(0xb01c0000+0x4c));	
	
	am_set_gpio(55,1);
	printk("[63~32]GPIO data =0X%X",am7x_readl(0xb01c0000+0x0014));
	value[0] = 0x45;
	value[1] = 0x50;
	retval=wm8988_write_reg(value,2);
	
	value[0] = 0x4B;
	value[1] = 0x50;
	retval=wm8988_write_reg(value,2);
	
	value[0] = 0x05;
	value[1] = 0x6F;
	retval=wm8988_write_reg(value,2);
	
	value[0] = 0x07;
	value[1] = 0x6F;
	retval=wm8988_write_reg(value,2);
	
	value[0] = 0x0E;
	value[1] = 0x02;
	retval=wm8988_write_reg(value,2);
	
	value[0] = 0x32;
	value[1] = 0xC0;
	retval=wm8988_write_reg(value,2);
	value[0] = 0x35;
	value[1] = 0xE0;
	retval=wm8988_write_reg(value,2);
	value[0] = 0x0A;
	value[1] = 0x00;
	retval=wm8988_write_reg(value,2);

	
	/*set sample   rante  */

}
static void wm8988_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = wm8988_client->addr;
	printk(KERN_ERR"client->addr=0x%x   0x%x\n",wm8988_client->addr,*buf);
	msg->flags = (wm8988_client->flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}
/*  
wm8988 not allow to read 
when read:  turn to idle
* val[0] should be the reg address 
* the reg value will be stored from val[1] in return
*/
/*
static int wm8988_read_reg(__u8 *val, __u16 len)
{
	int ret;
	struct i2c_msg wm8988_msg[2];
	//write address first
	wm8988_set_msg(&wm8988_msg[0],val,2,0);
	ret = i2c_transfer(wm8988_client->adapter,wm8988_msg,1);
	udelay(50);
	wm8988_set_msg(&wm8988_msg[1],val+2,len,I2C_M_RD);
	ret = i2c_transfer(wm8988_client->adapter,wm8988_msg+1,1);
	if(ret>0)
		udelay(1);
	else
		{
			printk(KERN_ERR"read_wm_i2c error \n");
			return ret;
		}
	return ret;
}
*/
static int wm8988_write_reg(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg wm8988_msg;
	//data_len +address
	wm8988_set_msg(&wm8988_msg, val, len, 0);
	ret = i2c_transfer(wm8988_client->adapter,&wm8988_msg,1);
	if(ret>0)
		udelay(1);
	else
		{
			printk(KERN_ERR"write_wm_i2c error \n");
			return ret;
		}
	
	return ret;
}


static int wm8988_open(struct inode *inode, struct file *file)
{
	return 0;
}



static int wm8988_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval =0 ;
	struct wm8988_data data;
	__u8 value[6];
	__u32 sm_rate=am7x_readl(0xB0010000+0x08);
	memset(value, 0, 6);
	
	switch(cmd)
	{
		case AM8253_INIT_WM8988I2C:
			am8253init_wm8988();
		case WRITE_REG_WM:	
			copy_from_user(&data, (void *)arg,sizeof(data));
			value[0]=data.reg;
			value[1]=data.val;
			retval=wm8988_write_reg(value,2);
	
			break;
		case STE_WM_SMRATE:
			/*
			copy_from_user(&data, (void *)arg,sizeof(data));
			printk("AUDIOPLL=%x",sm_rate);
			sm_rate=sm_rate>>24&0xf;
			sm_rate=96/sm_rate;
			printk("AUDIOPLL sample rate =%d",sm_rate);
			break;
			*/
		case SET_WM_VOL_R:
			/*0111111 to 0000000*/
			copy_from_user(&data, (void *)arg,sizeof(data));
			value[0]=0x07;
			value[1]=data.val;
			retval=wm8988_write_reg(value,2);
		case SET_WM_VOL_L:
			/*0111111 to 0000000*/
			copy_from_user(&data, (void *)arg,sizeof(data));
			value[0]=0x05;
			value[1]=data.val;
			retval=wm8988_write_reg(value,2);
			
			
		default:
			printk(KERN_ERR"--unknown command\n");			
			break;
	}
	
	return retval;
}

/* VFS methods */
static struct file_operations wm8988_fops ={
	.open = wm8988_open,
	.ioctl = wm8988_ioctl,
};


 
static int wm8988_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int ret = 0;
	printk(KERN_INFO"[WM]module probe!!!!\n");	
	#if 0

	printk("set i2S GPIO 1");
	/*
	1.set 55 gpio as pin  MFP2
	2.set output enable and value
	3.confirm Output data 
	*/
	
	//set bit [23~21]101
	printk("0.reg_readl(0xb01c0000+0x4c) =0X%X\n",am7x_readl(0xb01c0000+0x4c));	
	am7x_writel((am7x_readl(0xb01c0000+0x4c)|0x00a00000)&0xffbfffff,(0xb01c0000+0x4c));
	printk("1.reg_readl(0xb01c0000+0x4c) =0X%X",am7x_readl(0xb01c0000+0x4c));	
	
	am_set_gpio(55,1);
	printk("[63~32]GPIO data =0X%X",am7x_readl(0xb01c0000+0x0014));
	#endif
	am8253init_wm8988();
	//am7x_writel((am7x_readl(0xb01c0000+0x4c)|0x00a00000)&0xffbfffff,(0xb01c0000+0x4c));
	//GPIO 55 Output enable
	//am7x_writel((am7x_readl(0xb01c0000+0x0c)|0x00800000),(0xb01c0000+0x0c));
	
	return ret;
}

static int wm8988_remove(struct i2c_client *client)
{
	
	return 0;
}

static const struct i2c_device_id wm8988_id[] = {
	{ "wm8988", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, wm8988_id);

static struct i2c_driver wm8988_driver = {
	.driver = {
	.name = "wm8988",
	},
	.probe = wm8988_probe,
	.remove = wm8988_remove,
	.id_table = wm8988_id,
};

static struct i2c_board_info wm8988_info = {I2C_BOARD_INFO("wm8988", WM8988_DEV_ADDR)};


#ifdef CONFIG_WM_PROC_FS
//file dir read and write
static int wm_proc_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
	int ret = 0;

	printk("[%s]\n", __func__);
	
	//__dpp_set_bright(900);
	
	return count;
}

static int wm_proc_read(char*buffer, char**buffer_localation, off_t offset,

                            int buffer_length,int* eof, void *data )
{
	int ret = 0;
	printk("[%s]\n", __func__);
	return ret;
}
#endif



static int __init wm8988_init(void)
{
	struct i2c_adapter *adap=NULL;
	int result = 0;
	int wm_major = 0;//MAJOR 243
	int wm_minor = 0;
	int bus_id=0;
	
	printk(KERN_INFO"[wm8988]module init\n");
	bus_id=am_get_i2c_bus_id();
	adap = i2c_get_adapter(bus_id);
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	wm8988_client = i2c_new_device(adap,&wm8988_info);
	if(!wm8988_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}

	#if 0
	if (register_chrdev(WM8988_MAJOR, "wm8988",&wm8988_fops))
		printk("unable to get major %d for memory devs\n", WM8988_MAJOR);

	#else
	/* if you want to test the module, you obviously need to "mknod". */
	if(wm_major)
	{
		wm_dev = MKDEV(wm_major, wm_minor);
		result = register_chrdev_region(wm_dev, dev_count, wm_name);
	}
	else
	{
		result = alloc_chrdev_region(&wm_dev, wm_major, dev_count, wm_name);
		wm_major = MAJOR(wm_dev);
	}
	printk("wm_major =%d\n", wm_major);

	dev_cdev = cdev_alloc();
	cdev_init(dev_cdev,&wm8988_fops);
	dev_cdev->owner=THIS_MODULE;
	//dev_cdev->ops=&dpp2607_fops;
	result = cdev_add(dev_cdev, wm_dev,dev_count);
	if(result < 0)
	{
		printk("can't add wm dev");
		unregister_chrdev_region(wm_dev, dev_count);
		return result;
	}

	g_wm_class = class_create(THIS_MODULE,"wm_class");
	if(IS_ERR(g_wm_class)) {
		unregister_chrdev_region(wm_dev, dev_count);	
		cdev_del(dev_cdev);
		printk("Err: failed in creating class.\n");
		return -1; 
	}

	device_create(g_wm_class, NULL, wm_dev, NULL, wm_name);
	

	printk("[wm8988]module init end\n");
	#endif


	#ifdef CONFIG_WM_PROC_FS
	g_proc_entry = create_proc_entry(wm_name, 0644, NULL);
	if(NULL != g_proc_entry){
		g_proc_entry->read_proc = wm_proc_read;
		g_proc_entry->write_proc = wm_proc_write;
		g_proc_entry->owner = THIS_MODULE;
	}
	#endif
	
	return i2c_add_driver(&wm8988_driver);
}

static void __exit wm8988_exit(void)
{
	printk("[wm]module remove\n");

	
	cdev_del(dev_cdev);

	device_destroy(g_wm_class, wm_dev);    //delete device node under /dev//必须先删除设备，再删除class类
	class_destroy(g_wm_class);                 //delete class created by us

	unregister_chrdev_region(wm_dev, dev_count);	
	
	i2c_del_driver(&wm8988_driver);
	i2c_unregister_device(wm8988_client);
	i2c_put_adapter(wm8988_client->adapter);

	#ifdef CONFIG_WM_PROC_FS
	remove_proc_entry(wm_name, g_proc_entry->parent);
	#endif
}


MODULE_AUTHOR("LiangRao <lemon.rao@actions-micro.com>");
MODULE_DESCRIPTION("wm8988 driver");
MODULE_LICENSE("GPL");

module_init(wm8988_init);
module_exit(wm8988_exit);
