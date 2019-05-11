/*
 *  HDCP_DDC
 ****************************
 * Actions-micro PMU IC
 *
 * author: yangjy
 * date: 2013-01-21
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


/* HDCP device address */
#define HDCP_I2C_PRIMARY_ADDR		0x3A
#define HDCP_I2C_SECOND_ADDR		0x3B


/* HDCP registers */

/* HDCP voltage map */

static struct i2c_client *hdcp_i2c_client;

int hdcp_ddc_read(unsigned char offset, unsigned char r_len, unsigned char* read_buf)
{
	struct i2c_msg msg[2] = {
		{ .addr = HDCP_I2C_PRIMARY_ADDR, .flags = 0,        .buf = &offset, .len = 1 },
		{ .addr = HDCP_I2C_PRIMARY_ADDR, .flags = I2C_M_RD, .buf = read_buf,  .len = r_len },
	};
	if (i2c_transfer(hdcp_i2c_client->adapter, msg, 2) != 2) {
		printk(KERN_WARNING "hdcp I2C read failed\n");
		return 0;
	}
	return r_len;	
}

EXPORT_SYMBOL(hdcp_ddc_read);

int hdcp_ddc_write(unsigned char offset, unsigned char w_len, unsigned char* write_buf)
{
	unsigned char *buf = kmalloc(w_len+1, GFP_KERNEL);
	struct i2c_msg msg;

	if (!buf) {
		printk(KERN_WARNING "hdcp_ddc_write malloc error\n");
		return NULL;
	}
	*buf=offset;
	memcpy(buf+1,write_buf,sizeof(unsigned char)*w_len);
	
	 msg.addr = HDCP_I2C_PRIMARY_ADDR;
	 msg.flags = 0;
	 msg.buf = buf;
	 msg.len = w_len+1;

	if (i2c_transfer(hdcp_i2c_client->adapter, &msg, 1) != 1) {
		printk(KERN_WARNING "hdcp I2C write failed\n");
		return 0;
	}
	return w_len;
}
EXPORT_SYMBOL(hdcp_ddc_write);


static int hdcp_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	printk(KERN_DEBUG"[AL]hdcp ddc  module probe!!!!\n");
	return 0;
}

static int hdcp_i2c_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AL]module remove\n");
	return 0;
}

static const struct i2c_device_id hdcp_i2c_id[] = {
	{ "hdcp_i2c", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, hdcp_i2c_id);

static struct i2c_driver hdcp_i2c_driver = {
	.driver = {
		.name = "hdcp_i2c",
	},
	.probe = hdcp_i2c_probe,
	.remove = hdcp_i2c_remove,
	.id_table = hdcp_i2c_id,
};

static struct i2c_board_info hdcp_i2c_info = {I2C_BOARD_INFO("hdcp_i2c", HDCP_I2C_PRIMARY_ADDR)};

static int __init hdcp_i2c_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk(KERN_DEBUG"[AL]edid module init\n");
	adap = i2c_get_adapter( am_get_i2c_bus_id() );
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	hdcp_i2c_client = i2c_new_device(adap,&hdcp_i2c_info);
	if(!hdcp_i2c_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}
	
	return i2c_add_driver(&hdcp_i2c_driver);
}

static void __exit hdcp_i2c_exit(void)
{
	i2c_del_driver(&hdcp_i2c_driver);
	i2c_unregister_device(hdcp_i2c_client);
	i2c_put_adapter(hdcp_i2c_client->adapter);
}

MODULE_AUTHOR("YangJY <yangjy@actions-micro.com>");
MODULE_DESCRIPTION("hdcp_i2c driver");
MODULE_LICENSE("GPL");

module_init(hdcp_i2c_init);
module_exit(hdcp_i2c_exit);
