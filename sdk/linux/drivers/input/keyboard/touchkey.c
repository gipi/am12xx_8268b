/**
 * drivers/input/keyboard/touchkey.c
 *
 *this driver is for touchkey device function add in input system
 *
 *author: hikwlu
 *date:2011-04-06
 *version:0.1
 */
 
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include "actions_regs.h"
#include "actions_io.h"
#include <sys_msg.h>
#include "sys_cfg.h"
#include <am7x_board.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/i2c.h>
#include <am7x_i2c.h>
#include "am7x_gpio.h"

#define 		SO381010_ADDR		0x2C
/* OneTouch Register Definitions */
#define 		OT_CONFIG_REG_START_ADDR		0x0000
#define 		OT_CONFIG_REG_END_ADDR			0x002B
#define 		OT_NUM_CONFIG_REG_BYTES			((OT_CONFIG_REG_END_ADDR - OT_CONFIG_REG_START_ADDR + 1)*2)
/* Total number of Configuration Bytes */
#define 		OT_NUM_CONFIG_BYTES				(OT_NUM_CONFIG_REG_BYTES + 2)

unsigned char config_data[OT_NUM_CONFIG_BYTES]= {
	0x00,0x00,
	0x00, 0x07,		// values of 0x0000 Interface Configuration
	0x00, 0x20,		// values of 0x0001 General Configuration
	0x00, 0x00,		// values of 0x0002 Strip Ballistics
	0x00, 0x00,		// values of 0x0003 //(0x00.0x00 50KHZ)   (0x02 ,0x84 42KHZ)  (0x02 ,0x84 42KHZ) (0x03 ,0x84 38KHZ)
	0x00, 0xff,		// values of 0x0004 button enable S1 S7
	0x00, 0x00,		// values of 0x0005
	0x00, 0x00,		// values of 0x0006
	0x00, 0x00,		// values of 0x0007
	0x00, 0x00,		// values of 0x0008 strip1 enable S2~S6
	0x00, 0x00,		// values of 0x0009
	0x00, 0x00,		// values of 0x000A
	0x00, 0x00,		// values of 0x000B
	0x00, 0x00,		// values of 0x000C
	0x00, 0x00,		// values of 0x000D
	0x7F, 0x7F,		// values of 0x000E GPIO Control /* GPIO0~GPIO6 */
	0x00, 0x00,		// values of 0x000F
	0xb8, 0xb8,		// values of 0x0010 s1/s0 sensivity
	0xb8, 0xb8,		// values of 0x0011 s3/s2 sensivity
	0xb8, 0xb8,		// values of 0x0012 s5/s4 sensivity
	0xb8, 0xb8,		// values of 0x0013 s7/s6 sensivity
	0x00, 0x00,		// values of 0x0014
	0x00, 0x00,		// values of 0x0015
	0x00, 0x00,		// values of 0x0016
	0x00, 0x00,		// values of 0x0017
	0x00, 0x00,		// values of 0x0018
	0x00, 0x00,		// values of 0x0019
	0x00, 0x00,		// values of 0x001A
	0x00, 0x00,		// values of 0x001B
	0x00, 0x00,		// values of 0x001C
	0x00, 0x00,		// values of 0x001D
	0x00, 0x00,		// values of 0x001E
	0x00, 0x00,		// values of 0x001F
	0x00, 0x00,		// values of 0x0020
	0x00, 0x00,		// values of 0x0021
	0x00, 0x7F,		// values of 0x0022 Reserved  Enable LED Control /* GPIO0~GPIO6 */
	0x50, 0x50,		// values of 0x0023 LED Effect PeriodA/B 3s 1unit=1/80 second
	0x9F, 0x9F,		// values of 0x0024 Effect1/Brightness1 Effect0/Brightness0
	0x9F, 0x9F,		// values of 0x0025 Effect3/Brightness3 Effect2/Brightness2
	0x9F, 0x9F,		// values of 0x0026 Effect5/Brightness5 Effect4/Brightness4
	0x00, 0x9F,		// values of 0x0027 Reserved            Effect6/Brightness6
	0x00, 0x00,		// values of 0x0028
	0x00, 0x00,		// values of 0x0029
	0x00, 0x00,		// values of 0x002A
	0x00, 0x00		// values of 0x002B Double Tap Enable ,Single TapSize 256
};

#define	OT_NUM_DATA_REG_BYTES				8		/* (0x106 - 0x109)*2 */
#define	OT_DATA_REG_START_ADDR_HIGH		0x01
#define	OT_DATA_REG_START_ADDR_LOW			0x08
#define	OT_DATA_REG_START_ADDR_HIGH_SLID	0x01
#define	OT_DATA_REG_START_ADDR_LOW_SLID	0x06

static struct i2c_adapter *hostadap=NULL;
static struct i2c_board_info so381010_info = {I2C_BOARD_INFO("so381010", SO381010_ADDR)};

static struct timer_list	t_timer;
static struct input_dev *touchkey_dev;

static unsigned char cur_key,save_key;
static unsigned char key_cnt=0;

static int i2c_host_init(void)
{
	struct i2c_adapter *adap=NULL;
	adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	hostadap = adap;
	printk(KERN_INFO"alloc i2c bus success\n");
	return 0;
}
static void set_host_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = so381010_info.addr;
	msg->flags = (so381010_info.flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}

static int so381010_read_packet(__u8 *val, __u16 len)
{
	struct i2c_msg msg;

	/*read data from slave*/
	set_host_msg(&msg, val, len, I2C_M_RD);
	
	return i2c_transfer(hostadap,&msg,1);
	
}

static int so381010_write_packet(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg msg;
	
	/*write data into slave*/
	set_host_msg(&msg, val, len, 0);
	ret = i2c_transfer(hostadap,&msg,1);
	if(ret>0)
		udelay(1);
	
	return ret;
}

static int so381010_touchic_init(void)
{
	int ret;
	int i;
	unsigned char check_data[OT_NUM_CONFIG_BYTES];
	unsigned char preg[2]={OT_DATA_REG_START_ADDR_HIGH,OT_DATA_REG_START_ADDR_LOW};
	unsigned char tmp[OT_NUM_DATA_REG_BYTES];
	
	memset(check_data,0,OT_NUM_CONFIG_BYTES);
	/*write config register,include device addr,register addr*/
	ret = so381010_write_packet(config_data, OT_NUM_CONFIG_BYTES);
	mdelay(5);
	
	/*check the config data to see whether it is trully be written*/
	ret = so381010_read_packet(check_data, OT_NUM_CONFIG_BYTES-2);
	mdelay(5);
	
	for(i=0;i<OT_NUM_CONFIG_BYTES-2;i++)
	{
		if((i%16)==0)
			printk("\n");
		printk("%2x ",check_data[i]);
	}
	printk("\n");
	if(ret==1){
		for(i=0;i<OT_NUM_CONFIG_BYTES-2;i++){
			if(check_data[i]!=config_data[i+2]){
				printk(KERN_ERR"config data check error\n");
				return -1;
			}
		}
	
	}
	else{
		printk(KERN_ERR"re-read config data less ,ret = %d\n",ret);
		return -1;
	}
	/*write the base data register addr, 
	*  so that host can read data from a default addr
	*/
	so381010_write_packet(preg, sizeof(preg));
	mdelay(5);
	
	/*check one byte read*/
	memset(tmp,0x00,sizeof(tmp));
	ret = so381010_read_packet(tmp, sizeof(tmp));
 	if(ret)
 	{
 		int j;
		for(j=0;j<8;j++)
			printk("%x ",tmp[j]);

		printk("\n");
 	}
	mdelay(5);
	
	return 0;
}
static int  get_touch_key(void)
{
	unsigned char key_value[OT_NUM_DATA_REG_BYTES];
	int ret;
	ret = so381010_read_packet(key_value, sizeof(key_value));
	if(ret)
	{
		return key_value[3];
	}
	return 0;
}

static void clear_touchkey_status(void)
{
	cur_key = 0;
	save_key = 0;
	key_cnt = 0;
}
static void touchkey_scan(void)
{
	static unsigned key_code;
	cur_key = 0;
	if(!am_get_gpio(31)){
		cur_key = get_touch_key();
		if(cur_key){
			switch(cur_key){
				case 0x01:
					key_code = 0x01;
				break;
				case 0x02:
					key_code = 0x02;
				break;
				case 0x04:
					key_code = 0x03;
				break;
				case 0x08:
					key_code = 0x04;
				break;
				case 0x10:
					key_code = 0x05;
				break;
				case 0x20:
					key_code = 0x06;
				break;
				case 0x40:
					key_code = 0x07;
				break;
				case 0x80:
					key_code = 0x08;
				break;
			}
			//printk(KERN_INFO"origin key value==%x\n",key_code);
			input_report_key(touchkey_dev, key_code, 1);
			input_report_key(touchkey_dev, key_code, 0);
			input_sync(touchkey_dev);
		}
			
	}
	
#if 0	//for touch debounce
	if(cur_key)
	{
		key_cnt++;
		if(key_cnt>2)
		{
			if(cur_key==save_key)
			{
				printk("key value==%x\n",cur_key);
			}
			else
			{
				save_key = cur_key;
			}
		}
		else
		{	
			save_key = cur_key;
		}
	}
	else
	{
		clear_touchkey_status();
	}
#endif

	mod_timer(&t_timer,jiffies +10);
}
static int __init touchkey_init(void)
{
	int ret;
	int i;
	i2c_host_init();

	ret = so381010_touchic_init();
	if(ret<0){
		printk(KERN_ERR"touch ic init fail\n");
		return -ENODEV;
	}
	
	touchkey_dev = input_allocate_device();
	touchkey_dev->name = "touchkey";
	touchkey_dev->phys = "AM/input1";
	touchkey_dev->id.bustype = BUS_HOST;
	touchkey_dev->id.vendor = 0x0002;
	touchkey_dev->id.product = 0x0001;
	touchkey_dev->id.version = 0x0100;

	touchkey_dev->evbit[0] = BIT_MASK(EV_KEY);
	
	ret = input_register_device(touchkey_dev);
	if (ret) {
		printk(KERN_ERR "input_allocate_device() failed for touch key\n");
		input_free_device(touchkey_dev);
		return ret;
	}

	for (i = 0; i < 8; i++) {
		set_bit(i, touchkey_dev->keybit);
	}
	
	init_timer(&t_timer);
	t_timer.expires = jiffies +10;
	t_timer.function = (void*)touchkey_scan;
	add_timer(&t_timer);
	return 0;
}


static void __exit touchkey_exit(void)
{
	del_timer(&t_timer);
	i2c_put_adapter(hostadap);
}

module_init(touchkey_init);
module_exit(touchkey_exit);


MODULE_AUTHOR("kewen <hikwlu@actions-micro.com>");
MODULE_DESCRIPTION("touchkey driver");
MODULE_LICENSE("GPL");

