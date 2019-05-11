/*
 *  I2S
 ****************************
 * Actions-micro PMU IC
 *
 * author: shixj
 * date: 2014-07-30
 * version: 0.1
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <asm/delay.h>
#include <am7x_i2c.h>
#include <am7x_pm.h>
#include <am7x_dev.h>

#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>


typedef struct
{
	unsigned char addr;		
	unsigned char data;			
}I2S_info;


/* i2s device address */
#define I2S_I2C_ADDR 			0x1B
#define Addr_AnalogPathControl 	0x4
#define Addr_DigitalPathControl 	0x5
#define Addr_PowerDownControl 	0x6
#define Addr_DigitalAudioFormat 	0x7
#define Addr_SampleRateControl 	0x8
#define Addr_DigithalActivation   	0x9
#define Addr_Reset			     	0xf
I2S_info I2S_Value[]=
{
    	{ Addr_Reset*2,0},
	{Addr_SampleRateControl*2,0x5C},
	{Addr_PowerDownControl*2,0x27},
	{Addr_AnalogPathControl*2,0x12},
	{Addr_DigitalPathControl*2,0x0},
	{Addr_DigitalAudioFormat*2,0xA},
	{Addr_DigithalActivation*2,0x1}
};




static struct i2c_client *i2s_i2c_client;

int i2s_write(struct i2c_adapter *adap)
{
#if 0
	AT24C02_WriteByte(0x1E,0);
	AT24C02_WriteByte(0x10,0x5C);
	AT24C02_WriteByte(0x0C,0x27);
	AT24C02_WriteByte(0x08,0x12);
	AT24C02_WriteByte(0x0A,0x0);
	AT24C02_WriteByte(0x0E,0xA);
	AT24C02_WriteByte(0x12,0x1);
#endif
	int i;
	struct i2c_msg msgs;
	msgs.addr= I2S_I2C_ADDR;
	msgs.flags=0;
	msgs.len	= 2;

	for(i=0;i<sizeof(I2S_Value)/sizeof(I2S_info);i++)
	{
		msgs.buf=&I2S_Value[i];
		
		if (i2c_transfer(adap, &msgs, 1) != 1)
		{
			printk("I2s %d,error \n",i);
			return 0;
		}
	}
	return 1;
}

int  i2s_i2c_write()
{
	return  i2s_write(i2s_i2c_client->adapter);
}
static int i2s_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	printk(KERN_DEBUG"[AL]i2s  module probe!!!!\n");
	return 0;
}

static int i2s_i2c_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AL]module remove\n");
	return 0;
}

static const struct i2c_device_id i2s_i2c_id[] = {
	{ "i2s_i2c", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, i2s_i2c_id);

static struct i2c_driver i2s_i2c_driver = {
	.driver = {
		.name = "i2s_i2c",
	},
	.probe = i2s_i2c_probe,
	.remove = i2s_i2c_remove,
	.id_table = i2s_i2c_id,
};

static struct i2c_board_info i2s_i2c_info = {I2C_BOARD_INFO("i2s_i2c", I2S_I2C_ADDR)};

static int __init i2s_i2c_hw_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk("i2s module init\n");
	adap = i2c_get_adapter( am_get_i2c_bus_id() );
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	i2s_i2c_client = i2c_new_device(adap,&i2s_i2c_info);
	if(!i2s_i2c_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}
	
	return i2c_add_driver(&i2s_i2c_driver);
}

static void __exit i2s_i2c_hw_exit(void)
{
	i2c_del_driver(&i2s_i2c_driver);
	i2c_unregister_device(i2s_i2c_client);
	i2c_put_adapter(i2s_i2c_client->adapter);
}
EXPORT_SYMBOL(i2s_i2c_write);

MODULE_AUTHOR("Sherral <shixj@actions-micro.com>");
MODULE_DESCRIPTION("i2s_i2c_hw driver");
MODULE_LICENSE("GPL");

module_init(i2s_i2c_hw_init);
module_exit(i2s_i2c_hw_exit);

