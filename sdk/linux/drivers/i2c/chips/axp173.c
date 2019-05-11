/*
 *  AXP173
 ****************************
 * PMU IC for am7x demo board
 *
 * author: yekai
 * date: 2010-08-23
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

/* axp173 device address */
#define AXP173_ADDR		0x34

/* axp173 registers */
#define INPUT_POWER_STATE			0x00
#define POWER_STATE				0x01
#define OTG_VBUS_STATE				0x04
#define DATA_BUFFER0				0x06
#define DATA_BUFFER1				0x07
#define DATA_BUFFER2				0x08
#define DATA_BUFFER3				0x09
#define DC_DC2_CON					0x10
#define DC1_LDO_CON				0x12
#define DC_DC2_DATA					0x23
#define DC_DC2_DCON				0x25
#define DC_DC1_DATA					0x26
#define LDO4_DATA					0x27
#define LDO2_LDO3_DATA				0x28
#define SHUT_DONW_CHARGE			0x32

/* axp173 voltage map */
#define AXP173_VCC					DC_DC1_DATA
#define AXP173_VDD					DC_DC2_DATA
#define AXP173_DRAM_VOL			LDO4_DATA

static struct i2c_client *axp173_client;

static void axp173_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = axp173_client->addr;
	msg->flags = (axp173_client->flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}

/*  
* val[0] should be the reg address 
* the reg value will be stored from val[1] in return
*/
static int axp173_read_reg(__u8 *val, __u16 len)
{
	struct i2c_msg axp173_msg[2];
	
	axp173_set_msg(&axp173_msg[0],val,1,0);
	axp173_set_msg(&axp173_msg[1],val+1,len,I2C_M_RD);

	return i2c_transfer(axp173_client->adapter,axp173_msg,2);
}

/*
* val[0] should be the reg address
* val[1] should be the value set from val[0]
*/
static int axp173_write_reg(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg axp173_msg;

	axp173_set_msg(&axp173_msg, val, len, 0);
	ret = i2c_transfer(axp173_client->adapter,&axp173_msg,1);
	if(ret>0)
		udelay(10);
	
	return ret;
}

static int axp173_get_mapping(am_pm_type id, __u8 *reg)
{
	switch(id){
	case AM_VCC:
		*reg = AXP173_VCC;
		break;
	case AM_VDD:
		*reg = AXP173_VDD;
		break;
	case AM_DRAM:
		*reg = AXP173_DRAM_VOL;
		break;
	default:
		return -EINVAL;
	}

	return 0;	
}

int axp173_pm_ioctl(am_pm_type  id, unsigned int cmd, void  *arg)
{
	int ret = 0;

	ret = axp173_get_mapping(id, arg);
	if(ret)
		goto END;
	
	switch(cmd){
	case AM_PM_GET:
		ret = axp173_read_reg(arg, 1);
		break;
	case AM_PM_SET:
		ret = axp173_write_reg(arg, 2);
		break;
	case AM_PM_DISABLE:
	case AM_PM_ENABLE:
	default:
		break;
	};

END:	
	if(ret<0)
		return ret;
	else
		return 0;
}

static int axp173_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	__u8 value[4];
	__u8 old=0;
	
	printk(KERN_DEBUG"[AXP]module probe!!!!\n");

	am_set_pm_ctl(axp173_pm_ioctl);

	/*set vdd 1.3v*/
	value[0] = DC_DC2_DATA;
	axp173_read_reg(&value[0],1);
	if(value[1] != 0x18){
		old = value[1];
		value[1] = 0x18;
		am_change_ddr_delay(value[1]*25+700,old*25+700);
		axp173_write_reg(&value[0],2);
		am_setup_watchdog(2);
		printk("vdd set 0x%x\n",value[1]);
	}
	
	/*set dram 2.6v*/
	value[0] = LDO4_DATA;
	axp173_read_reg(&value[0],1);
	if(value[1]!=0x4c){
		value[1] = 0x4c;//Value*25mV+700mV
		axp173_write_reg(&value[0],2);
		printk("dram set 0x%x\n",value[1]);
	}

#if 0
	value[0] = DC_DC2_DATA;
	axp173_read_reg(&value[0],1);
	printk("DC2=0x%x\n",value[1]);
	
	am_pm_ioctl(AM_DRAM, AM_PM_GET, &value[0]);
	printk("DRAM=0x%x\n",value[1]);
	value[1] = 0x14;
	am_pm_ioctl(AM_VDD, AM_PM_SET, &value[0]);
#endif	
	return 0;
}

static int axp173_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AXP]module remove\n");
	return 0;
}

/*
static int axp173_command(struct i2c_client *client,unsigned int cmd, void *arg)
{
	if(!client)
		client = axp173_client;
	
	switch(cmd){
	case I2C_SLAVE:
	case I2C_SLAVE_FORCE:
		client->addr = arg;
		break;
	case I2C_RDWR:
		axp173_rdwr_ioctl(client,arg);
		break;
	default:
		break;
	}
	
	return 0;
}
*/

static const struct i2c_device_id axp173_id[] = {
	{ "axp173", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, axp173_id);

static struct i2c_driver axp173_driver = {
	.driver = {
		.name = "axp173",
	},
	.probe = axp173_probe,
	.remove = axp173_remove,
	.id_table = axp173_id,
};

static struct i2c_board_info axp173_info = {I2C_BOARD_INFO("axp173", AXP173_ADDR)};

static int __init axp173_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk(KERN_DEBUG"[AXP]module init\n");
	adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	axp173_client = i2c_new_device(adap,&axp173_info);
	if(!axp173_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}
	
	return i2c_add_driver(&axp173_driver);
}

static void __exit axp173_exit(void)
{
	i2c_del_driver(&axp173_driver);
	i2c_unregister_device(axp173_client);
	i2c_put_adapter(axp173_client->adapter);
}

MODULE_AUTHOR("YeKai <yekai@actions-micro.com>");
MODULE_DESCRIPTION("AXP173 driver");
MODULE_LICENSE("GPL");

module_init(axp173_init);
module_exit(axp173_exit);
