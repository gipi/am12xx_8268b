/*
 *  AL1601
 ****************************
 * Actions-micro PMU IC
 *
 * author: yekai
 * date: 2010-11-26
 * version: 0.1
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/miscdevice.h>


#include <asm/delay.h>
#include <am7x_i2c.h>
#include <am7x_pm.h>
#include <am7x_dev.h>
#include <sys_pmu.h>


/* AL1601 device address */
#define AL1601_ADDR		0x71

/* AL1601 registers */
#define CHARGE_STATUS   0x01
#define BAT_VOL_ADC     0x02
#define BAT_TEMP_ADC    0x06

#define ALVLDO_CON 		0x0D
#define DCDC0_CON1		0x0F
#define DCDC1_CON1		0x11
#define DCDC2_CON1		0x13
#define DCDC3_CON1		0x15
#define DCDC3_CON2		0x16
#define AUD_AMP_CON1	0x19

/* AL1601 voltage map */
#define AL1601_ALVLDO				ALVLDO_CON
#define AL1601_VCC					DCDC0_CON1
#define AL1601_VDD					DCDC1_CON1
#define AL1601_DRAM_VOL			DCDC2_CON1
#define AL1601_VBUS					DCDC3_CON1
#define AL1601_VBUS_CON			DCDC3_CON2
#define AL1601_AUDIO_CON1			AUD_AMP_CON1


static struct i2c_client *al1601_client;

static void al1601_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = al1601_client->addr;
	msg->flags = (al1601_client->flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}

/*  
* val[0] should be the reg address 
* the reg value will be stored from val[1] in return
*/
static int al1601_read_reg(__u8 *val, __u16 len)
{
	struct i2c_msg al1601_msg[2];
	
	al1601_set_msg(&al1601_msg[0],val,1,0);
	al1601_set_msg(&al1601_msg[1],val+1,len,I2C_M_RD);

	return i2c_transfer(al1601_client->adapter,al1601_msg,2);
}

/*
* val[0] should be the reg address
* val[1] should be the value set from val[0]
*/
static int al1601_write_reg(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg al1601_msg;

	al1601_set_msg(&al1601_msg, val, len, 0);
	ret = i2c_transfer(al1601_client->adapter,&al1601_msg,1);
	if(ret>0)
		udelay(1);
	
	return ret;
}

static int al1601_get_mapping(am_pm_type id, __u8 *reg)
{
	switch(id){
	case AM_VCC:
		*reg = AL1601_VCC;
		break;
	case AM_VDD:
		*reg = AL1601_VDD;
		break;
	case AM_DRAM:
		*reg = AL1601_DRAM_VOL;
		break;
	case AM_VBUS:
		*reg = AL1601_VBUS;
		break;
	case AM_VBUS_CON:
		*reg = AL1601_VBUS_CON;
		break;
	case AM_ALDO:
		*reg = AL1601_ALVLDO;
		break;
	case AM_AUDIO_CON:
		*reg = AL1601_AUDIO_CON1;
		break;
	case AM_CHARGE_STA:
		*reg = CHARGE_STATUS;
		break;
	case AM_BAT_VOL:
		*reg = BAT_VOL_ADC;
		break;
	case AM_BAT_TEMP:
		*reg = BAT_TEMP_ADC;
		break;
	default:
		return -EINVAL;
	}

	return 0;	
}

static int al1601_pm_ioctl(am_pm_type  id, unsigned int cmd, void  *arg)
{
	int ret = 0;

	if(cmd&AM_PM_NEEDS_NO_ARG){
		char tmp[4];
		switch(cmd){
		case AM_SYS_TO_VBUS_OFF:
		case AM_SYS_TO_VBUS_ON:
			al1601_get_mapping(AM_ALDO,tmp);
			ret = al1601_read_reg(tmp, 1);
			if(cmd==AM_SYS_TO_VBUS_OFF)
				tmp[1] &= 0x3f;
			else
				tmp[1] |= 0x80;
			ret = al1601_write_reg(tmp, 2);
			break;
		case AM_ENABLE_VBUS:
		case AM_DISABLE_VBUS:
			al1601_get_mapping(AM_VBUS_CON,tmp);
			ret = al1601_read_reg(tmp, 1);
			if(cmd==AM_ENABLE_VBUS)
				tmp[1] |= 0xc0;
			else
				tmp[1] = (tmp[1]&0x3f)|0x80;
			ret = al1601_write_reg(tmp, 2);
			break;
		case AM_ENABLE_AUD_AMP:
		case AM_DISABLE_AUD_AMP:
			al1601_get_mapping(AM_AUDIO_CON,tmp);
			ret = al1601_read_reg(tmp, 1);
			if(cmd==AM_ENABLE_AUD_AMP)
				tmp[1] |= 0x3;
			else
				tmp[1] = (tmp[1]&0xfc)|0x2;
			ret = al1601_write_reg(tmp, 2);
			break;
		case AM_PM_DISABLE:
		case AM_PM_ENABLE:
		default:
			break;
		}
	#if 1
		al1601_read_reg(tmp, 1);
		printk("set reg(%2x): %2x\n",tmp[0],tmp[1]);
	#endif
		goto END;
	}

	if(!arg){
		ret = -EINVAL;
		goto END;
	}else{
		ret = al1601_get_mapping(id, arg);
		if(ret)
			goto END;
	}
	
	switch(cmd){
	case AM_PM_GET:
		ret = al1601_read_reg(arg, 1);
		break;
	case AM_PM_SET:
		ret = al1601_write_reg(arg, 2);
		break;
	default:
		break;
	};

END:	
	if(ret<0)
		return ret;
	else
		return 0;
}

static int al1601_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	__u8 value[4];
	__u8 old=0;
	int ret = 0;
	printk(KERN_DEBUG"[AL]module probe!!!!\n");

	am_set_pm_ctl(al1601_pm_ioctl);

	/*set vdd 1.3V*/
	ret = al1601_pm_ioctl(AM_VDD, AM_PM_GET, &value[0]);
	if(ret)
		return ret;
	if(value[1] != 0x63){
		old = value[1];
		value[1] = 0x63;
		am_change_ddr_delay(((value[1]>>0x3)&0x7)*50+1100,((old>>0x3)&0x7)*50+1100);
		ret = al1601_pm_ioctl(AM_VDD,AM_PM_SET,&value[0]);
		if(ret)
			return ret;
		am_setup_watchdog(2);
	}
	
	/*set dvol 2.6v*/
	ret = al1601_pm_ioctl(AM_DRAM, AM_PM_GET, &value[0]);
	if(ret)
		return ret;

	if(value[1]!=0x23){
		value[1] = 0x23;
		ret = al1601_pm_ioctl(AM_DRAM,AM_PM_SET,&value[0]);
		if(ret)
			return ret;

	}

#if 0
	value[0]=AL1601_VBUS_CON;
	al1601_read_reg(&value[0], 1);	
	printk(KERN_DEBUG"vbus con=%x\n",value[1]);
	value[1]=0xc0;
	al1601_write_reg(&value[0], 2);
	
	am_pm_ioctl(AM_VDD, AM_PM_GET, &value[0]);
	printk("VDD=0x%x\n",value[1]);
	am_pm_ioctl(AM_DRAM, AM_PM_GET, &value[0]);
	printk("DRAM=0x%x\n",value[1]);
#endif
	
	return 0;
}

static int al1601_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AL]module remove\n");
	return 0;
}

static const struct i2c_device_id al1601_id[] = {
	{ "al1601", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, al1601_id);

static struct i2c_driver al1601_driver = {
	.driver = {
		.name = "al1601",
	},
	.probe = al1601_probe,
	.remove = al1601_remove,
	.id_table = al1601_id,
};

static struct i2c_board_info al1601_info = {I2C_BOARD_INFO("al1601", AL1601_ADDR)};


static 	int al1601_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval =0 ;
	struct al1601_data data;
	__u8 value[4];

	copy_from_user(&data,arg,sizeof(data));
	
	
	switch(cmd)
	{
		case READ_REG:
			value[0] = data.reg;
			retval=al1601_read_reg(value,1);
			data.val = value[1];
			break;
		case WRITE_REG:	
			value[0] = data.reg;
			value[1] = data.val;
			retval=al1601_write_reg(value,2);
			break;
		default:
			printk(KERN_ERR"unknown command\n");			
			break;
	}
	copy_to_user(arg,&data,sizeof(data));
	
		
	return retval;
}

static int al1601_open(struct inode *inode, struct file *file)
{

	return 0;
}

static struct file_operations al1601_fops ={
	.open = al1601_open,
	.ioctl = al1601_ioctl,
};


struct miscdevice alMisc = {
		.minor =	0,
		.name = 	"al1601",
		.fops = 	&al1601_fops,
};


static int __init al1601_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk(KERN_DEBUG"[AL]module init\n");
	adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	al1601_client = i2c_new_device(adap,&al1601_info);
	if(!al1601_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}

#if 1 //add 
	//if (register_chrdev(AL1601_MAJOR,"al1601",&al1601_fops))
	//	printk("unable to get major %d for memory devs\n", AL1601_MAJOR);
	


	misc_register(&alMisc);
	
#endif 	
	return i2c_add_driver(&al1601_driver);
}

static void __exit al1601_exit(void)
{
	i2c_del_driver(&al1601_driver);
	i2c_unregister_device(al1601_client);
	i2c_put_adapter(al1601_client->adapter);
	misc_deregister(&alMisc);
}

MODULE_AUTHOR("YeKai <yekai@actions-micro.com>");
MODULE_DESCRIPTION("AL1601 driver");
MODULE_LICENSE("GPL");

module_init(al1601_init);
module_exit(al1601_exit);
