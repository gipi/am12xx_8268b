/*
 *  BTVD
 ****************************
 * Actions-micro PMU IC
 *
 * author: yangli
 * date: 2010-11-26
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


/* BTVD device address */
#define BTVD_I2C_ADDR		0x5c//0x5d


#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))      


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

/* BTVD registers */

/* BTVD voltage map */

static struct i2c_client *btvd_i2c_client;

static void btvd_i2c_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = btvd_i2c_client->addr;
	msg->flags = (btvd_i2c_client->flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}

/*  
* val[0] should be the reg address 
* the reg value will be stored from val[1] in return
*/
int btvd_i2c_read_reg(__u8 *val, __u16 len)
{
	struct i2c_msg btvd_i2c_msg;
	int ret ; 
	btvd_i2c_set_msg(&btvd_i2c_msg,val,1,0);
	ret = i2c_transfer(btvd_i2c_client->adapter,&btvd_i2c_msg,1);
	if(ret<=0)
		return ret;

	btvd_i2c_set_msg(&btvd_i2c_msg,val+1,len,I2C_M_RD);
	return i2c_transfer(btvd_i2c_client->adapter,&btvd_i2c_msg,1);
}
EXPORT_SYMBOL(btvd_i2c_read_reg);
/*
* val[0] should be the reg address
* val[1] should be the value set from val[0]
*/
int btvd_i2c_write_reg(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg btvd_i2c_msg;

	btvd_i2c_set_msg(&btvd_i2c_msg, val, len, 0);
	ret = i2c_transfer(btvd_i2c_client->adapter,&btvd_i2c_msg,1);
	if(ret>0)
		udelay(1);
	
	return ret;
}
EXPORT_SYMBOL(btvd_i2c_write_reg);

static int btvd_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	__u8  tmp[2] = {0};
	int ret;
	printk(KERN_DEBUG"[AL]btvd module probe!!!!\n");

#if 0
	tmp[0] = 0x04;
	tmp[1] = 0xC0;
	ret = btvd_i2c_write_reg(tmp,2);

/*
	tmp[0] = 0xC1;
	tmp[1] =0x01;// 0x40;
	ret = btvd_i2c_write_reg(tmp,2);

	
	tmp[0] = 0x28;
	tmp[1] = 0x02;
	ret = btvd_i2c_write_reg(tmp,2);*/
	
	
	tmp[0] = 0x03;
	tmp[1] = 0x09;
	ret = btvd_i2c_write_reg(tmp,2);


#endif
	tmp[0] = 0x0f;
	tmp[1] = 0x0a;
	ret = btvd_i2c_write_reg(tmp,2);

	tmp[0] = 0x03;
	tmp[1] = 0x6d;
	ret = btvd_i2c_write_reg(tmp,2);

	
	tmp[0] = 0x15;
	tmp[1] = 0x25;
	ret = btvd_i2c_write_reg(tmp,2);

	tmp[0] = 0x0d;
	tmp[1] = 0x47;
	ret = btvd_i2c_write_reg(tmp,2);

	tmp[0] = 0x16;
	tmp[1] = 0x5f;
	ret = btvd_i2c_write_reg(tmp,2);

#if 0
	printk(KERN_ERR"btvd i2c +++++++++++++++++begin\n");
	tmp[0] = 0x03;
	ret = btvd_i2c_read_reg(tmp,1);
	if(ret<=0)
		goto test_err;
	
	printk(KERN_ERR"1 read 0x%x register,value is 0x%x\n\n",tmp[0],tmp[1]);
	
	tmp[0] = 0x03;
	tmp[1] = 0x81;
	ret = btvd_i2c_write_reg(tmp,2);
	if(ret <= 0)
		goto test_err;

	printk(KERN_ERR"2 write 0x%x register,value is 0x%x\n\n",tmp[0],tmp[1]);

	tmp[0] = 0x03;
	tmp[1] = 0x00;
	ret = btvd_i2c_read_reg(tmp,1);
	if(ret <= 0)
		goto test_err;

	printk(KERN_ERR"3 read 0x%x register,value is 0x%x\n\n",tmp[0],tmp[1]);

	printk(KERN_ERR"btvd i2c +++++++++++++++++end\n");
test_err:
#endif

	return 0;
}

static int btvd_i2c_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AL]module remove\n");
	return 0;
}

static const struct i2c_device_id btvd_i2c_id[] = {
	{ "btvd_i2c", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, btvd_i2c_id);

static struct i2c_driver btvd_i2c_driver = {
	.driver = {
		.name = "btvd_i2c",
	},
	.probe = btvd_i2c_probe,
	.remove = btvd_i2c_remove,
	.id_table = btvd_i2c_id,
};

static struct i2c_board_info btvd_i2c_info = {I2C_BOARD_INFO("btvd_i2c", BTVD_I2C_ADDR)};

static int __init btvd_i2c_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk(KERN_DEBUG"[AL]btvd module init\n");
	adap = i2c_get_adapter( am_get_i2c_bus_id() );
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	btvd_i2c_client = i2c_new_device(adap,&btvd_i2c_info);
	if(!btvd_i2c_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}
	
	return i2c_add_driver(&btvd_i2c_driver);
}

static void __exit btvd_i2c_exit(void)
{
	i2c_del_driver(&btvd_i2c_driver);
	i2c_unregister_device(btvd_i2c_client);
	i2c_put_adapter(btvd_i2c_client->adapter);
}

MODULE_AUTHOR("YangLi <yanglix@actions-micro.com>");
MODULE_DESCRIPTION("btvd_i2c driver");
MODULE_LICENSE("GPL");

module_init(btvd_i2c_init);
module_exit(btvd_i2c_exit);
