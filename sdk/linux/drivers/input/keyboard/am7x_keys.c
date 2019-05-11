/*
 * Driver for Kinds of keys beyond AM_CHIP_X capable of generating interrupts.
 *
 * Copyright 2010 Kewen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include "actions_regs.h"
#include "actions_io.h"

#include <sys_msg.h>
#include "sys_cfg.h"
#include <am7x_board.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include "am7x_gpio.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif

#define EZ_OFFSET		48

#define AM_KEY_MAJOR  254
#define IR_MAX_DEVS  2
#define GPIO_CFG_NUM 5
#define ADC_CFG_NUM 15
#define DEFAULT_GPIO_NUM 0xff
#define IR_USERCODE_E	0x20df

#define KEY_SCAN_PERIOD	10

#define KBD_CONFIG_FILE		"/am7x/case/data/keyDriver.bin"
#define PLATFORM_KBDDEVICE_NAME	"key-am7x"

enum{
	ADC_KEY,
	GPIO_KEY
};

//#define  DBG_AM_KEY

#ifdef  DBG_AM_KEY
#define KEY_DBG_MSG(format,args...)   printk(format, ##args)
#else
#define KEY_DBG_MSG(format,args...)   do {} while (0)
#endif

#if (MODULE_CONFIG_EZWILAN_ENABLE && MODULE_CONFIG_EZCASTPRO_MODE!=8075)
enum {
	GPIO_NULL_VAL = 0,
	GPIO_ACTION_VAL = 1
};
#else
enum {
	GPIO_ACTION_VAL = 0,
	GPIO_NULL_VAL = 1
};
#endif

typedef struct _gpio_info{
	unsigned short bcode;
	unsigned short gpionum;
}gpio_info;

typedef struct _ir_Info{
	unsigned short usrcode;
	unsigned short irhwcode;
}ir_info;

key_cfg_info kcfg;
unsigned short *ir_key=NULL;
gpio_info* gpio_key=NULL;

unsigned int adc_key[16];
unsigned char gpioTotalNum=0;
unsigned char bktotalNum=0;
unsigned char irTotalNum=0;
static struct input_dev *am_key_dev;

struct cdev  *ir_cdev=NULL;
dev_t  ir_dev;

#define KC_HD_I	(1<<16)
#define KC_UP_I	(1<<17)
#define KC_DN_I	(1<<18)
#define KC_WH_E	(1<<19)
#define KC_HD_E	(1<<20)
#define KC_UP_E	(1<<21)
#define KC_DN_E	(1<<22)
#define KC_DF_F	(1<<23)
#define KC_NUM(x)	((x)<<24)
#define KC_HD_T(x)	((x)<<28)


struct timer_list k_timer;
struct timer_list ir_hold_timer;
static unsigned int adc_val,cur_key,save_key,key_stored=0,key_count=0,bk_pressed=0;
static unsigned int   gpio_valid_key=0,ir_key_temp=0;


#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))              
#define IR_MFL_MASK_OFFSET 14
#define IR_MFL_MASK_WIDTH 3
#define IR_MFL_MASK_VALUE 3
#define IR_MFL_MASK (~((1<<(IR_MFL_MASK_WIDTH)-1)<<IR_MFL_MASK_OFFSET))
#define GUI_Ir_Hold			0xC000

#define IR_FIFO_NUM 10
#define IR_REPEAT_CODE 0xC0
#define IR_START_CODE   0xC0
#define IR_END_CODE      0xC0
static unsigned char cCodeH,cCodeL;
static unsigned int rFifo[IR_FIFO_NUM];
static unsigned char fifoIndex=0;
unsigned int key_save;
static unsigned short uart_ir_key;
static unsigned char irkey_flag=0;
static unsigned char short_press = 0;

//extern 	INT32U	PHY_NandReset(INT32U ChipNum);
static unsigned int save_remote_ctl = 0;


void am_set_mfl(unsigned int num,unsigned int value)
{
	volatile unsigned int*  reg=0;

	reg = (volatile unsigned int*)(GPIO_MFCTL0+num*4);
	*reg = value;
}

unsigned int am_get_mfl(unsigned int num)
{
	volatile unsigned int*  reg=0;

	reg = (volatile unsigned int*)(GPIO_MFCTL0+num*4);
	return *reg;
}

/**
 * @brief Get the gpio key.
 *
 * The gpio key is config by the linuxtool
 * @return gpio key value
 * - 0 : no key
 * - other :  key value
 */
static unsigned int check_gpio_key(void)
{
	unsigned int i=0 ;
	for(i=0; i<gpioTotalNum; i++)
	{	
		//printk("<%s %d> gpionum[%d]=%d\n",__FUNCTION__,__LINE__,i,gpio_key[i].gpionum);
		if(DEFAULT_GPIO_NUM != gpio_key[i].gpionum)
			//printk("<%s %d> value=%02x\n",__FUNCTION__,__LINE__,am_get_gpio(gpio_key[i].gpionum));
			if(GPIO_ACTION_VAL == am_get_gpio(gpio_key[i].gpionum))
			{
				//printk("<%s %d> bcode=%d\n",__FUNCTION__,__LINE__,gpio_key[i].bcode);
				return gpio_key[i].bcode;
			}
	}
	
	return 0;
}

static unsigned short check_ir_key(unsigned short irkey)
{
	int i;
	for(i=0;i<irTotalNum;i++)
	{
		if(ir_key[i] == irkey)
		{
			return i+bktotalNum+1;
		}
	}
	
	return 0;
}
void save_uart_irkey(unsigned short input)
{
	KEY_DBG_MSG("%x\n",input);
	irkey_flag = 1;
	uart_ir_key = (unsigned short)input;
}



void send_uart_irkey(unsigned short input,unsigned char updn)
{
	KEY_DBG_MSG(KERN_INFO "%x\n",input);
	if(updn)
	{
		ir_key_temp = check_ir_key(input&0x00ff);// | 0x100
		printk(KERN_INFO "send ir key down uart_ir_key %x,ircode=%x\n",ir_key_temp,input);
		input_report_key(am_key_dev, ir_key_temp, 1);

	}
	else
	{
		KEY_DBG_MSG(KERN_INFO "send ir key up uart_ir_key %x\n",input);
		input_report_key(am_key_dev, ir_key_temp, 0);
		input_sync(am_key_dev);
		ir_key_temp = 0;
	}
}
static void ir_hold_detect_isr(void)
{
	KEY_DBG_MSG("short_press clear\n");
	short_press = 0;
	fifoIndex = 0;
	mod_timer(&ir_hold_timer, jiffies+0x0fffffff);
	send_uart_irkey(ir_key_temp, 0);
}
static irqreturn_t ir_remote_isr(void)
{
	unsigned int i;

	if(fifoIndex>=IR_FIFO_NUM) 
	{
		fifoIndex=0;
		for(i=0;i<IR_FIFO_NUM;i++)
			rFifo[i] = 0;
	}

	while(!(act_readl(IR_REMOTE_STATUS) &0x20)){
		rFifo[fifoIndex]= act_readl(IR_REMOTE_RXDATA) & 0xff;
		//printk(KERN_INFO "IR_Rxdata==%x,fifoIndex = %d\n",rFifo[fifoIndex],fifoIndex);
		fifoIndex++;
	}

	if(short_press)
	{
		mod_timer(&ir_hold_timer,jiffies+30);
	}
	else if(fifoIndex>4)
	{
		for(i=0;i<fifoIndex-2;i++)
		{
			if(rFifo[i]==cCodeH && rFifo[i+1]==cCodeL)
			{	
				if((rFifo[i+2] & rFifo[i+3])==0x00)//check anti-code
				{
					key_save = rFifo[i+2];
					send_uart_irkey(key_save,1);
					short_press = 1;
					fifoIndex = 0;
					for(i=0;i<IR_FIFO_NUM;i++)
						rFifo[i] = 0;
					mod_timer(&ir_hold_timer,jiffies+30);
					//printk("key_save = %x\n",key_save);
					break;
				}
				else
				{
					printk("invalid ir code\n");
					fifoIndex = 0;
					for(i=0;i<IR_FIFO_NUM;i++)
						rFifo[i] = 0;
					break;
				}
				
			}
		}
	}
	

	if(act_readl(IR_REMOTE_STATUS) & 1)
		IRQ_CLEAR_PENDING(1,IR_REMOTE_STATUS); // clear IRQ pending bit 

	return IRQ_HANDLED;

}

static int  ir_remote_init(void)
{

	int rc;
	printk(KERN_INFO "start ir device driver\n");
	//KEY_DBG_MSG("multi function %d === 0x%x\n",6,am_get_mfl(6));

#if CONFIG_AM_CHIP_ID == 1213
	// set multifunc pin for 1213
	//act_writel((act_readl(GPIO_MFCTL4)&(~0x7))|0x3, GPIO_MFCTL4);
	printk("GPIO_MFCTL4 = 0x%x\n",act_readl(GPIO_MFCTL4));
#endif

	act_writel(0x00048410, IR_REMOTE_CTRL);
	rc = request_irq(IRQ_IRREMOTE, (irq_handler_t)ir_remote_isr, 0, "ir_remote", NULL);
	if(rc < 0){
		printk(KERN_ERR "request irq error\n");
		return rc;
	}else
		printk(KERN_INFO "request irq ok\n");

	init_timer(&ir_hold_timer);
	ir_hold_timer.expires = jiffies+0x0fffffff;
	ir_hold_timer.function = (void*)ir_hold_detect_isr;
	add_timer(&ir_hold_timer);
	
	printk(KERN_INFO "custome code : H=0x%x,L=0x%x\n",cCodeH,cCodeL);
	
	
	return 0;
}


void ir_remote_exit(void)
{
	free_irq(IRQ_IRREMOTE, NULL);
	del_timer(&ir_hold_timer);
}



void clear_key_status(void)
{
	key_stored=0;
	cur_key=0;
	save_key=0;
	key_count=0;
	gpio_valid_key = 0;
	bk_pressed = 0;
}
static int gpio_key_debounce(void)
{
	if(cur_key)
	{    
		key_count++;
		if( key_count >= 2)
		{
			gpio_valid_key = cur_key;

			key_stored = 1;
			if(bk_pressed==0)
			{
				bk_pressed = 1;
				printk("<%s %d>\n",__FUNCTION__,__LINE__);
				printk(KERN_INFO "send KEY_DOWN,k=%x\n",gpio_valid_key);
				input_report_key(am_key_dev, gpio_valid_key, 1);	
			}
			
		}	
		else
		{
			key_stored = 0;
			gpio_valid_key = cur_key;
		}
	}
	else
	{
		if(gpio_valid_key)
		{
			if(key_stored)
			{
				if(key_count>=2)
				{
					KEY_DBG_MSG(KERN_INFO "send KEY_UP,k=%x\n",gpio_valid_key);
					//input_report_key(am_key_dev, gpio_valid_key, 1);
					input_report_key(am_key_dev, gpio_valid_key, 0);
					input_sync(am_key_dev);
					clear_key_status();	
				}
				else
				{
					clear_key_status();
				}
				
			}
			else
			{
				clear_key_status();
			}
				
		}
		else
		{
			clear_key_status();
		}
	}
	
	mod_timer(&k_timer,jiffies +10);
	return 0;
}

#ifndef CONFIG_AM_8251
static void key_scan(void)
{
	INT8U iLoop = 0;
	cur_key = 0;

	//gpio key scan start
	cur_key = check_gpio_key();
	
	if(cur_key||gpio_valid_key)
	{	
		//printk("<%s %d>\n",__FUNCTION__,__LINE__);
		gpio_key_debounce();
		return;
	}
	//adc key scan start
	adc_val=((act_readl(KEY_VALUE) >> 24)&0xf);

	if(0xf != adc_val)
	{
		printk("-----key = %d\n",adc_val);
		cur_key =  adc_val+1;
	}

	if(cur_key)
	{    
	
		key_count++;
		if( key_count >= 2)
		{
			save_key = (save_key<cur_key)? save_key : cur_key;

			//save_key = cur_key; 
			key_stored = 1;
			if(bk_pressed==0)
			{
				bk_pressed = 1;
				printk(KERN_INFO "send KEY_DOWN,k=%x\n",adc_key[save_key-1]);
				input_report_key(am_key_dev, adc_key[save_key-1], 1);	
			}
			
		}	
		else
		{
			key_stored = 0;
			save_key = cur_key;
		}
	}
	else
	{
		if(save_key)
		{
			if(key_stored)
			{
				if(key_count>=2)
				{
					//value = 1-value;	
					KEY_DBG_MSG(KERN_INFO "send KEY_UP,k=%x\n",adc_key[save_key-1]);
					//input_report_key(am_key_dev, adc_key[save_key-1], 1);
					input_report_key(am_key_dev, adc_key[save_key-1], 0);
					input_sync(am_key_dev);
					clear_key_status();	
				}
				else
				{
					clear_key_status();
				}
				
			}
			else
			{
				clear_key_status();
			}
				
		}
		else
		{
			clear_key_status();
		}
	}

	mod_timer(&k_timer,jiffies +10);
}
#else
static void key_scan(void){
	cur_key = 0;
	//gpio key scan start
	cur_key = check_gpio_key();
	
	if(cur_key||gpio_valid_key)
	{	
		gpio_key_debounce();
		return;
	}
	mod_timer(&k_timer,jiffies +10);
}
#endif
static int am7x_keytype_cfg(struct input_dev *dev,void* param)
{
	int totalitem=0;
	key_cfg_info *kcfg=NULL;
	
	if(dev->keycode)
		return -1;
	if(param==NULL)
		kcfg = (key_cfg_info*)dev->keyparam;
	else 
		kcfg = (key_cfg_info*)param;
	
	if(kcfg->typeused&0x01)
	{
		if(kcfg->bkeytype==1)
		{
			totalitem = kcfg->bkeynum.tbknum;;
		}
		else if(kcfg->bkeytype==2)
		{
			totalitem = kcfg->bkeynum.matrixbk.ihor + kcfg->bkeynum.matrixbk.iver;
		}
	}

	if(kcfg->typeused&0x02)
	{
		totalitem += kcfg->irkeynum;
		irTotalNum = kcfg->irkeynum; 
	}
	dev->keycodemax = totalitem;
	dev->keycode = (key_map_info*)kmalloc(totalitem*sizeof(key_map_info),GFP_KERNEL);

	cCodeH=(kcfg->irusrcode>>8)&0xff;
	cCodeL=(kcfg->irusrcode)&0xff;
	return 0;
}

static int am7x_key_encoder(struct input_dev *dev,void* param)
{
	key_map_info *key_info=NULL;
	int i,j;
	unsigned int temp[ADC_CFG_NUM]={0};
	printk("am7x_keycode_open\n");
	
	//if(dev->keycode==NULL)
		//return -1;
	if(param==NULL)
		key_info = (key_map_info*)dev->keycode;
	else 
		key_info = (key_map_info*)param;	
	/** init adc key config data */
	memset(adc_key,0,sizeof(int)*ADC_CFG_NUM);
	for(i=0;i<dev->keycodemax;i++)
	{
		if(key_info[i].keytype==1)
		{
			//j=;
			for(j=key_info[i].keyvalue.adczone.iadcfrom;j<=key_info[i].keyvalue.adczone.iadcto;j++)
			{
				temp[j] = i+1;//key code start from 0x01
			}
		}
		else if(key_info[i].keytype==2)
		{
			gpioTotalNum++;
		}
	}
	memcpy(adc_key,temp,sizeof(int)*ADC_CFG_NUM);

	
	/** init gpio key config data */
	gpio_key =kmalloc(gpioTotalNum*sizeof(gpio_info),GFP_KERNEL);
	j=0;
	for(i=0;i<dev->keycodemax;i++)
	{
		if(key_info[i].keytype==2)
		{
			gpio_key[j].bcode = i+1;//key code start from 0x01
			gpio_key[j].gpionum = (unsigned short)key_info[i].keyvalue.hwcode;
			//printk("gpio_key[%d],i=%d,gpionum=%d",j,i,gpio_key[j].gpionum);
			j++;
		}
	}

	
	/** init ir key config data */
	ir_key =kmalloc(irTotalNum*sizeof(short),GFP_KERNEL);
	j=0;
	for(i=0;i<dev->keycodemax;i++)
	{
		if(key_info[i].keytype==3)
		{
			ir_key[j] = (unsigned short)key_info[i].keyvalue.hwcode;
			j++;		
		}
	}
#ifndef CONFIG_AM_8251
	/** register each kind of the key into the input subsystem*/
	for (i = 0; i < ADC_CFG_NUM; i++) {
		set_bit(adc_key[i], am_key_dev->keybit);
	}
#endif	
	for (i = 0; i < gpioTotalNum; i++) {
		set_bit(gpio_key[i].bcode, am_key_dev->keybit);
	}

	for(i=0;i<irTotalNum;i++)
	{
		set_bit(i+bktotalNum+1, am_key_dev->keybit);
	}
#if 0
	printk("ir usr code==%x%x\n",cCodeH,cCodeL);
	for(j=0;j<gpioTotalNum;j++)
	{
		printk("gpio_key[%d],gpio_num=%d\n",j,gpio_key[j].gpionum);
	}
	
	for(j=0;j<ADC_CFG_NUM;j++)
	{
		printk("adc_keycode[%d]=%x\n",j,adc_key[j]);
	}

	for(j=0;j<irTotalNum;j++)
	{
		printk("ir_keycode[%d]=%x\n",j,ir_key[j]);
	}
	
#endif

	return 0;
}


static int kbd_config_init(struct input_dev *dev)
{
	int ret=0;
	struct file *fp;
	struct kstat fstat;
	mm_segment_t fs; 
	
	int bktotal;
	int key_total;
	key_map_info *key_info=NULL;
	
	memset(&kcfg,0,sizeof(key_cfg_info));
	
	fs = get_fs(); 
    	set_fs(KERNEL_DS);
	ret = vfs_stat(KBD_CONFIG_FILE, &fstat);
	if(ret){
		printk("stat config fail %d\n",ret);
		goto END;
	}

	fp = filp_open(KBD_CONFIG_FILE, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
       	printk("open %s error\n",KBD_CONFIG_FILE); 
		ret = -ENOENT;
        	goto END;
	}

	vfs_llseek(fp,EZ_OFFSET,SEEK_SET);
	ret = vfs_read(fp,(char*)&kcfg,2,&fp->f_pos);

	if(kcfg.bkeytype==0)
	{
		vfs_read(fp, (char*)&kcfg.bkeynum.tbknum, 2, &fp->f_pos);
		bktotalNum = bktotal = kcfg.bkeynum.tbknum;
	}
	else if(kcfg.bkeytype==1)
	{
		vfs_read(fp, (char*)&kcfg.bkeynum.matrixbk.ihor, 1, &fp->f_pos);
		vfs_read(fp, (char*)&kcfg.bkeynum.matrixbk.iver, 1, &fp->f_pos);
		bktotal = kcfg.bkeynum.matrixbk.ihor + kcfg.bkeynum.matrixbk.iver;		
		bktotalNum =  kcfg.bkeynum.matrixbk.ihor * kcfg.bkeynum.matrixbk.iver;	
	}
	vfs_llseek(fp,sizeof(int)*bktotal,SEEK_CUR);
	vfs_read(fp, (char*)&kcfg.irkeynum, 2, &fp->f_pos);
	vfs_read(fp, (char*)&kcfg.irusrcode, 2, &fp->f_pos);
	key_total = bktotal + kcfg.irkeynum;
	key_info = (key_map_info*)kmalloc(key_total*sizeof(key_map_info),GFP_KERNEL);

	vfs_llseek(fp,EZ_OFFSET+4,SEEK_SET);
	vfs_read(fp, (char*)key_info, bktotal*sizeof(key_map_info), &fp->f_pos);
	
	vfs_llseek(fp,2+2,SEEK_CUR);
	vfs_read(fp,(char*)&key_info[bktotal], kcfg.irkeynum*sizeof(key_map_info), &fp->f_pos);
	filp_close(fp,NULL);
	set_fs(fs);
#if 0
	int i;
	printk(KERN_INFO "bk type=%x,bk key_total==%d\n",kcfg.bkeytype,bktotalNum);
	printk(KERN_INFO "ir key_total==%d\n",kcfg.irkeynum);
	printk(KERN_INFO "ir usr code==%x\n",kcfg.irusrcode);
	printk(KERN_INFO "key_total==%d\n",key_total);
	for(i=0;i<key_total;i++)
	{
		printk(KERN_INFO "keycode[%d]==%x\n",i,key_info[i]);
	}
#endif	

	irTotalNum = kcfg.irkeynum; 
	cCodeH=(kcfg.irusrcode>>8)&0xff;
	cCodeL=(kcfg.irusrcode)&0xff;

	dev->keycodemax = key_total;

	if(am7x_key_encoder(dev,(void*)key_info)<0)
		return -1;
	kfree(key_info);
END:
	return 0;
}


static int am7x_key_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	printk("HAHA Key Type=%d, code=%d, value=%d\n",type,code,value);
	return 0;
}

static int am7x_key_probe(struct platform_device *pdev)
{
	int error;
	int temp_v;
	printk(KERN_INFO "*********am_key_init************\n");
	am_key_dev = input_allocate_device();
	if (!am_key_dev)
	{
		printk(KERN_ERR "input_allocate_device() failed for am key\n");
		return -ENOMEM;
	}

	//printk("++++++++MTL5=%x   KEY_ACTION_MASK\n",am_get_mfl(5));
	am_key_dev->name = "AM Keyboard";
	am_key_dev->phys = "AM/input0";
	am_key_dev->id.bustype = BUS_HOST;
	am_key_dev->id.vendor = 0x0001;
	am_key_dev->id.product = 0x0001;
	am_key_dev->id.version = 0x0100;

	am_key_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP)|BIT_MASK(EV_PWR);
	am_key_dev->keycode = NULL;
	am_key_dev->keyparam = NULL;
	
	am_key_dev->keycodesize = sizeof(unsigned char);
	am_key_dev->keycodemax = 0;//ARRAY_SIZE(am_key_code);
	am_key_dev->key_encode = am7x_key_encoder;
	am_key_dev->key_cfg = am7x_keytype_cfg;
	am_key_dev->event = am7x_key_event;
	//printk(KERN_INFO "am_key_dev->keybit %x\n",am_key_dev->keybit[0]);

	/* error check */
	error = input_register_device(am_key_dev);
	if (error) {
		printk(KERN_ERR "input_allocate_device() failed for am key\n");
		input_free_device(am_key_dev);
		return error;
	}
	/*init am7x key board from keyDriver.bin*/
	kbd_config_init(am_key_dev);
	//use board key
	if(kcfg.typeused&0x01)
	{
		act_writel(act_readl(CMU_DEVCLKEN)|(1<<25)|(1<<18), CMU_DEVCLKEN);
		mdelay(100);
		act_writel(act_readl(CMU_DEVRST)|(1<<29), CMU_DEVRST);
		mdelay(100);
		act_writel(act_readl(CMU_SPCLK)|(7), CMU_SPCLK);
#if	CONFIG_AM_CHIP_ID==1203	
		act_writel(act_readl(AMU_CTL)|(1<<14), AMU_CTL);
#elif	CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID==1220 || CONFIG_AM_CHIP_ID==1213
		act_writel(act_readl(AMU_CTL)|(1<<8), AMU_CTL);
#endif
		temp_v = KC_HD_T(3)|KC_NUM(6)|KC_DN_E|KC_UP_E|KC_HD_E|KC_WH_E|KC_DF_F;
		act_writel(0x36800000, KEY_CTL);
		
#if CONFIG_AM_CHIP_ID ==1211	
		init_timer(&k_timer);
		k_timer.expires = jiffies +KEY_SCAN_PERIOD;
		k_timer.function = (void*)key_scan;
		add_timer(&k_timer);
#elif CONFIG_AM_CHIP_ID ==1220 || CONFIG_AM_CHIP_ID ==1213
		// currently disabled.
		init_timer(&k_timer);
		k_timer.expires = jiffies +KEY_SCAN_PERIOD;
		k_timer.function = (void*)key_scan;
		add_timer(&k_timer);
#endif
	}

	//use ir key
	if(kcfg.typeused&0x02)
	{
		if(ir_remote_init()<0)
			printk(KERN_ERR "IR Remote IRQ request fail!\n");
	}
	platform_set_drvdata(pdev, am_key_dev);
	return 0;
}

static int  am7x_key_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "end ir device driver\n");
	del_timer(&k_timer);
	input_unregister_device(am_key_dev);
	return 0;
}

#ifdef CONFIG_PM
static int am7x_key_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct input_dev *am7x_kbd;
	am7x_kbd = platform_get_drvdata(pdev);
	/*disable ir remote interrupt*/
	save_remote_ctl = act_readl(IR_REMOTE_CTRL);
	act_writel(0, IR_REMOTE_CTRL);
	/*disable kbd scan timer*/
#if	CONFIG_AM_CHIP_ID != 1213
	mod_timer(&k_timer,jiffies+0xffffffff);
#endif
	return 0;
}
static int am7x_key_resume(struct platform_device* pdev)
{
	struct input_dev *am7x_kbd;
	am7x_kbd = platform_get_drvdata(pdev);
	/*enable ir remote interrupt*/
	act_writel(save_remote_ctl, IR_REMOTE_CTRL);
	/*enable kbd scan timer*/
#if	CONFIG_AM_CHIP_ID != 1213	
	mod_timer(&k_timer,jiffies+KEY_SCAN_PERIOD);
#endif
	return 0;
}


#else
#define am7x_key_suspend	NULL
#define am7x_key_resume	NULL
#endif
static struct platform_driver am7x_key_driver = {
	.driver		= {
		.name	= PLATFORM_KBDDEVICE_NAME,/*shoule be the same as the platform device*/
		.owner	= THIS_MODULE,
	},
	.probe		= am7x_key_probe,
	.remove		= am7x_key_remove,
	.suspend		= am7x_key_suspend,	/*require for powser consumption*/
	.resume		= am7x_key_resume,	/*require for powser consumption*/
};

static int __init am_keys_init(void)
{
	return platform_driver_register(&am7x_key_driver);
}

static void __exit am_keys_exit(void)
{
	platform_driver_unregister(&am7x_key_driver);
}

module_init(am_keys_init);
module_exit(am_keys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kewen");
MODULE_DESCRIPTION("Key driver for Actions-Micro");
MODULE_ALIAS("platform:am7x_keys");
