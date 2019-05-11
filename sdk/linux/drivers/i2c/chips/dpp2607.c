/*
 *  DPP2607
 ****************************
 * Actions-micro DAC IC
 *
 * author: Charles
 * date: 2012-09-19
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


#include <asm/delay.h>
#include <am7x_i2c.h>
#include <am7x_pm.h>
#include <am7x_dev.h>
#include <sys_pmu.h>
#include <sys_dpp2607.h>
#include "dpp2607.h"//<linux/i2c/dpp2607.h>



/* dpp2607 device address */
#define DPP2600_DEV_ADDR		0x1B	//0x36 * 2


#define CONFIG_DPP_PROC_FS



#define MAX_DPP_CURRENT	((__u16)3000)

/* dpp2607 registers */
/* here's all the must remember stuff */
dev_t dpp_dev;
int dev_count=1;
char *dpp_name = "dpp2607";
struct cdev *dev_cdev = NULL;
static struct i2c_client *dpp2607_client;

#ifdef CONFIG_DPP_PROC_FS
static struct proc_dir_entry *g_proc_entry = NULL;
#endif


struct class *g_dpp_class = NULL;


/* typedefs for portability */

// This one is for discrete LED current
static __s32 led_drive_current = -1;
static __s32 g_dpp_current_wall_color = -1;


// This one is for PAD1000 LED current                                     
static __u16 led_drive_current_array[10] = { 
0,
100,   // 77 mA       //0x64
200,   // 177 mA      //0xc8
300,   // 277 mA      //0x01 0x2c
400,   // 377 mA      //0x01 0x90
500,   // 477 mA      //0x01 0xf4
600,   // 577 mA      //0x02 0x58
664,   // 641 mA      //0x02 0x98
700,   // 677 mA      //0x02 0xbc
840 //old 900  877 mA     //0x03 0x84
}; 




static void dpp2607_set_msg(struct i2c_msg *msg, __u8 *buf, __u16 len, __u16 flags)
{
	msg->addr = dpp2607_client->addr;
	msg->flags = (dpp2607_client->flags&I2C_M_TEN)|flags;
	msg->buf = buf;
	msg->len = len;
}
/*  
* val[0] should be the reg address 
* the reg value will be stored from val[1] in return
*/
static int dpp2607_read_reg(__u8 *val, __u16 len)
{
	int ret;
	struct i2c_msg dpp2607_msg[2];
	
	dpp2607_set_msg(&dpp2607_msg[0],val,2,0);
	ret = i2c_transfer(dpp2607_client->adapter,dpp2607_msg,1);
	udelay(50);
	dpp2607_set_msg(&dpp2607_msg[1],val+2,len,I2C_M_RD);
	ret = i2c_transfer(dpp2607_client->adapter,dpp2607_msg+1,1);
	
	return ret;
}
/*
* val[0] should be the reg address
* val[1] should be the value set from val[0]
*/
static int dpp2607_write_reg(__u8 *val, __u16 len)
{
	int ret = 0;
	struct i2c_msg dpp2607_msg;

	dpp2607_set_msg(&dpp2607_msg, val, len, 0);
	ret = i2c_transfer(dpp2607_client->adapter,&dpp2607_msg,1);
	if(ret>0)
		udelay(1);
	
	return ret;
}

////////////////////////////////////////////////////////////////
// I2C Write 4 bytes to DPP

static int write_dpp_i2c(__u8 subaddr, __u32 data, __u8 verify)
{
	
	int ret = 0;
	__u8 i2c_array[5];
 	struct i2c_msg dpp2607_msg;
	
	i2c_array[0] = subaddr;
	i2c_array[1] = (__u8) (data>>24);
	i2c_array[2] = (__u8) (data>>16);
	i2c_array[3] = (__u8) (data>>8);
	i2c_array[4] = (__u8) data;

	dpp2607_set_msg(&dpp2607_msg, i2c_array, 5, 0);
	ret = i2c_transfer(dpp2607_client->adapter,&dpp2607_msg,1);
	if(ret>0)
		udelay(1);
	else
		{
			printk(KERN_ERR"write_dpp_i2c error \n");
			return ret;
		}
#if 0
	if (verify)
	{
		read_dpp_i2c(addr, subaddr, &read_data);

		// compare the read data to what we just sent
		if (read_data != data) 
		{
			printk(KERN_ERR"write_dpp_i2c Verify error \n");
		}
	}
#endif
	
	return ret;
}

/////////////////////////////////////////////////////////////////
// I2C read 4 bytes from DPP

static int read_dpp_i2c(__u8 subaddr, __u32* data)
{
	int ret = 0;
	__u8 i2c_array[6];
	struct i2c_msg dpp2607_msg[2];
	
	if (data == NULL)
	{
		printk(KERN_ERR"read_dpp_i2c data null \n");
		return ret;
	}
	
  // setup the readback
	i2c_array[0] = (__u8) READ_REG_SELECT;
	i2c_array[1] = subaddr;
	dpp2607_set_msg(&dpp2607_msg[0],i2c_array,2,0);
	ret = i2c_transfer(dpp2607_client->adapter,dpp2607_msg,1);
	
	udelay(50);
	dpp2607_set_msg(&dpp2607_msg[1],i2c_array+2,4,I2C_M_RD);
	ret = i2c_transfer(dpp2607_client->adapter,dpp2607_msg+1,1);
	
	if(ret>0){
		udelay(1);
	}else{
			printk(KERN_ERR"read_dpp_i2c error \n");
			*data = 0;
			return ret;
	}		

	*data = (__u32)i2c_array[2]<<24 |
	      (__u32)i2c_array[3]<<16 |
	      (__u32)i2c_array[4]<<8  |
	      (__u32)i2c_array[5];
	
	return ret;	  
}





/**********   Curtain Functions *************/
/*
 * Enable image curtain. Curtain color can be set by calling function DPP_set_curtaincolor(color)  
 *
 * @param   enable - I - TRUE  = Enable image curtain
 *                       FALSE = Disable image curtain
 * 
 * @return  none
 * 
 * Note: When enabled the entire DMD image will be forced to curtain color (requires the Sequence to be running).  
 * This provides an alternate method of masking temporary corruption caused by reconfiguration from reaching the display
 */
static void DPP_enable_curtain(__u8 enable)
{
	__u32 read;
	read_dpp_i2c(CURTAIN_CONTROL, &read);
	if(enable)
	{
		if((read & 0x1) == 0)  // if already enabled do nothing.
			// do not change the curtain color. Only enable curtain
			write_dpp_i2c(CURTAIN_CONTROL, (read | 0x1), 0);	
	}
	else
		write_dpp_i2c(CURTAIN_CONTROL, (read & ~0x1), 0);						
}

/*
 * This function returns the status of image curtain.
 *
 * @return  TRUE   = Enabled
 *          FALSE  = Disabled
 */
static __u8 DPP_is_curtain_enable(void)

{
	__u32 read;
	read_dpp_i2c(CURTAIN_CONTROL, &read);
	if(read & 0x1)
		return TRUE;
	else
		return FALSE;			
}



/*
 * Enable image curtain. Curtain color can be set by calling function DPP_set_curtaincolor(color)  
 *
 * @param   color  - I - 	CURTAIN_BLACK             
 *							CURTAIN_RED               
 *							CURTAIN_GREEN             
 *							CURTAIN_BLUE              
 *							CURTAIN_YELLOW            
 *							CURTAIN_MAGENTA           
 *							CURTAIN_CYAN              
 *							CURTAIN_WHITE             
 * 
 * @return  none
 * 
 * Note: This function does not enable/disable image curtain. It only modifies the curtain color. To enable/disbale 
 * image curtain call function DPP_enable_curtain(enable)
 */ 
static void DPP_set_curtaincolor(__u8 color)
{
	__u32 read;
	read_dpp_i2c(CURTAIN_CONTROL, &read);
	read &= 0x1;
	read |= (color << 4);
	write_dpp_i2c(CURTAIN_CONTROL, read, 0);			
} 

/////////////////////////////////////////////////////////////////
// This function is to write compound I2C command to ICP8051

static int dpp_write_ICP8051(__u8 cmd, __u32 param)
{
	__u32 dpp2600_status;
	int count = 20;
	
	// Busy Enable in  handshake register
	write_dpp_i2c(0x3A, 1, 0);
	
	write_dpp_i2c(0x39, param, 0);
	
	write_dpp_i2c(0x38, cmd, 0);
	
	read_dpp_i2c(0x3A, &dpp2600_status);
	
   	do 
	{
	    	read_dpp_i2c(0x3A, &dpp2600_status);
	    	// MCLK is 4 MHz so delay of 100 ms is 4000000*100/1000
	      udelay(100*1000);
      		;
	} while ( ((dpp2600_status & 0x1) != 0x0 ) && count--);
	
	return (count > 0) ? TRUE : FALSE;
}	



static __u32 dpp_read_ICP8051_status(__u8 cmd)
{
	__u32 dpp2600_status = 0xffff;
	int count = 20;
		
	// Busy Enable in handshake register
	write_dpp_i2c(0x3A, 1, 0);
	write_dpp_i2c(0x38, cmd, 0);
	read_dpp_i2c(0x3A, &dpp2600_status);
	

   	do 
  	{
	      read_dpp_i2c(0x3A, &dpp2600_status);
	      // MCLK is 4 MHz so delay of 100 ms is 4000000*100/1000
	      udelay(100*1000);
	} while ( ((dpp2600_status & 0x1) != 0x0 ) && count--);	
	
	read_dpp_i2c(0x39, &dpp2600_status);
	
	return dpp2600_status;	
}	



#define DPP_WAIT_ICP_DOWN()	(dpp_write_ICP8051(0xd3, 0x00))

inline static int dpp_wait_task_down(void)
{
	return dpp_write_ICP8051(0xd3, 0x00);
}


inline static int dpp_read_Pad_Thermister(void)
{
	return (dpp_read_ICP8051_status(0xc5)&0xFFF)/10 ;
}


static int __dpp_set_bright(__u16 cur)
{
   	int ret = 0;

	if(cur == led_drive_current || cur > MAX_DPP_CURRENT){
		return 0;
	}
	
	write_dpp_i2c(R_DRIVE_CURRENT, cur, 0);
	write_dpp_i2c(G_DRIVE_CURRENT, cur, 0);
	write_dpp_i2c(B_DRIVE_CURRENT, cur, 0);


	if(0 != cur ){
		write_dpp_i2c(RGB_DRIVER_ENABLE, 0x07, 0);
	}else{
		write_dpp_i2c(RGB_DRIVER_ENABLE, 0x00, 0);
	}

	if(dpp_wait_task_down() == FALSE){
		led_drive_current = -1;
		ret = -1;
		printk("%s , %d: set bright err\n", __FUNCTION__, __LINE__);
	}else{
		led_drive_current = cur;
		printk("[%s]led current_cur == %d\n", __func__, cur);
	}
	
	return ret;
}


static int dpp2607_open(struct inode *inode, struct file *file)
{
	return 0;
}





static int dpp_set_bright(__u8 cur)
{
       int ret = 0;

	if(led_drive_current_array[cur] == led_drive_current){
		return 0;
	}
	
	ret = __dpp_set_bright(led_drive_current_array[cur]);
	
	return ret;
}


static inline int dpp_set_true_bright(__u16 cur)
{
       int ret = 0;

	if(cur == led_drive_current){
		return 0;
	}
	
	ret = __dpp_set_bright(cur);
	
	return ret;
}



static int dpp_wall_correction(__u8 num)
{
	int ret = 0;
	char mode = 0;


	if(led_drive_current < 625){//625 this is for P30
		mode = num;
	}else{
		mode = num + 5;
	}

	if(g_dpp_current_wall_color == mode){
		return 0;
	}

	if(dpp_write_ICP8051(0xc1, mode) == TRUE){
		g_dpp_current_wall_color = mode;
	}else{
		printk("%s , %d: set wall color err\n", __FUNCTION__, __LINE__);
		g_dpp_current_wall_color = -1;
	}
	
	return ret;

}




static int dpp_init_dev(__u8 cur)
{
       int ret = 0;
	
	write_dpp_i2c(CURTAIN_CONTROL, 0x01, 0);
	write_dpp_i2c(SEQUENCE_MODE, 0x01, 0);       // 同步
	write_dpp_i2c(DATA_FORMAT, RGB888, 0);   //RGB888
	write_dpp_i2c(INPUT_RESOLUTION, nHD_Landscape, 0);   //   0x1b 640*360//   0x0d 800*480
	write_dpp_i2c(INPUT_SOURCE, 0x00, 0);

	
	dpp_set_bright(cur);
	
	dpp_wait_task_down();	
	//set gamma
	dpp_write_ICP8051(0x26, 0x1);// film

	write_dpp_i2c(AGC_CTRL, 0x06, 0);//AGC
	
       udelay(35000);

	write_dpp_i2c(CURTAIN_CONTROL, 0x00, 0);


	return ret;
}




static int dpp2607_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval =0 ;
	struct dpp2607_data data;
	__u16 cur = 0;
	__u8 value[6];
	__u32 tmp = 0;
	
	memset(value, 0, 6);
	
	switch(cmd)
	{
		case READ_REG_DPP:
			copy_from_user(&data, (void *)arg, sizeof(data));
			value[0] = 0x15;
			value[1] = data.reg;
			retval=dpp2607_read_reg(value,4);
			data.val[0] = value[1];
			data.val[1] = value[2];
			data.val[2] = value[3];
			data.val[3] = value[4];
			copy_to_user((void *)arg, &data, sizeof(data));
			break;
		case WRITE_REG_DPP:	
			copy_from_user(&data, (void *)arg,sizeof(data));
			value[0] = data.reg;
			value[1] = data.val[0];
			value[2] = data.val[1];
			value[3] = data.val[2];
			value[4] = data.val[3];
			retval=dpp2607_write_reg(value,5);
			break;
			
		case CMD_BRIGHT_DPP:
			copy_from_user(&data, (void *)arg, sizeof(data));
			if(data.val[3]<0 || data.val[3] > 9){
				data.val[3] = 9;
			}
			retval  = dpp_set_bright(data.val[3]);
			break;
		case CMD_CORRECT_DPP://data.val[3] 选择的模式   data.val[2]当前的电流模式
			copy_from_user(&data, (void *)arg,sizeof(data));
			if(data.val[3]<0 || data.val[3] > 4){
				data.val[3] = 0;
			}
			retval = dpp_wall_correction(data.val[3]);
			break;
			
		case RESET_DPP:
			dpp_init_dev(9);
			break;
			
		case CMD_READ_TMP_DPP:
			retval = dpp_read_Pad_Thermister();
			copy_to_user((void *)arg,&retval,sizeof(retval));
			retval = 0;
			break;
			
		case CMD_BRIGHT_TRUE_DPP:
			copy_from_user(&cur, (void *)arg,sizeof(cur));
			if(cur > MAX_DPP_CURRENT){
				printk(KERN_ERR"--error curr to set dpp!cur == %d\n", cur);
				cur = MAX_DPP_CURRENT;
			}
			retval  = dpp_set_true_bright(cur);
			break;
		case CMD_SET_ICP:
			copy_from_user(&data, (void*)arg,sizeof(data));
			dpp_write_ICP8051(data.val[2], data.val[3]);
			break;
		case CMD_GET_ICP:
			copy_from_user(&data, (void*)arg,sizeof(data));
			
			tmp = dpp_read_ICP8051_status(data.val[3]);
			data.val[0] = (__u8)(tmp>>24);
			data.val[1] = (__u8)(tmp>>16);
			data.val[2] = (__u8)(tmp>>8);
			data.val[3] = (__u8)tmp;
			retval = 0;
			break;
		
		default:
			printk(KERN_ERR"--unknown command\n");			
			break;
	}
	
	return retval;
}

/* VFS methods */
static struct file_operations dpp2607_fops ={
	.open = dpp2607_open,
	.ioctl = dpp2607_ioctl,
};


 
static int dpp2607_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int ret = 0;
	printk(KERN_INFO"[DPP]module probe!!!!\n");	

	dpp_init_dev(9);
	
	return ret;
}

static int dpp2607_remove(struct i2c_client *client)
{
	
	return 0;
}

static const struct i2c_device_id dpp2607_id[] = {
	{ "dpp2607", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, dpp2607_id);

static struct i2c_driver dpp2607_driver = {
	.driver = {
	.name = "dpp2607",
	},
	.probe = dpp2607_probe,
	.remove = dpp2607_remove,
	.id_table = dpp2607_id,
};

static struct i2c_board_info dpp2607_info = {I2C_BOARD_INFO("dpp2607", DPP2600_DEV_ADDR)};


#ifdef CONFIG_DPP_PROC_FS
static int dpp_proc_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
	int ret = 0;

	printk("[%s]\n", __func__);
	
	__dpp_set_bright(900);
	
	return count;
}

static int dpp_proc_read(char*buffer, char**buffer_localation, off_t offset,

                            int buffer_length,int* eof, void *data )
{
	int ret = 0;
	printk("[%s]\n", __func__);
	return ret;
}
#endif



static int __init dpp2607_init(void)
{
	struct i2c_adapter *adap=NULL;
	int result = 0;
	int dpp_major = 0;//DPP2607_MAJOR 243
	int dpp_minor = 0;
	
	printk(KERN_INFO"[DPP2607]module init\n");
	
	adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return -ENXIO;
	}
	dpp2607_client = i2c_new_device(adap,&dpp2607_info);
	if(!dpp2607_client){
		printk(KERN_ERR"gen device error!\n");
		return -ENXIO;
	}

	#if 0
	if (register_chrdev(DPP2607_MAJOR, "dpp2607",&dpp2607_fops))
		printk("unable to get major %d for memory devs\n", DPP2607_MAJOR);

	#else
	/* if you want to test the module, you obviously need to "mknod". */
	if(dpp_major)
	{
		dpp_dev = MKDEV(dpp_major, dpp_minor);
		result = register_chrdev_region(dpp_dev, dev_count, dpp_name);
	}
	else
	{
		result = alloc_chrdev_region(&dpp_dev, dpp_major, dev_count, dpp_name);
		dpp_major = MAJOR(dpp_dev);
	}
	printk("dpp_major =%d\n", dpp_major);

	dev_cdev = cdev_alloc();
	cdev_init(dev_cdev,&dpp2607_fops);
	dev_cdev->owner=THIS_MODULE;
	//dev_cdev->ops=&dpp2607_fops;
	result = cdev_add(dev_cdev, dpp_dev,dev_count);
	if(result < 0)
	{
		printk("can't add dpp dev");
		unregister_chrdev_region(dpp_dev, dev_count);
		return result;
	}

	g_dpp_class = class_create(THIS_MODULE,"dpp_class");
	if(IS_ERR(g_dpp_class)) {
		unregister_chrdev_region(dpp_dev, dev_count);	
		cdev_del(dev_cdev);
		printk("Err: failed in creating class.\n");
		return -1; 
	}

	device_create(g_dpp_class, NULL, dpp_dev, NULL, dpp_name);
	

	printk("[DPP2607]module init end\n");
	#endif


	#ifdef CONFIG_DPP_PROC_FS
	g_proc_entry = create_proc_entry(dpp_name, 0644, NULL);
	if(NULL != g_proc_entry){
		g_proc_entry->read_proc = dpp_proc_read;
		g_proc_entry->write_proc = dpp_proc_write;
		g_proc_entry->owner = THIS_MODULE;
	}
	#endif
	
	return i2c_add_driver(&dpp2607_driver);
}

static void __exit dpp2607_exit(void)
{
	printk("[DPP]module remove\n");

	
	cdev_del(dev_cdev);

	device_destroy(g_dpp_class, dpp_dev);    //delete device node under /dev//必须先删除设备，再删除class类
	class_destroy(g_dpp_class);                 //delete class created by us

	unregister_chrdev_region(dpp_dev, dev_count);	
	
	i2c_del_driver(&dpp2607_driver);
	i2c_unregister_device(dpp2607_client);
	i2c_put_adapter(dpp2607_client->adapter);

	#ifdef CONFIG_DPP_PROC_FS
	remove_proc_entry(dpp_name, g_proc_entry->parent);
	#endif
}

MODULE_AUTHOR("Charles <lihaiji@actions-micro.com>");
MODULE_DESCRIPTION("dpp2607 driver");
MODULE_LICENSE("GPL");

module_init(dpp2607_init);
module_exit(dpp2607_exit);
