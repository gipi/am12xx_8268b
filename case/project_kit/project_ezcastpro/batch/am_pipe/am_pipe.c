/**
 * drivers/ScalerCom/ScalerCom.c
 *
 *this driver is for communication with Scaler by uart
 *
 *author: Nick
 *date:2010-11-25
 *version:0.1
 */

#include <linux/interrupt.h>
#include "actions_regs.h"
#include <linux/cdev.h>
#include <linux/syscalls.h>
#include "linux/fs.h"
#include <linux/file.h> 
#include "linux/module.h"
#include "linux/init.h"
#include <linux/syscalls.h>
#include <linux/major.h>
#include <linux/poll.h>
#include "am7x-freq.h"
#include <am7x_board.h>
#include <sys_msg.h>
#include "actions_io.h"
#include "am7x_gpio.h"
#include <am7x_dev.h>
#include <asm/mipsregs.h>
#include <sys_cfg.h>
#include <linux/delay.h>

#define DBG_AM_MSG		1
#if  DBG_AM_MSG
#define DBG_PIPE_MSG(format,args...)   printk(format,##args)
#else
#define DBG_PIPE_MSG(format,args...)   do {} while (0)
#endif
#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define act_readl(port)  (*(volatile unsigned int *)(port))
#define FUI_KEYACTION_DOWN_MASK (KEY_ACTION_DOWN<<KEY_ACTION_MASK_SHIFT)

int channel = 1;
char projector_status = '0';
int lan_in_flag = 0;
int eth0_flag = 0;
int OTA_percentage = -1;
int OTA_status = -1;
int OTA_update_need = -1;
int OTA_checksum_ok = 1;
char OTA_version[7];
char AM_version[7];
int DHCP_on_off = 1;
static dev_t pipe_dev;
static struct cdev  *pipe_cdev=NULL;


struct am_wifi_status{
	unsigned char mode; //none=0, client=1, ap=2, WD=3
	unsigned char IP_addr[16];
	unsigned char SSID[50];
	unsigned char PSK[32]; //ap mode only
	unsigned char security; //ap mode on;y
	unsigned int projector_ID;
	unsigned int modelname;
};

struct am_wifi_status *am_wifi = NULL;


typedef enum{
	CMD_GET_WIFI_MODE=1, //check mode first, if 0, show plug-out
	CMD_GET_WIFI_CONFIG,//2
	CMD_SET_WIFI_MODE,//3
	CMD_SET_WIFI_CONFIG,//4
	CMD_GET_PROJECTOR_STATUS,//5
	CMD_GET_CONNECT_USERS,//6
	CMD_GET_PROJECTOR_ID,//7
	CMD_SET_PROJECTOR_ID,//8
	CMD_SET_CONNECT_USERS,//9
	CMD_SET_CHANNEL_AVAILABLE,//10
	CMD_GET_CHANNEL_AVAILABLE,//11
	CMD_SET_PROJECTOR_STATUS,//12
	CMD_SET_LAN_PORT_FLAG,//13
	CMD_GET_LAN_PORT_FLAG,//14
	CMD_SET_LAN_IN_FLAG,//15
	CMD_GET_LAN_IN_FLAG,//16
	CMD_OTA_CHECK_FLAG,//17
	CMD_SET_OTA_PERCENTAGE,//18
	CMD_GET_OTA_PERCENTAGE,//19
	CMD_SET_OTA_STATUS,//20
	CMD_GET_OTA_STATUS,//21
	CMD_SET_OTA_UPDATE_NEED,//22
	CMD_GET_OTA_UPDATE_NEED,//23
	CMD_OTA_FLAG,//24
	CMD_SET_OTA_VERSION,//25
	CMD_GET_OTA_VERSION,//26
	CMD_OTA_CANCEL,//27
	CMD_OTA_SET_CHECKSUM,//28
	CMD_OTA_GET_CHECKSUM,//29
	CMD_OTA_START_UPGRADE,//30
	CMD_SET_AM_VERSION,//31
	CMD_GET_AM_VERSION,//32
	CMD_SET_DHCP_ON_OFF,//33
	CMD_GET_DHCP_ON_OFF,//34
	CMD_WAKE_UP_SCALER//35
}AM_PIPE_CMD_ID;

void pipe_dev_init()
{
	am_wifi=kzalloc(sizeof(struct am_wifi_status),GFP_KERNEL);
	if(!am_wifi){
		printk(KERN_ERR "malloc memory  fails for uartcom device\n");
		return;
	}
}


static void send_key_press(INT32U type,INT32U key){
	struct am_sys_msg uMsg;
	key = key & 0x000000ff;
	//key  = key | 0x100; // for ir
	//printk("type = %x, key = %x\n",type,key);
	uMsg.type = SYSMSG_KEY;
	uMsg.subtype = KEY_SUBTYPE_IR;
	uMsg.dataload = key | type;
	//--printk("dataload = %x\n",uMsg.dataload);
	am_put_sysmsg(uMsg);
}

static int am7x_pipe_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	INT8U *arg_str;
	int i;
	INT8U key = 0;
	INT8U Users = 0;
//	unsigned char charcmd[6]={0};
	DBG_PIPE_MSG("am_pipe_ioctl cmd=%d\n", cmd);
	printk("%s cmd=%d\n", __func__, cmd);
	switch(cmd){
		case CMD_GET_WIFI_MODE://1
			copy_to_user((void*)arg,(void*)&am_wifi->mode,1);
			break;
		case CMD_GET_WIFI_CONFIG://2
/*			DBG_PIPE_MSG("AM_PIPE: wifi mode==%d \n",am_wifi->mode);
			DBG_PIPE_MSG("AM_PIPE: IP==%s \n",am_wifi->IP_addr);
			DBG_PIPE_MSG("AM_PIPE: SSID==%s \n",am_wifi->SSID);			
			DBG_PIPE_MSG("AM_PIPE: PSK==%s \n",am_wifi->PSK);
			DBG_PIPE_MSG("AM_PIPE: security==%c \n",am_wifi->security);
			DBG_PIPE_MSG("AM_PIPE: projector_ID==%d \n",am_wifi->projector_ID);
			DBG_PIPE_MSG("AM_PIPE: modelname==%d \n",am_wifi->modelname);*/
			copy_to_user((void*)arg,(void*)am_wifi,sizeof(struct am_wifi_status));
			break;
		case CMD_SET_WIFI_MODE://3
			am_wifi->mode=0;
			am_wifi->mode = arg;
			if( am_wifi->mode == 8 )
				am_wifi->mode =1;
			//copy_from_user(&am_wifi->mode,(void*)arg,1);
			DBG_PIPE_MSG("new wifi mode = %d\n",am_wifi->mode);
			break;
		case CMD_SET_WIFI_CONFIG://4
			memset(am_wifi, 0,sizeof(struct am_wifi_status));		
			copy_from_user(am_wifi,(void*)arg,sizeof(struct am_wifi_status));
			DBG_PIPE_MSG("new wifi mode==%d \n",am_wifi->mode);
			DBG_PIPE_MSG("new IP==%s \n",am_wifi->IP_addr);
			DBG_PIPE_MSG("new SSID==%s \n",am_wifi->SSID);			
			DBG_PIPE_MSG("new PSK==%s \n",am_wifi->PSK);
			DBG_PIPE_MSG("new security==%c \n",am_wifi->security);
			DBG_PIPE_MSG("new projector_ID==%d \n",am_wifi->projector_ID);
			DBG_PIPE_MSG("new modelname==%d \n",am_wifi->modelname);
			break;
		case CMD_GET_PROJECTOR_STATUS://目前有無人在投影,5
			copy_to_user((void*)arg,(void*)&projector_status,sizeof(int));
			break;
		case CMD_GET_CONNECT_USERS://多少使用者連上投影機,6
			copy_to_user((void*)arg,(void*)&Users,sizeof(int));
			//hostapd_cli -p/tmp/hostapd all_sta
			break;
		case CMD_GET_PROJECTOR_ID://7
			DBG_PIPE_MSG("new projector_ID==%d \n",am_wifi->projector_ID);
			copy_to_user((void*)arg,(void*)&am_wifi->projector_ID,sizeof(int));
			break;
		case CMD_SET_PROJECTOR_ID://8
			am_wifi->projector_ID = arg;
			key = 0x91;
			break;
		case CMD_SET_CONNECT_USERS://9
			Users = arg;
			break;
		case CMD_SET_CHANNEL_AVAILABLE://10
			channel = arg;
			key = 0x92;
			break;
		case CMD_GET_CHANNEL_AVAILABLE://11
			copy_to_user((void*)arg,&channel,sizeof(int));
			break;
		case CMD_SET_PROJECTOR_STATUS://12
			projector_status = arg;
			break;
		case CMD_SET_LAN_PORT_FLAG://13
			lan_in_flag =  arg;
			key = 0x93;
			break;
		case CMD_GET_LAN_PORT_FLAG://14
			copy_to_user((void*)arg,&lan_in_flag,sizeof(int));
			break;
		case CMD_SET_LAN_IN_FLAG://15
			eth0_flag = arg;
			break;
		case CMD_GET_LAN_IN_FLAG://16
			copy_to_user((void*)arg,&eth0_flag,sizeof(int));
			break;
		case CMD_OTA_CHECK_FLAG://17
			key = 0x94;
			break;
		case CMD_SET_OTA_PERCENTAGE://18
			OTA_percentage = arg;
			break;
		case CMD_GET_OTA_PERCENTAGE://19
			copy_to_user((void*)arg,&OTA_percentage,sizeof(int));
			break;
		case CMD_SET_OTA_STATUS://20
			OTA_status = arg;
			break;
		case CMD_GET_OTA_STATUS://21
			copy_to_user((void*)arg,&OTA_status,sizeof(int));
			break;
		case CMD_SET_OTA_UPDATE_NEED://22
			OTA_update_need = arg;
			break;
		case CMD_GET_OTA_UPDATE_NEED://23
			copy_to_user((void*)arg,&OTA_update_need,sizeof(int));
			break;
		case CMD_OTA_FLAG://24
			key = 0x95;
			break;
		case CMD_SET_OTA_VERSION://25
			copy_from_user(OTA_version,(void*)arg,7);
			DBG_PIPE_MSG("SET_OTA_version:%s\n",OTA_version);
			break;
		case CMD_GET_OTA_VERSION://26
			copy_to_user((void*)arg,&OTA_version,7);
			DBG_PIPE_MSG("GET_OTA_version:%s\n",OTA_version);
			break;
		case CMD_OTA_CANCEL://27
			key = 0x96;
			break;
		case CMD_OTA_SET_CHECKSUM://28
			OTA_checksum_ok = arg;
			break;
		case CMD_OTA_GET_CHECKSUM://29
			copy_to_user((void*)arg,&OTA_checksum_ok,sizeof(int));
			key = 0x97;
			break;
		case CMD_OTA_START_UPGRADE://30
			key = 0x97;
			break;
		case CMD_SET_AM_VERSION://31
			copy_from_user(AM_version,(void*)arg,7);
			break;
		case CMD_GET_AM_VERSION://32
			copy_to_user((void*)arg,&AM_version,7);
			break;
		case CMD_SET_DHCP_ON_OFF://33
			DHCP_on_off = arg;
			DBG_PIPE_MSG("CMD_SET_DHCP_ON_OFF:%d\n",DHCP_on_off);
			key = 0x98;
			break;
		case CMD_GET_DHCP_ON_OFF://34
			DBG_PIPE_MSG("CMD_GET_DHCP_ON_OFF:%d\n",DHCP_on_off);
			copy_to_user((void*)arg,&DHCP_on_off,sizeof(int));
			break;
		case CMD_WAKE_UP_SCALER://35
			DBG_PIPE_MSG("wake up scaler...gpio 81 high\n");
			INT32U* gpio3 = (INT32U*)(GPIO_MFCTL3);
			*gpio3 = (*gpio3 & 0xFFFFFFFC);
			am_set_gpio(81,0);
			mdelay(100);
			DBG_PIPE_MSG("wake up scaler...gpio 81 low\n");			
			am_set_gpio(81,1);			
			break;
	}
	if(key!=0){
		DBG_PIPE_MSG("key: 0x%02x\n", key);	
		send_key_press(FUI_KEYACTION_DOWN_MASK,key);
	}
	return 0;
}

static int am7x_pipe_open(struct inode *pinode, struct file *pfile)
{
	DBG_PIPE_MSG("am_pipe_dev open\n");
	return 0;
}

static int am7x_pipe_release(struct inode *pinode, struct file *pfile)
{
	DBG_PIPE_MSG("am_pipe_dev release\n");
	return 0;
}

static struct file_operations pipe_drv_fops=
{
	.owner  = THIS_MODULE,
//	.read = am_uartcom_read,
//	.write = am_uartcom_write,
//	.poll = am_uartcom_poll,
	.ioctl = am7x_pipe_ioctl,
	.open = am7x_pipe_open,
	.release = am7x_pipe_release,
};


static INT32S  __init am7x_pipe_init(void)
{
	INT32S result=0;

	DBG_PIPE_MSG("*********am7x_pipe init\n");

	pipe_dev =MKDEV(234,0);
	result = register_chrdev_region(pipe_dev,1,"am7x_pipe_drv");

	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for uartcom\n");
		return -EIO;
	}
	DBG_PIPE_MSG("uartcom major=%d, minor=%d\n",MAJOR(pipe_dev),MINOR(pipe_dev));

	pipe_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!pipe_cdev){
		printk(KERN_ERR "malloc memory  fails for pipe_cdev device\n");
		unregister_chrdev_region(pipe_dev,SYSMSG_MAX_DEVS);
		return -ENOMEM;
	}
  	cdev_init(pipe_cdev, &pipe_drv_fops);
	if(cdev_add(pipe_cdev, pipe_dev, 1))
		goto out_err;

	pipe_dev_init();
		
	return 0;
out_err:
	printk(KERN_ERR "register failed  for sysmsg device\n");
	kfree(pipe_cdev);
	unregister_chrdev_region(pipe_dev,SYSMSG_MAX_DEVS);
	return -ENODEV;

}

static void __exit am7x_pipe_exit(void)
{
	if(pipe_cdev){
		cdev_del(pipe_cdev);
		kfree(pipe_cdev);
	}
	unregister_chrdev_region(pipe_dev,1);
}

module_init(am7x_pipe_init);
module_exit(am7x_pipe_exit);

MODULE_AUTHOR("Richard");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Actions-micro pipe device");
