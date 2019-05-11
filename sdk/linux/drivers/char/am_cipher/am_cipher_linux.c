/**********************************************************************
* Actions Micro cipher module.
* 
*/

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_pfn_range
	SetPageReserved
	ClearPageReserved
*/
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>

#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
#include <linux/interrupt.h>

/* needed for virt_to_phys() */
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/syscalls.h>
#include <linux/major.h>
#include <linux/device.h>

#include<linux/delay.h>
#include "hdcp2_hal.h"
#include <am7x_flash_api.h>
#include "AES128.h"

#ifdef CONFIG_MTD_M25P80
#include "sys_buf_def.h"
#define SNOR_FLASH			//8252n,snor
void nand_adfu_open();
void nand_adfu_close();
#endif


#define CIPHER_CMD_RSA256 0


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Actions-Micro");
MODULE_DESCRIPTION("driver module for Actions-Mircro cipher");

typedef struct 
{
	unsigned char		HDCPLC[16];
	unsigned char		HDCPKeyData[842];
	unsigned char  	HDCPKeyDataReserved[2];		//must be 0
	unsigned int    	HDCPKeyChecksum;			//DWORD sum of HDCPKeyData
}HDCPKey_Info;


typedef struct 
{
	HDCPKey_Info  HDCPEncryptKey;
	unsigned char  Reseverd[136];
	unsigned char  SoftwareKey[16];
	unsigned int    EncryptMode;
	unsigned int    Checksum;
}EncryptHDCP;		/*1k*/

typedef struct {
	uint8_t *key;
	uint8_t *iv;
} AES_para_t;	

typedef struct {
	unsigned int len;	
	uint32_t in;
	uint32_t out;
} AES_data_t;



static unsigned char priv_key[320];
static unsigned char encryp_data[128];
static unsigned char decrypt_data[16];

struct timer_list am7x_cipher_timer;


static int am7x_cipher_major = 0;

static struct semaphore am7x_cipher_sem;

extern H2status Decrypt_EKpubKm_kPrivRx( const H2uint8* KprivRx, H2uint8* km, const H2uint8 *EKpubKm);

static void am7x_cipher_timer_do(void)
{
	mod_timer(&am7x_cipher_timer, jiffies+0x0fffffff);
	Decrypt_EKpubKm_kPrivRx( priv_key,decrypt_data,encryp_data);
	up(&am7x_cipher_sem);
}

static void dump_read_data(const unsigned char *pdata,unsigned int len)
{

#if 0
	unsigned int tmp=0;
	while(len--){
		if(tmp%16 == 0){
			printk("\n%02x",*(pdata+tmp));
		}else{
			printk(" %02x",*(pdata+tmp));	
		}
		tmp++;
	}
	printk("\n");
#endif
}
unsigned int   cal_checksum(char *ptr,   int len)   
{   
	int i;
	unsigned int *D_ptr=(unsigned int *)ptr;
	unsigned int CheckSum=0;
	
	for(i=0;i<len/sizeof(unsigned int);i++)
		CheckSum += *D_ptr++;
	
	return CheckSum;
}

static void XOR_encrypt(unsigned int *KEY,unsigned int *plaintext,int len,unsigned int *crypto_data )
{
	unsigned int *crypto_dataP=crypto_data;
	unsigned int *plaintextP=plaintext;
	unsigned int *KEYP=KEY;
	int i;
	for (i =0;i<(len/16);i++)
	{
		*crypto_dataP++=*plaintextP++^*KEYP++;
		*crypto_dataP++=*plaintextP++^*KEYP++;
		*crypto_dataP++=*plaintextP++^*KEYP++;
		*crypto_dataP++=*plaintextP++^*KEYP++;
		KEYP=KEY;
	}
}


static int AESDecrypt(char *EncryptKey,char *DecryptKey,int len)
{
	int ret1 ,ret2;
	int val,i;
	int templen;
	INT32U * Encaddr,*Decaddr,RET;

	Encaddr = (INT32U *)EncryptKey;
	Decaddr = (INT32U *)DecryptKey;

	dma_cache_wback_inv((INT32U)((INT32U)Encaddr &~MASK_COST_Read),len);

	templen =len;
	do{
		{
			act_writel(*Encaddr++,AES_TEXT_IN);
			act_writel(*Encaddr++,AES_TEXT_IN);
			act_writel(*Encaddr++,AES_TEXT_IN);
			act_writel(*Encaddr++,AES_TEXT_IN);
		}
		while(!(act_readl(AES_CTL)&0x1000))
		{
			;
		}
		udelay(1000);

		{
			*Decaddr++=act_readl(AES_TEXT_OUT);
			*Decaddr++=act_readl(AES_TEXT_OUT);
			*Decaddr++=act_readl(AES_TEXT_OUT);
			*Decaddr++=act_readl(AES_TEXT_OUT);
		}
		act_writel((act_readl(AES_CTL)|0x71101),AES_CTL);
	}while(templen -=16 );
	
	return H2_OK;

}
static int DoDecryptHDCPKey(char *EncryptKey,char *DecryptKey,int len)
{
	EncryptHDCP *	EncryptHDCPData;

	EncryptHDCPData=(EncryptHDCP *)EncryptKey;
	if(2==EncryptHDCPData->EncryptMode)
	{
		XOR_encrypt((unsigned int *)EncryptHDCPData->SoftwareKey,(unsigned int *)EncryptKey,len,(unsigned int *)DecryptKey);
	}
	else 
	{
		printk("EncryptMode=%d\n",EncryptHDCPData->EncryptMode);
		act_writel(act_readl(CMU_DEVRST2)&(~0x100), CMU_DEVRST2);		//AES reset	
		act_writel(act_readl(CMU_DEVRST2)|0x100, CMU_DEVRST2);	 	//AES reset	
		act_writel(act_readl(0xb001007c)|0x10, 0xb001007c);				//AES enable

		//hardware AES Decrypt
		if(1==EncryptHDCPData->EncryptMode)
		{//use software AES Key
			act_writel(*(unsigned int*)(EncryptHDCPData->SoftwareKey+12),AES_KEY_P0);
			act_writel(*(unsigned int*)(EncryptHDCPData->SoftwareKey+8),  AES_KEY_P1);
			act_writel(*(unsigned int*)(EncryptHDCPData->SoftwareKey+4),  AES_KEY_P2);
			act_writel(*(unsigned int*)(EncryptHDCPData->SoftwareKey),      AES_KEY_P3);
			
			act_writel(0x71111,AES_CTL);
			while((act_readl(AES_CTL)&0x10))
			{
				;
			}
		}
		else
		{//use chipID as AES Key
			act_writel(0x71131,AES_CTL);
		}
		AESDecrypt(EncryptKey,DecryptKey,len);
	}

	return H2_OK;

}

int AES_DecryptInit(uint8_t *key, uint8_t *iv);
int AES_DecryptBlock(uint8_t *data, int len, uint8_t *out);
int AES_DecryptBlock_Finish(int timeout);

static int am7x_cipher_ioctl(struct inode *inode, struct file *filp,
						  unsigned int cmd, unsigned long arg)
{
	/** to be done */
	if(0==cmd)
	{
		//Decrypt HDCP Key
		EncryptHDCP *EncryptHDCPData;
		HDCPKey_Info *DecryptHDCPKey;
		char *EncryptKey;
		char *DecryptKey;
		char *tempbuf1;
		char *tempbuf2;
		unsigned int HDCPKeyLba;
		unsigned int checksum;
		H2status rc = H2_ERROR;

		tempbuf1=kmalloc(2048, GFP_KERNEL);
		if(tempbuf1 == NULL){
			return rc;
		}
		tempbuf2=kmalloc(2048, GFP_KERNEL);
		if(tempbuf2 == NULL){
			kfree(tempbuf1);
			return rc;
		}

		EncryptKey=((unsigned int)tempbuf1+0x3ff)&0xfffffc00;	
		DecryptKey=((unsigned int)tempbuf2+0x3ff)&0xfffffc00;

		//printk("tempbuf1=%x,tempbuf2=%x,EncryptKey=%x,DecryptKey=%x\n\n",tempbuf1,tempbuf2,EncryptKey,DecryptKey);
#ifdef SNOR_FLASH	
		nand_adfu_open();
		nand_adfu_read((SNOR_BREC_OFFSET+SNOR_BREC_SIZE)/512,(void *)EncryptKey,1);			//read LFI
#else
		nand_adfu_read(0,(void *)EncryptKey,1);			//read LFI
#endif

		HDCPKeyLba=	*(unsigned int*)(EncryptKey+0x90);	//mbr lba
		if(0==HDCPKeyLba)
			HDCPKeyLba=0x10000;
		HDCPKeyLba+=2;
#ifdef SNOR_FLASH	
		nand_adfu_read(HDCPKeyLba+(SNOR_BREC_OFFSET+SNOR_BREC_SIZE)/512,(void *)EncryptKey,2);	//read 2 sector from snor
		nand_adfu_close();	//snor
#else
		nand_adfu_read(HDCPKeyLba,(void *)EncryptKey,2);	//read 2 sector from nand
#endif		
		checksum=cal_checksum(EncryptKey,1020);

		EncryptHDCPData=(EncryptHDCP *)EncryptKey;
		if((0==checksum)||(checksum!=EncryptHDCPData->Checksum))
		{
			printk("no valid HDCPKey in NandFlash\n");
			dump_read_data(EncryptKey,1024);
			goto DecryptHDCPEnd;
		}

		if(H2_ERROR==DoDecryptHDCPKey(EncryptKey,DecryptKey,sizeof(HDCPKey_Info)))
			goto DecryptHDCPEnd;

		checksum=cal_checksum(DecryptKey,offsetof(HDCPKey_Info,HDCPKeyChecksum));
		DecryptHDCPKey=(HDCPKey_Info *)DecryptKey;
		if(checksum!=DecryptHDCPKey->HDCPKeyChecksum)
		{
			printk("bad HDCPKey,num=%d\n",offsetof(HDCPKey_Info,HDCPKeyChecksum));
			printk("EncryptKey:\n");
			dump_read_data(EncryptKey,1024);
			printk("DecryptKey:\n");
			dump_read_data(DecryptKey,sizeof(HDCPKey_Info));
			goto DecryptHDCPEnd;
		}
		//dma_cache_wback((INT32U)((INT32U)DecryptKey &~MASK_COST_Read),1024);

		copy_to_user(arg,DecryptKey,sizeof(HDCPKey_Info));

		rc=H2_OK;

DecryptHDCPEnd:
		kfree(tempbuf1);
		kfree(tempbuf2);
		return rc;
	}
	else if(1==cmd)
	{
		AES_para_t AES_para;
		unsigned char key[16];
		unsigned char iv[16];
		
		copy_from_user(&AES_para, ( AES_para_t* )arg, sizeof(AES_para_t));
		copy_from_user(iv, ( unsigned char*)AES_para.iv, 16);
		copy_from_user(key, ( unsigned char*)AES_para.key, 16);

		AES_DecryptInit(key,iv);
	}
	else if(2==cmd)
	{
		AES_data_t AES_data;
		int len;
		uint8_t *data;
		uint8_t *out;

		copy_from_user(&AES_data, ( AES_data_t* )arg, sizeof(AES_data_t));
		
		len=AES_data.len;
		data=AES_data.in;
		out=AES_data.out;
		
		AES_DecryptBlock(data, len, out);
	}
	else if(3==cmd)
	{
		return AES_DecryptBlock_Finish(arg);
	}
	return 0;
}

static int am7x_cipher_open(struct inode *inode, struct file *filp)
{
	/** to be done */
	return 0;
}


static ssize_t am7x_cipher_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
	long jiffies = msecs_to_jiffies(3000);
	
	if(down_timeout(&am7x_cipher_sem, jiffies)){
		printk("%s,down error\n",__FUNCTION__);
		return 0;
	}
	del_timer(&am7x_cipher_timer);
	copy_to_user(buf,decrypt_data,16);
	return 16;
}

static ssize_t am7x_cipher_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	char *data;

	data = kmalloc(size, GFP_KERNEL);
	if(data == NULL){
		return 0;
	}
	
	copy_from_user(data, buf, size);

	if(data[0]==CIPHER_CMD_RSA256){
		/** HDCP2 RSA decryption */
		memcpy(priv_key,data+1,320);
		memcpy(encryp_data,data+1+320,128);
		kfree(data);
		
		/** start the timer to do it */
		init_timer(&am7x_cipher_timer);
		am7x_cipher_timer.expires = jiffies+5;
		am7x_cipher_timer.function = (void*)am7x_cipher_timer_do;
		add_timer(&am7x_cipher_timer);
		return size;
	}

	return 0;
}


/* VFS methods */
static struct file_operations am7x_cipher_fops = {
	open:am7x_cipher_open,
	read:am7x_cipher_read,
	ioctl:am7x_cipher_ioctl,
	write:am7x_cipher_write,
};

struct cdev *am7x_cipher_cdev;
struct class *am7x_cipher_class;
dev_t am7x_cipher_dev;
static int am7x_cipher_load()
{
	int result=-1;
	int count=1;

	if(am7x_cipher_major)
	{
		am7x_cipher_dev = MKDEV(am7x_cipher_major, 0);
		result = register_chrdev_region(am7x_cipher_dev, count, "am7x-cipher");
	}
	else
	{
		result = alloc_chrdev_region(&am7x_cipher_dev, 0, count, "am7x-cipher");
		am7x_cipher_major = MAJOR(am7x_cipher_dev);
	}

	am7x_cipher_cdev = cdev_alloc();
	cdev_init(am7x_cipher_cdev,&am7x_cipher_fops);
	am7x_cipher_cdev->owner = THIS_MODULE;
	
	result = cdev_add(am7x_cipher_cdev, am7x_cipher_dev, count);
	if(result < 0)
	{
		printk("can't add [%s]dev", __func__);
		unregister_chrdev_region(am7x_cipher_dev, count);
		cdev_del(am7x_cipher_cdev);
		return result;
	}

	am7x_cipher_class = class_create(THIS_MODULE, "am7x_cipher_class");
	if(IS_ERR(am7x_cipher_class)) {
		unregister_chrdev_region(am7x_cipher_dev, count);
		cdev_del(am7x_cipher_cdev);
		printk("am7x_cipher Err: failed in creating class.\n");
		return -1; 
	}

	device_create(am7x_cipher_class, NULL, am7x_cipher_dev, NULL, "am7x-cipher");

	sema_init(&am7x_cipher_sem, 0);

	printk( "am7x-cipher: module inserted. Major = %d\n", am7x_cipher_major);

	return 0;
	
}

int __init am7x_cipher_init(void)
{
	int err;

	err = am7x_cipher_load();
	
	return err;
}
/*------------------------------------------------------------------------------
	Function name	: hx170dec_cleanup
	Description 	: clean up

	Return type 	: int
------------------------------------------------------------------------------*/
static void am7x_cipher_unload()
{
	if(am7x_cipher_cdev)
		cdev_del(am7x_cipher_cdev);
	
	if(am7x_cipher_class){
		device_destroy(am7x_cipher_class, am7x_cipher_dev);    //delete device node under /dev//必须先删除设备，再删除class类
		class_destroy(am7x_cipher_class);                 //delete class created by us
	}
	
	if(am7x_cipher_dev)
		unregister_chrdev_region(am7x_cipher_dev, 1);

	printk(KERN_INFO "am7x-cipher: module removed\n");
	
	return;
}

void __exit am7x_cipher_cleanup(void)
{
	am7x_cipher_unload();
	return;
}


module_init(am7x_cipher_init);
module_exit(am7x_cipher_cleanup);



