#include <linux/init.h>
#include<linux/module.h>
#include <linux/kernel.h>   
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <sys_msg.h>
#include <am7x_gpio.h>
#include "actions_regs.h"
#include "actions_io.h"
#include "rb_spikey.h"

#define BUFLENGTH  5
static struct input_dev *am_spikey_dev;
static struct timer_list spikey_timer;
static INT32U gGPIO_Multi5;
static INT32U gGPIO_Multi7;
static INT32U gGPIO_Multi8;
static unsigned int old_data_buff[BUFLENGTH];
static unsigned int new_data_buff[BUFLENGTH];
static unsigned int send_data_buff[BUFLENGTH];
static unsigned int send_dataup_buff[BUFLENGTH];
static const int out_data [] = {1,2,4,8,16,32,64,128};
static const int in_data [] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

static struct Readboykey {
	int old_length;
	int new_length;
	int send_length;
	int send_uplength;
}Readboy_key;
/****************
*@函数声明
*@
****************/
#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))   
/****************
*@键值对照表
*@
****************/
/*
static int key_table[8][16] = {
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
	81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
	97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128
};*/
static int key_table[8][16] = {
	64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
	80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
	96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191
};
/********************
*@SPI方式读写8bit
*@
********************/
static unsigned char spi_rxtxbyte(unsigned char data)
{
	unsigned char i, temp;
	temp = 0;
	SCLK_LOW;
	for(i=0; i<8; i++)
	{
		if (data & 0x80)
			SI_HIGH;//write bit
		else 
			SI_LOW;	
		data <<= 1;
		SCLK_HIGH; 
		temp <<= 1;
		if(SO) temp++;//read bit   
		SCLK_LOW;
	}
	return temp;	
}
/********************
*@写命令到键盘
*@
********************/
static void spi_write_reg(unsigned char addr, unsigned char value) 
{
    	CSn_LOW;
    	//while (SO); 
    	spi_rxtxbyte(addr|SPI_RW_ADDRESS_MASK);
    	spi_rxtxbyte(value);
    	CSn_HIGH;
}
/********************
*@读取键盘的数据
*@读取8bit
********************/
static unsigned char  spi_read_reg(unsigned char addr)
{
   	 unsigned char x;
    	CSn_LOW;
    	//while (SO);
    	spi_rxtxbyte(addr &(~(SPI_RW_ADDRESS_MASK)));
    	x = spi_rxtxbyte(0);
    	CSn_HIGH;
    	return x;
}
/********************
*@读取完整的键值16bit
*@
********************/
static unsigned short read_spi_port(void)
{
	unsigned short usLowByte, usHighByte;
    	usLowByte = spi_read_reg(KEYSCAN_IN_LOW_DATA) & 0xff;
      usHighByte = spi_read_reg(KEYSCAN_IN_HIGH_DATA) &0xff;
    	return usLowByte + (usHighByte << 8); 
}
/********************
*@
*@
********************/
static unsigned int  write_spi_port(unsigned char value)
{
	spi_write_reg(KEYSCAN_OUT_DIR, value);//send (0x04+0x00)|0x80   value
	return 0;
}
/********************
*@初始化SPI Key 的IO 口
*@
********************/
void init_spikeyboard_gpio(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else
	//set GPIO54 为 普通IO
	gGPIO_Multi5=act_readl(GPIO_MFCTL5);	
	act_writel((gGPIO_Multi5&(~(7<<12))),GPIO_MFCTL5); 
	//set GPIO52 为 普通IO
	gGPIO_Multi7=act_readl(GPIO_MFCTL7);	
	act_writel((gGPIO_Multi7&(~(7<<0))),GPIO_MFCTL7); 
	//set GPIO51为 普通IO
	gGPIO_Multi7=act_readl(GPIO_MFCTL7);	
	act_writel((gGPIO_Multi7&(~(7<<4))),GPIO_MFCTL7); 
	//set GPIO53为 普通IO
	gGPIO_Multi8=act_readl(GPIO_MFCTL8);	
	act_writel((gGPIO_Multi8&(~7<<8)),GPIO_MFCTL8);
#endif
}
/********************
*@初始化SPI Key通信
*@
********************/
void reset_spi_communicate(void)
{
	spi_write_reg(0xFF, 0);//send 0xff|0x80 	0x00

	spi_write_reg(KEYSCAN_OUT_ATTR,0x00);//send (0x08+0x00)|0x80        	0x00
	spi_write_reg(KEYSCAN_OUT_DIR,0x00);	//send (0x04+0x00)|0x08         	 0x00
	spi_write_reg(KEYSCAN_OUT_BUFFER,0xff );//send (0x00+0x00)|0x08  	 0xff
	
	spi_write_reg(KEYSCAN_IN_ATTR,0x00);//send (0x08+0x01)|0x80  	0x00
	spi_write_reg(KEYSCAN_IN_DIR, 0x00); //send (0x04+0x01)|0x80  	0x00
	spi_write_reg(KEYSCAN_IN_BUFFER,0xff );//send(0x00+0x01)|0x80 	0xff  

	spi_write_reg(KEYSCAN_IN1_ATTR,0x00);//send (0x80+0x02)|0x80 	0x00
	spi_write_reg(KEYSCAN_IN1_DIR, 0x00); //send(0x04+0x02)|0x80  	0x00
	spi_write_reg(KEYSCAN_IN1_BUFFER,0xff ); //send(0x00+0x02)|0x80 0xff
}
/********************
*@行列扫描方式,一次最多存储两个键
*@return pressed key count
********************/
int scan_keyboard(unsigned int ausScanCode[], int ucMaxKeys)
{
	unsigned char ucScanCol, ucScanRow, ucPressedKeyCount=0;
	unsigned short usCurKey;
	
	usCurKey = read_spi_port();
	if(0 != (usCurKey & 0xffff))
	{
		reset_spi_communicate();//reset spikey
	}
	write_spi_port(0xff);
	usCurKey = read_spi_port();
	if ( 0 != (usCurKey & 0xffff))
	{
		for(ucScanCol = 0; ucScanCol < 8; ucScanCol ++)
		{
			write_spi_port(out_data[ucScanCol]);
			usCurKey = read_spi_port();
			for(ucScanRow = 0; ucScanRow < 16; ucScanRow ++)
			{
				if (usCurKey & in_data[ucScanRow])
				{
					ausScanCode[ucPressedKeyCount++] = key_table[ucScanCol][ucScanRow];
					if(ucPressedKeyCount >= ucMaxKeys)
					{
						goto KeyEnd;
					}
				}
			}	
		}
	}
	else
	{
	}
KeyEnd:	
	write_spi_port(0x00);	
	return ucPressedKeyCount;
}
/********************
*@定时器服务函数
*@
********************/
void keyspistatus(void)
{
	int i,j;
	Readboy_key.send_length=0;
	Readboy_key.send_uplength=0;
	memset(new_data_buff,0x00,sizeof(unsigned int)*BUFLENGTH);
	memset(send_dataup_buff,0x00,sizeof(unsigned int)*BUFLENGTH);
	Readboy_key.new_length = scan_keyboard(new_data_buff, 5);
	if((Readboy_key.new_length==0)&(Readboy_key.old_length==0))
	{
		input_report_key(am_spikey_dev, 0x43, 0);//将shift 释放
		input_sync(am_spikey_dev);			
		goto nochange;
	}
	if(Readboy_key.new_length>Readboy_key.old_length)//press
	{
		if(Readboy_key.old_length!=0)
		{
			for(i=0;i<Readboy_key.new_length;i++)
			{
				for(j=0;j<Readboy_key.old_length;j++)
				{
					if(new_data_buff[i]==old_data_buff[j])
						break;	
				}
				if(j>=Readboy_key.old_length)
				{
					send_data_buff[Readboy_key.send_length]=new_data_buff[i];
					Readboy_key.send_length++;
				}
			}
		}
		else
		{
			for(i=0;i<Readboy_key.new_length;i++)
			{
				send_data_buff[Readboy_key.send_length]=new_data_buff[i];
				Readboy_key.send_length++;
			}
		}
		for(i=0;i<Readboy_key.send_length;i++)
			input_report_key(am_spikey_dev, send_data_buff[i]&0xFFFF, 1);
		input_sync(am_spikey_dev);
	}
	else if(Readboy_key.new_length<Readboy_key.old_length)//release
	{	
		if(Readboy_key.new_length!=0)
		{
			for(i=0;i<Readboy_key.old_length;i++)
			{
				for(j=0;j<Readboy_key.new_length;j++)
				{
					if(old_data_buff[i]==new_data_buff[j])
						break;	
				}
				if(j>=Readboy_key.new_length)
				{
					send_data_buff[Readboy_key.send_length]=old_data_buff[i];
					Readboy_key.send_length++;
				}
			}
		}
		else
		{
			for(i=0;i<Readboy_key.old_length;i++)
			{
				send_data_buff[Readboy_key.send_length]=old_data_buff[i];
				Readboy_key.send_length++;
			}		
		}
		for(i=0;i<Readboy_key.send_length;i++)
			input_report_key(am_spikey_dev, send_data_buff[i]&0xFFFF, 0);
		input_sync(am_spikey_dev);	
	}
	else if(Readboy_key.new_length==Readboy_key.old_length)//hold key
	{
		for(i=0;i<Readboy_key.new_length;i++)//check press
		{
			for(j=0;j<Readboy_key.old_length;j++)
			{
				if(new_data_buff[i]==old_data_buff[j])
					break;	
			}
			if(j>=Readboy_key.old_length)
			{
				send_data_buff[Readboy_key.send_length]=new_data_buff[i];
				Readboy_key.send_length++;
			}
		}
		for(i=0;i<Readboy_key.old_length;i++)//check release
		{
			for(j=0;j<Readboy_key.new_length;j++)
			{
				if(old_data_buff[i]==new_data_buff[j])
					break;	
			}
			if(j>=Readboy_key.new_length)
			{
				send_dataup_buff[Readboy_key.send_uplength]=old_data_buff[i];
				Readboy_key.send_uplength++;
			}
		}
		
		if(Readboy_key.send_length!=Readboy_key.send_uplength)
			goto errchange;
		if((Readboy_key.send_length==0)&(Readboy_key.send_uplength==0))
			goto nochange;

		for(i=0;i<Readboy_key.send_length;i++)
			input_report_key(am_spikey_dev, send_data_buff[i]&0xFFFF, 1);
errchange:	
		for(i=0;i<Readboy_key.send_uplength;i++)
			input_report_key(am_spikey_dev, send_dataup_buff[i]&0xFFFF, 0);
		input_sync(am_spikey_dev);	
	}
	memset(old_data_buff,0x00,sizeof(unsigned int)*BUFLENGTH);
	for(i=0;i<Readboy_key.new_length;i++)
	{
		old_data_buff[i]=new_data_buff[i];
	}
	Readboy_key.old_length=Readboy_key.new_length;
nochange:	
	mod_timer(&spikey_timer,jiffies +8);
}
/********************
*@初始化定时器
*@扫描是否有键按下或释放
********************/
static int init_spikey_timer(void)
{
	init_timer(&spikey_timer);			
	spikey_timer.function = (void*)keyspistatus;
	spikey_timer.expires = jiffies +5*HZ;
	add_timer(&spikey_timer);	
	return 0;
}
/********************
*@系统注册
*@初始化工作
********************/
static int __init spikey_init(void)
{
	int error;
	int i;
	am_spikey_dev = input_allocate_device();
	if (!am_spikey_dev)
	{
		printk(KERN_ERR "input_allocate_device() failed for am key\n");
		return -ENOMEM;
	}
	am_spikey_dev->name = "SPI Keyboard";
	am_spikey_dev->phys = "SPI/input0";
	am_spikey_dev->id.bustype = BUS_HOST;
	am_spikey_dev->id.vendor = 0x0001;
	am_spikey_dev->id.product = 0x0001;
	am_spikey_dev->id.version = 0x0100;

	am_spikey_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	am_spikey_dev->keycode = NULL;
	am_spikey_dev->keyparam = NULL;
	
	am_spikey_dev->keycodesize = sizeof(unsigned char);
	am_spikey_dev->keycodemax = 0;//ARRAY_SIZE(am_key_code);
	am_spikey_dev->key_encode = NULL;//am7x_key_encoder;
	am_spikey_dev->key_cfg = NULL;//am7x_keytype_cfg;

	for(i=0;i<256;i++)
	{
		set_bit(i, am_spikey_dev->keybit);
	}
	
	error = input_register_device(am_spikey_dev);
	if (error) {
		printk(KERN_ERR "input_allocate_device() failed for am key\n");
		input_free_device(am_spikey_dev);
		return error;
	}
	init_spikeyboard_gpio();	
	reset_spi_communicate();
	Readboy_key.new_length=0;
	Readboy_key.old_length=0;
	init_spikey_timer();
	printk(KERN_INFO">>start to init spi key<<\n");
	return 0;
}
/********************
*@注销模块
*@
********************/
static void __exit spikey_exit(void)
{
	del_timer(&spikey_timer);
	input_unregister_device(am_spikey_dev);
}

module_init(spikey_init);
module_exit(spikey_exit);
MODULE_AUTHOR("luozhenghai");
MODULE_DESCRIPTION("SPI keyboard driver");
MODULE_LICENSE("GPL");
