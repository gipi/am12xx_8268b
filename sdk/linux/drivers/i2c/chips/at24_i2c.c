/*
 * at24.c - handle most I2C EEPROMs
 * 
 */ 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>  
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <asm/delay.h>
#include <am7x_i2c.h>
#include <am7x_pm.h>
#include <am7x_dev.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include "sys_at24c02.h"
#include <asm/io.h>
#include <asm/uaccess.h>


#define AT24_I2C_ADDR 0X57
#define AT24_LENGHTH  0X08
#define I2C_MAJOR  89

static  struct i2c_client *at24_client;
struct at24_str {
	struct i2c_client *at24_client;
	struct mutex		lock;
} *at24_data;

static void at24_eeprom_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 
flags)
{
	msg->addr = AT24_I2C_ADDR;
	msg->flags = (at24_data->at24_client->flags & I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}

int at24_eeprom_multiread(eeprom_data_p i2c_data)  //struct i2c_adapter *adapter,
{
	unsigned char databuf[4 + MAX_DATA_LEN];
	unsigned int  i;
	struct i2c_msg at24_msg[2];
	int ret=0;	
//	printk("read data len=%d,addr=%d!\n",i2c_data->len,i2c_data->addr);
	mutex_lock(&at24_data->lock);
	if(i2c_data->addr < 256){
		if(i2c_data->len <= MAX_DATA_LEN){
			databuf[0] = (unsigned char)i2c_data->addr; 	

			at24_msg[0].addr = AT24_I2C_ADDR;
			at24_msg[0].flags = 0;
			at24_msg[0].len	= 1;
			at24_msg[0].buf	= databuf;
	
			at24_msg[1].addr	= AT24_I2C_ADDR;
			at24_msg[1].flags	= I2C_M_RD;
			at24_msg[1].len	= i2c_data->len;
			at24_msg[1].buf	= (databuf+1);		
			msleep(10);
			ret = i2c_transfer(at24_data->at24_client->adapter,at24_msg,2);
//			ret = i2c_transfer(adapter, at24_msg, 2);
			if(2==ret){
				for(i=0;i<i2c_data->len;i++){
					i2c_data->buf[i]= databuf[i+1];	
//					printk("buf[%d]=%02x, ",i,i2c_data->buf[i]);
				}
			}
			else{
				ret = -1;	
			}
		} 
	    else{
//			printk("read_eeprom address exceed!\n");
			ret = -1;
		}
	}
	else{
//		printk("read_eeprom too many 1!\n");
		ret = -1;
	}
//	printk("\n multiread ret=%d\n",ret);
	mutex_unlock(&at24_data->lock);
	return ret;
}


int at24_eeprom_multiwrite(eeprom_data_p i2c_data)//struct i2c_adapter *adapter,
{
	unsigned char databuf[4 +MAX_DATA_LEN];
	unsigned int  i;
	struct i2c_msg at24_msg;
	int ret=0;
//	printk("write data len=%d,addr=%d!\n",i2c_data->len,i2c_data->addr);
	mutex_lock(&at24_data->lock);
	if(i2c_data->addr < 256){
		if(i2c_data->len <= MAX_DATA_LEN){
			databuf[0] = (unsigned char)(i2c_data->addr);
			for(i=0;i< i2c_data->len;i++){
				databuf[i+1]= i2c_data->buf[i];
				}
			at24_msg.addr = AT24_I2C_ADDR;
			at24_msg.flags = 0;
			at24_msg.len = 1+(i2c_data->len);
			at24_msg.buf = (databuf);			

			ret = i2c_transfer(at24_data->at24_client->adapter,&at24_msg,1);
			msleep(10);
			if(1 != ret){
				printk("write eeprom transfer error!\n");
				ret = -1;
				}			
		}
		else{
//			printk("write_eeprom too many data!\n");
			ret = -1;
		}
	}
	else{
//		printk("write_eeprom address exceed!\n");		
		ret = -1;
	}
//	printk("multiwrite ret=%d\n",ret);
	mutex_unlock(&at24_data->lock);
	return ret;
}


int i2c_eeprom_write(eeprom_data_p eeprom_data)
{ 
	int ret=0;
	struct eeprom_data_t data_temp;
	unsigned int i,temp;
	if(eeprom_data->addr + eeprom_data->len >256){
		printk("write data exceeds EEPROM address,please cheak again!\n");
		ret= -1;
		goto out_err;
	}
if(((eeprom_data->addr) & 0xf8) == (((eeprom_data->addr + eeprom_data->len) - 1)&0xf8)){
		if(1!=at24_eeprom_multiwrite(eeprom_data)){
			ret = -1;
			goto out_err;
			}
	}
	else{
		data_temp.addr = eeprom_data->addr;
		data_temp.len = ((eeprom_data->addr)& 0xf8) + 8 - eeprom_data->addr;
		temp = data_temp.len;
		for(i=0;i<data_temp.len;i++) {
			data_temp.buf[i] = eeprom_data->buf[i];
			}
		
		if(1 != at24_eeprom_multiwrite(&data_temp)){
			ret= -1;
			goto out_err;
			}
		data_temp.addr = (eeprom_data->addr & 0xf8) + 8;
		data_temp.len = (eeprom_data->len) - temp;
		for(i=0;i<data_temp.len;i++) {
			data_temp.buf[i] = eeprom_data->buf[temp+i];
		}	
		if(1 != at24_eeprom_multiwrite(&data_temp)){
			ret= -1;
			}
		//mdelay(4);//msleep(4);
 	}	
out_err:
 	return ret;
}

EXPORT_SYMBOL(i2c_eeprom_write);

int i2c_eeprom_read(eeprom_data_p eeprom_data)
{
		int ret=0;
		if(eeprom_data->addr + eeprom_data->len >256){
			printk("read data exceeds EEPROM address,please cheak again!\n");
			ret= -1;
			goto to_err;
		}
		if(2 != at24_eeprom_multiread(eeprom_data)){
			ret = -1;
			printk("read eeprom transfer error!\n");
		}	
to_err:
		return ret;
}

EXPORT_SYMBOL(i2c_eeprom_read);


int i2c_write_mac_addr(mac_address_p mac_value){
	struct eeprom_data_t data_temp;
	unsigned int temp;
	int ret=0;
	if(mac_value==NULL){
		ret = -1;
	}
	else{
		data_temp.len = 6;
		data_temp.addr = 0;
		data_temp.buf[0] = 'A';
		data_temp.buf[1] = 'C';
		data_temp.buf[2] = 'T';
		data_temp.buf[3] = 'M';
		data_temp.buf[4] = 'A';
		data_temp.buf[5] = 'C';
		if(i2c_eeprom_write(&data_temp)){
			ret = -1;
			goto wrt_err;
		}
		data_temp.len = 7;
		data_temp.addr = 6;
		data_temp.buf[0] = mac_value->mac_addr_0;
		data_temp.buf[1] = mac_value->mac_addr_1;
		data_temp.buf[2] = mac_value->mac_addr_2;
		data_temp.buf[3] = mac_value->mac_addr_3;
		data_temp.buf[4] = mac_value->mac_addr_4;
		data_temp.buf[5] = mac_value->mac_addr_5;
		temp = 0x1a9+data_temp.buf[0];
		temp +=data_temp.buf[1];
		temp +=data_temp.buf[2];
		temp +=data_temp.buf[3];
		temp +=data_temp.buf[4];
		temp +=data_temp.buf[5];
		data_temp.buf[6] = (unsigned char)temp;
		if(i2c_eeprom_write(&data_temp)){
			ret = -1;
//			goto wrt_err;
		}
//		ret = 0;
	}
wrt_err:
	return ret;
}

EXPORT_SYMBOL(i2c_write_mac_addr);

struct file* file_open(const char* path, int flags, int rights) {
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file* file) {
    filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

/* Mos: vfs_fsync has different prototype in 2.6.27, and might have bug? https://bugzilla.kernel.org/show_bug.cgi?id=12579 */
/* int file_sync(struct file* file) {
    vfs_fsync(file, 0);
    return 0;
}*/

int i2c_read_mac_addr(mac_address_p mac_value){
	struct eeprom_data_t data_temp;
	unsigned int temp;
	int ret=0;

    struct file* fp = NULL;
    unsigned char data[7];

	if(mac_value==NULL){
		ret = -1;
	}
	else{
		data_temp.len = 6;
		data_temp.addr = 0;
		if(i2c_eeprom_read(&data_temp)){
			ret = -1;
			goto rd_err;
		}
		if('A' != data_temp.buf[0]){
			ret = -1;
			goto rd_err;
		}
		if('C' != data_temp.buf[1]){
			ret = -1;
			goto rd_err;
		}
		if('T' != data_temp.buf[2]){
			ret = -1;
			goto rd_err;
		}
		if('M' != data_temp.buf[3]){
			ret = -1;
			goto rd_err;
		}
		if('A' != data_temp.buf[4]){
			ret = -1;
			goto rd_err;
		}
		if('C' != data_temp.buf[5]){
			ret = -1;
			goto rd_err;
		}	
		data_temp.len = 7;
		data_temp.addr = 6;
		if(i2c_eeprom_read(&data_temp)){
			ret = -2;
			goto rd_err;			
		}
		temp = 0x1a9+data_temp.buf[0];
		temp +=data_temp.buf[1];
		temp +=data_temp.buf[2];
		temp +=data_temp.buf[3];
		temp +=data_temp.buf[4];
		temp +=data_temp.buf[5];
		if(data_temp.buf[6] != (unsigned char)temp){
			ret = -2;
			goto rd_err;			
		}
		mac_value->mac_addr_0 = data_temp.buf[0];
		mac_value->mac_addr_1 = data_temp.buf[1];
		mac_value->mac_addr_2 = data_temp.buf[2];
		mac_value->mac_addr_3 = data_temp.buf[3];
		mac_value->mac_addr_4 = data_temp.buf[4];
		mac_value->mac_addr_5 = data_temp.buf[5];		
//		ret = 0;

        /* Mos: Backup MAC Address into NAND Flash, workaround EEPROM might read abnormal when connect Chimei TV/Monitor */
        fp = file_open("/mnt/vram/MACADDR_BACKUP", O_RDONLY|O_WRONLY, 0644);
        if (fp == NULL){
            /* File not exist */
            fp = file_open("/mnt/vram/MACADDR_BACKUP", O_WRONLY|O_CREAT, 0644);
            if (fp == NULL){
                printk("MACADDR_BACKUP not exist, and create fail!\n");
            }
            else{
                file_write(fp, 0, data_temp.buf, 7);
            }
        }
        else{
            /* File exist but not equal with EEPROM */
            file_read(fp, 0, data, 7);
            if (memcmp(data, data_temp.buf, 7) != 0){
                file_write(fp, 0, data_temp.buf, 7);
            }
        }
        if (fp != NULL){
            file_close(fp);
        }
	}

    return ret;

rd_err:

    /* Mos: EEPROM read abnormal... now try to get MAC from backup */
    fp = file_open("/mnt/vram/MACADDR_BACKUP", O_RDONLY, 0644);
    if (fp == NULL){
        /* File not exist */
        printk("EEPROM read abnormal, MACADDR_BACKUP not exist!!\n");
        ret = -1;
    }
    else{
        file_read(fp, 0, data_temp.buf, 7);

        temp = 0x1a9+data_temp.buf[0];
        temp +=data_temp.buf[1];
        temp +=data_temp.buf[2];
        temp +=data_temp.buf[3];
        temp +=data_temp.buf[4];
        temp +=data_temp.buf[5];
        if(data_temp.buf[6] != (unsigned char)temp){
            printk("EEPROM read abnormal, MACADDR_BACKUP checksum error!!\n");
            ret = -2;
        }
        else{
            printk("EEPROM read abnormal, recovery from MACADDR_BACKUP!!\n");
            mac_value->mac_addr_0 = data_temp.buf[0];
            mac_value->mac_addr_1 = data_temp.buf[1];
            mac_value->mac_addr_2 = data_temp.buf[2];
            mac_value->mac_addr_3 = data_temp.buf[3];
            mac_value->mac_addr_4 = data_temp.buf[4];
            mac_value->mac_addr_5 = data_temp.buf[5];
            ret = 0;
        }
    }
    if (fp != NULL){
        file_close(fp);
    }
	return ret;
}
EXPORT_SYMBOL(i2c_read_mac_addr);

static ssize_t at24_eeprom_open(struct inode * inode, struct file * filp)
{	
	printk("at24_eeprom_open\n");
 	return 0;
	
}

static ssize_t at24_eeprom_read(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)//( unsigned char *PCM_DATA , unsigned int  PCM_LENS)
{
	printk("at24_eeprom_read\n");	
	return 0;
}
static ssize_t at24_eeprom_write(struct file *pfile, char __user *buf, size_t size, loff_t *offset)//( unsigned char *PCM_DATA , unsigned int  PCM_LENS)
{		
	printk("at24_eeprom_write\n");				
	return 0;
}

static ssize_t at24_eeprom_release(struct inode *pinode, struct file *pfile)
{
	printk("at24_eeprom_release\n");	
	return 0;
}


static int at24_eeprom_ioctl(struct inode *inode, struct file *file, unsigned int 
cmd, unsigned long arg){
		int  retval=0; 
		struct eeprom_data_t eeprom_datatemp; 
		switch (cmd) {
		case EEPROM_READ_MULTIBYTE:	
				if(copy_from_user((eeprom_data_p)&eeprom_datatemp, (eeprom_data_p)arg, 
	sizeof(eeprom_datatemp))){
//					printk("read eeprom copy to kernel failed!\n");
					return -EFAULT;	
					}				
	       		if(2==at24_eeprom_multiread(&eeprom_datatemp)){
				if(copy_to_user((eeprom_data_p)arg,(eeprom_data_p)&eeprom_datatemp,  
		sizeof(eeprom_datatemp))){
//					printk("read eeprom copy to user failed!\n");
		        	return -EFAULT;	
					}
	       		}
				else{
//					printk("read eeprom transfer failed!\n");
					retval = -EINVAL;
				}
			
			break;
	
			case EEPROM_WRITE_MULTIBYTE:	
			//	printk("write eeprom start!\n");
							if(copy_from_user((eeprom_data_p)&eeprom_datatemp, (eeprom_data_p)arg, 
				sizeof(eeprom_datatemp))){
//								printk("write eeprom copy to kernel failed!\n");
								return -EFAULT; 
								}
							if(1!=at24_eeprom_multiwrite(&eeprom_datatemp)){ 
//								printk("read eeprom transfer failed!\n");
								retval = -EINVAL;
							}
						
			 break; 
			default:
				printk("unkown command for i2c eeprom!\n");
				return -ENOTTY;
			}
		return retval;
	
}


struct cdev  *at24_cdev=NULL;
dev_t  at24_dev;
static struct  class *at24_class=NULL; 
static struct file_operations at24_drv_fops=
{
	.owner  = THIS_MODULE,	
	.read 	= at24_eeprom_read,
	.write = at24_eeprom_write,
	.ioctl = at24_eeprom_ioctl,
	.open = at24_eeprom_open,
	.release = at24_eeprom_release,
};


static struct i2c_board_info at24_info={
	I2C_BOARD_INFO("i2c_eeprom",AT24_I2C_ADDR),
	};

static int at24_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	printk(KERN_DEBUG"[AL]i2c eeprom module probe!\n");
	return 0;
}


static int at24_i2c_remove(struct i2c_client *client){
	printk(KERN_DEBUG"remove i2c eeprom module!\n");	
	return 0;	
}

static const struct i2c_device_id at24_i2c_id[]={
	{"at24c02",0},
	{},
};

MODULE_DEVICE_TABLE(i2c, at24_i2c_id);



static struct i2c_driver at24_driver = {
	.driver = {
	.name = "at24c02",
	},
	.probe = at24_i2c_probe,
	.remove = at24_i2c_remove,
	.id_table = at24_i2c_id,
};

static int __init at24_i2c_init(void)
{	
	struct i2c_adapter *adap= NULL;
	int ret = 0;
	printk("i2c eeprom char device init!\n");
	at24_dev = MKDEV(I2C_MAJOR,6);
	ret = register_chrdev_region(at24_dev,1,"i2c_24c02");
	if(ret){
		printk(KERN_ERR "alloc_chrdev_region() failed for i2c eeprom");
	}
	printk("i2c_24c02 major=%d,minor=%d!\n",MAJOR(at24_dev), MINOR(at24_dev));
	at24_cdev=kzalloc(sizeof(struct cdev),GFP_KERNEL);
	cdev_init(at24_cdev,&at24_drv_fops);
	if(cdev_add(at24_cdev,at24_dev,1))
		goto out_err0;
	at24_class = class_create(THIS_MODULE,"i2c_24c02");
	if(IS_ERR(at24_class)){
		printk(KERN_ERR "can not create a cdev_class for 24c02");
		goto out_err0;
		}
	device_create(at24_class,NULL,at24_dev,0,"i2c_24c02");

	adap = i2c_get_adapter( am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
//		ret = -ENXIO;
		goto out_err0;		
		}
	at24_data=kzalloc(sizeof(struct at24_str ),GFP_KERNEL);
	if(!at24_data){
		printk(KERN_ERR "alloc_at24_str failed for i2c eeprom");	
//		ret = -ENXIO;
		goto out_err0;	
	}
	mutex_init(&at24_data->lock);
	at24_data->at24_client = i2c_new_device(adap,&at24_info);
	if(!(at24_data->at24_client) ){
		printk(KERN_ERR"gen device for i2c eeprom error!\n");
//		ret = -ENXIO;
		goto out_err1;		
		}
	return i2c_add_driver(&at24_driver);
out_err1:
	kfree(at24_data);
out_err0:
	printk(KERN_ERR "register failed for i2c eeprom");
	kfree(at24_cdev);
	unregister_chrdev_region(at24_dev,1);
	return -ENODEV;	
}

static void __exit at24_i2c_exit(void)
{
	if(at24_cdev){
		cdev_del(at24_cdev);
		kfree(at24_cdev);
	}
	if(at24_data){
		kfree(at24_data);
	}
	device_destroy(at24_class,at24_dev);
	class_destroy(at24_class);
	unregister_chrdev_region(at24_dev,1); //AM7X_I2C_EEPROM
	i2c_del_driver(&at24_driver);
	i2c_unregister_device(at24_client);
	i2c_put_adapter(at24_client->adapter);
	
}
module_exit(at24_i2c_exit);
module_init(at24_i2c_init);

MODULE_DESCRIPTION("Driver for most I2C EEPROMs");
MODULE_AUTHOR("leiwg@Actions-micro.com");
MODULE_LICENSE("GPL");

