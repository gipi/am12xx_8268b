/*
 *  EDID
 ****************************
 * Actions-micro PMU IC
 *
 * author: yangjy
 * date: 2012-12-04
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


/* EDID device address */
#define EDID_I2C_ADDR				0x50
#define EDID_LENGTH					0x80
#define EDID_I2C_SEGMENT_ADDR		0x30

/* EDID registers */

/* EDID voltage map */

static struct i2c_client *edid_i2c_client;

static unsigned char *do_probe_ddc_edid(struct i2c_adapter *adapter)
{
	unsigned char start = 0x0;
	unsigned char extension_flag = 0;
	unsigned char i;
	//int i = 0;
	unsigned char *buf = kmalloc(EDID_LENGTH, GFP_KERNEL);
	//unsigned char *buf_data= kmalloc(EDID_LENGTH, GFP_KERNEL);
	struct i2c_msg msgs[] = {
		{
			.addr	= EDID_I2C_ADDR,
			.flags	= 0,
			.len	= 1,
			.buf	= &start,
		}, {
			.addr	= EDID_I2C_ADDR,
			.flags	= I2C_M_RD,
			.len	= EDID_LENGTH,
			.buf	= buf,
		}
	};


	if (!buf) {
		dev_warn(&adapter->dev, "unable to allocate memory for EDID "
			 "block.\n");
		return NULL;
	}

	if (i2c_transfer(adapter, msgs, 2) == 2)
	{
		extension_flag = buf[126];
		if (extension_flag>=1)
		{
			printk("extension_flag=%d\n",extension_flag);
			unsigned char *buf_extension = kmalloc((EDID_LENGTH*(extension_flag+1)), GFP_KERNEL);
			if (!buf_extension)
			{
				dev_warn(&adapter->dev, "unable to allocate memory for EDID extension "
					 "block.\n");
				return buf;
			}
			if (extension_flag ==1)
			{
				start = 0x00;
				msgs[0].addr	= EDID_I2C_ADDR;
				msgs[0].flags	= 0;
				msgs[0].len		= 1;
				msgs[0].buf		= &start;
				msgs[1].addr	= EDID_I2C_ADDR;
				msgs[1].flags	= I2C_M_RD;
				msgs[1].len		= EDID_LENGTH*2;
				msgs[1].buf = buf_extension;
				if (i2c_transfer(adapter, msgs, 2) == 2)
				{
					printk("read edid block1 ok\n");
					kfree(buf);
					return buf_extension;					
				}
				else
				{
					printk("read edid block1 error\n");
					kfree(buf_extension);					
					return buf;
				}				
			}
			else
			{
				for (i=0;i<=extension_flag/2;i++)
				{
					start = i;
					msgs[0].addr	= EDID_I2C_SEGMENT_ADDR;
					msgs[0].flags	= 0;
					msgs[0].len		= 1;
					msgs[0].buf		= &start;
					if (i2c_transfer(adapter, msgs, 1) == 1)
					{
						start = 0x0;
						msgs[0].addr	= EDID_I2C_ADDR;
						msgs[0].flags	= 0;
						msgs[0].len		= 1;
						msgs[0].buf		= &start;
						msgs[1].addr	= EDID_I2C_ADDR;
						msgs[1].flags	= I2C_M_RD;
						msgs[1].len		= EDID_LENGTH*2;
						msgs[1].buf = buf_extension+i*EDID_LENGTH*2;
						if (i2c_transfer(adapter, msgs, 2) == 2)
						{
							printk("read edid segment[%d] ok\n",i);
						}
						else
						{
							printk("read edid segment[%d] data error\n",i);
							kfree(buf_extension);					
							return buf;
						}
					}
					else
					{
						printk("read edid segment error\n");
						kfree(buf_extension);					
						return buf;
					}
				}
				kfree(buf);
				return buf_extension;
			}
		}
		return buf;		
	}

	dev_warn(&adapter->dev, "unable to read EDID block.\n");
	kfree(buf);
	return NULL;
}

unsigned char *edid_read(struct i2c_adapter *adapter)
{
	//struct i2c_algo_bit_data *algo_data = adapter->algo_data;
	unsigned char *edid = NULL;
	int i, j;

	//algo_data->setscl(algo_data->data, 1);

	for (i = 0; i < 3; i++) {
		/* For some old monitors we need the
		 * following process to initialize/stop DDC
		 */
		/*algo_data->setsda(algo_data->data, 1);
		msleep(13);

		algo_data->setscl(algo_data->data, 1);
		for (j = 0; j < 5; j++) {
			msleep(10);
			if (algo_data->getscl(algo_data->data))
				break;
		}
		if (j == 5)
			continue;

		algo_data->setsda(algo_data->data, 0);
		msleep(15);
		algo_data->setscl(algo_data->data, 0);
		msleep(15);
		algo_data->setsda(algo_data->data, 1);
		msleep(15);*/

		/* Do the real work */
		edid = do_probe_ddc_edid(adapter);
		/*algo_data->setsda(algo_data->data, 0);
		algo_data->setscl(algo_data->data, 0);
		msleep(15);

		algo_data->setscl(algo_data->data, 1);
		for (j = 0; j < 10; j++) {
			msleep(10);
			if (algo_data->getscl(algo_data->data))
				break;
		}

		algo_data->setsda(algo_data->data, 1);
		msleep(15);
		algo_data->setscl(algo_data->data, 0);
		algo_data->setsda(algo_data->data, 0);*/
		if (edid)
			break;
	}
	/* Release the DDC lines when done or the Apple Cinema HD display
	 * will switch off
	 */
	//algo_data->setsda(algo_data->data, 1);
	//algo_data->setscl(algo_data->data, 1);

	adapter->class |= I2C_CLASS_DDC;
	return edid;
}

unsigned char * edid_i2c_hw_read()
{
	unsigned char *edid = NULL;
	edid = edid_read(edid_i2c_client->adapter);
	return edid;	
}

EXPORT_SYMBOL(edid_i2c_hw_read);


static int edid_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	printk(KERN_DEBUG"[AL]edid ddc module probe!!!!\n");
	return 0;
}

static int edid_i2c_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG"[AL]module remove\n");
	return 0;
}

static const struct i2c_device_id edid_i2c_id[] = {
	{ "edid_i2c", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, edid_i2c_id);

static struct i2c_driver edid_i2c_driver = {
	.driver = {
		.name = "edid_i2c",
	},
	.probe = edid_i2c_probe,
	.remove = edid_i2c_remove,
	.id_table = edid_i2c_id,
};

static struct i2c_board_info edid_i2c_info = {I2C_BOARD_INFO("edid_i2c", EDID_I2C_ADDR)};

static int __init edid_i2c_hw_init(void)
{
	struct i2c_adapter *adap=NULL;
	
	printk(KERN_DEBUG"[AL]edid module init\n");
	adap = i2c_get_adapter( am_get_i2c_bus_id() );
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	edid_i2c_client = i2c_new_device(adap,&edid_i2c_info);
	if(!edid_i2c_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}
	
	return i2c_add_driver(&edid_i2c_driver);
}

static void __exit edid_i2c_hw_exit(void)
{
	i2c_del_driver(&edid_i2c_driver);
	i2c_unregister_device(edid_i2c_client);
	i2c_put_adapter(edid_i2c_client->adapter);
}
MODULE_AUTHOR("YangJY <yangjy@actions-micro.com>");
MODULE_DESCRIPTION("edid_i2c_hw driver");
MODULE_LICENSE("GPL");

module_init(edid_i2c_hw_init);
module_exit(edid_i2c_hw_exit);
