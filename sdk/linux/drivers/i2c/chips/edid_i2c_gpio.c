/*
 *  EDID_GPIO
 ****************************
 * Actions-micro PMU IC
 *
 * author: liangwenhua
 * date: 2013-05-28
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

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif

#define I2C_ADRRESS  			0xa0
#define EDID_LENGTH			0x80
#define EDID_I2C_SEGMENT_ADDR		0x30
#define I2C_ACK      0
#define I2C_NACK    1
//INT32S readbuf=0xff;

/**************************
*@º¯ÊýÉùÃ÷
*@
**************************/
INT8U am_get_gpio(loff_t num);
void am_set_gpio(loff_t num,INT8U value);
#if (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)		//EZWire is use gpio i2c to read edid
char GPIO26 = -1;
#define IS_825XW			(((GPIO26 < 0) && ((GPIO26=am_get_gpio(26)) == 1)) || (GPIO26 == 1))
#define MFC_DAT_W			((act_readl(SCLK_MFC)&(~(0x7<<13)))|(0x4<<13))

#define SCLK_GPIO			(IS_825XW?35:40)
#define SCLK_MFC			GPIO_MFCTL1
#define SCLK_MFC_DAT_OLD	((act_readl(SCLK_MFC)&(~(0x3<<18)))|(0x2<<18))
#define SCLK_MFC_DAT		(IS_825XW?MFC_DAT_W:SCLK_MFC_DAT_OLD)

#define SDA_GPIO			(IS_825XW?34:39)
#define SDA_MFC				GPIO_MFCTL1
#define SDA_MFC_DAT_OLD		((act_readl(SDA_MFC)&(~(0x3<<16)))|(0x2<<16))
#define SDA_MFC_DAT			(IS_825XW?MFC_DAT_W:SDA_MFC_DAT_OLD)
#else
#define SCLK_GPIO	52
#define SDA_GPIO	53
#define SCLK_MFC	GPIO_MFCTL3
#define SDA_MFC		GPIO_MFCTL3
#define SCLK_MFC_DAT	((act_readl(SCLK_MFC)&0xffffcfff|0x00004000))
#define SDA_MFC_DAT		((act_readl(SDA_MFC)&0xfffe7fff|0x00020000))
#endif

#define M_SCLK_HIGH()		am_set_gpio(SCLK_GPIO,1)
#define M_SCLK_LOW()		am_set_gpio(SCLK_GPIO,0)
#define M_SDA_HIGH()		am_set_gpio(SDA_GPIO,1)
#define M_SDA_LOW()		am_set_gpio(SDA_GPIO,0)
#define M_SDAIN()			am_get_gpio(SDA_GPIO)  
#define M_SCLIN()			am_get_gpio(SCLK_GPIO) 
#define IICCLK_MULTFUNC_EN()		act_writel(SCLK_MFC_DAT,SCLK_MFC)
#define IICDATA_MULTFUNC_EN() 	act_writel(SDA_MFC_DAT,SDA_MFC)

#define M_SCLKH()			M_SCLK_HIGH()
#define M_SCLKL()			M_SCLK_LOW()
#define M_SDAH()			M_SDA_HIGH()
#define M_SDAL()			M_SDA_LOW()
unsigned char *databuf;
static int delay(void) 
{
	udelay(10);
	return 0;
}
static int M_I2CStart(void) 
{
	IICDATA_MULTFUNC_EN();
	M_SDAH();
	delay();
	IICCLK_MULTFUNC_EN();
      	M_SCLKH();
	delay();
	IICDATA_MULTFUNC_EN();
	M_SDAL();
	delay();
	IICCLK_MULTFUNC_EN();
	M_SCLKL();
	delay();
	return 0;
}
static int M_I2CStop(void) 
{
	IICDATA_MULTFUNC_EN();
	M_SDAL();
	delay();
	IICCLK_MULTFUNC_EN();
	M_SCLKH();
	delay();
	IICDATA_MULTFUNC_EN();
	M_SDAH();
	delay();
	return 0;
}
static int M_I2CReStart(void) 
{
	M_I2CStop();
		delay();
	M_I2CStart();
	return 0;
}
static INT32S  getSendAck(INT32S  iAckType)
{
	INT32S  iAck;
	if(iAckType==1)//master check ACK from slave
	{
		IICDATA_MULTFUNC_EN();
		M_SDAH();
		delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKH();
		//M_SDA_INEN();
		delay();
		IICDATA_MULTFUNC_EN();
		iAck = M_SDAIN();
		delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKL();	
		delay();
		//M_SDA_OUTEN();
	//delay();
	} 
	else if(iAckType==0)//master send ACK when read data
	{
		IICDATA_MULTFUNC_EN();
		M_SDAL();
	delay();
	IICCLK_MULTFUNC_EN();
		M_SCLKH();
	delay();
		iAck = 0;
		IICCLK_MULTFUNC_EN();
		M_SCLKL();
	delay();
	}
	else if(iAckType==2) //master send NACK when read data
	{
		IICDATA_MULTFUNC_EN();
		M_SDAH();
	delay();
	IICCLK_MULTFUNC_EN();
		M_SCLKH();
	delay();
		iAck = 1;
		IICCLK_MULTFUNC_EN();
		M_SCLKL();
	delay();
	}	
	return iAck;
}
static int M_I2CWrite8Bit(unsigned char iData) 
{
    	INT32S  i=0;
    for (i=0;i<8;i++)
	{
		IICDATA_MULTFUNC_EN();
		if (iData &0x80)
		      M_SDAH();
		else
			M_SDAL();
		//printk("")M_SDAIN()
	delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKH();       //HIGH
	delay();
	IICCLK_MULTFUNC_EN();
		M_SCLKL();    //LOW
	delay();
		iData = iData << 1;
	delay();
	}
	return getSendAck(1);
}
static int  M_I2CRead8Bit(void) 
{
    	INT32S  i=0;
    	INT32S  iRetVal=0;
	IICDATA_MULTFUNC_EN();	
	M_SDAH();	
	//printk("\n");
	for (i=0; i<8; i++)
	{
		iRetVal = iRetVal << 1;
		IICCLK_MULTFUNC_EN();
		M_SCLKL();    //LOW
	 	delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKH();   //HIGH
		delay();
		iRetVal |=M_SDAIN();
		//printk("%d", M_SDAIN());
		delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKL();    //LOW
		delay();
	}
	//printk("\niRetVal: 0x%x\n", iRetVal);
    return iRetVal;
}
static int I2CWrite_SW(unsigned long  devAddr, unsigned long  reg, unsigned long  numWrite,unsigned char  *pBuf)
{
	INT32S  i, status;
	INT32S  retVal = -1;
	M_I2CStart();
	status = M_I2CWrite8Bit(devAddr);
	//printk("status1=================%d\n",status);
	if (status == I2C_ACK)
	{
        	status = M_I2CWrite8Bit(reg);
		//printk("status1=================%d\n",status);
		if (status == I2C_NACK)
		{
			M_I2CStop();
			return -1;
		}
        	for (i=0; i<numWrite; i++)
		{
			status = M_I2CWrite8Bit(pBuf[i]);
			if (status == I2C_NACK)
			{
				M_I2CStop();
				return -1;
			}
		}
		retVal = 0;
	}
	M_I2CStop();
	return retVal;
}
 unsigned int I2CRead_SW(unsigned long  devAddr, unsigned long  reg,unsigned long  numRead,unsigned char * pBuf)
{
	//printk("start I2CRead_SW!!!!!!!!!!!!!!\n");
	INT32S  i=0,status;
	unsigned char *databuf;//= kmalloc(numRead, GFP_KERNEL);
	databuf=pBuf;
	memset(databuf,0,numRead);
	M_I2CStart();
	status = M_I2CWrite8Bit(devAddr);
	//printk("read status1============%d\n",status);
	if (status == I2C_ACK)
	{
		status = M_I2CWrite8Bit(reg);
		//printk("read status2============%d\n",status);
		if (status == I2C_NACK)
		{
			M_I2CStop();
			return -1;
		}
		M_I2CStart();
		status = M_I2CWrite8Bit(devAddr |0x00000001);
		//printk("read status3============%d\n",status);
		if (status == I2C_NACK)
		{
			M_I2CStop();
			return -1;
		}
		for(i;i<numRead-1;i++)
		{
			*databuf= M_I2CRead8Bit();
			getSendAck(0);
			//printk("*databuf[%d]==============0x%x\n",i,*databuf);
			databuf++;
		}
		*databuf= M_I2CRead8Bit();
		getSendAck(2);
		
		M_I2CStop();		
		return 0; 
		//printk("*databuf[%d]==============0x%x\n",i,*databuf); 
	}
	M_I2CStop();	
	return -1; 

}
static int GPIO_I2C_write(int addr,int data)
{
	 /*unsigned char  Read_Buffer[EDID_LENGTH]={0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,
		  0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf,0xaf};*/
	unsigned char  Read_Buffer[EDID_LENGTH];	  
	unsigned char *pRead_Buffer;
	unsigned char i=0;
	pRead_Buffer=Read_Buffer;
	for(i=0;i<EDID_LENGTH;i++)
	{
		printk("Read_Buffer[%d]=======================0x%x\n",i,Read_Buffer[i]);
	}   	
	I2CWrite_SW(I2C_ADRRESS,addr, EDID_LENGTH,Read_Buffer);
}

unsigned char * GPIO_I2C_read(int addr)
{
	//printk("start rGPIO_I2C_read!!!!!!!!!!!!!!\n");
	unsigned char i=0,ret;
	unsigned char extension_flag = 0;
	unsigned char * databuf=NULL;
	unsigned char *databuf_temp = NULL;
	databuf =  kmalloc(EDID_LENGTH, GFP_KERNEL);
	if (!databuf)
	{
		printk("unable to allocate memory for EDID block0.\n");
		return NULL;
	}
	ret=I2CRead_SW(I2C_ADRRESS,addr,EDID_LENGTH,databuf);
	if (ret)
	{
		printk("unable to read EDID block\n");
		kfree(databuf);
		return NULL;
	}
	extension_flag = databuf[126];
	if (extension_flag>=1)
	{
		printk("extension_flag=%d\n",extension_flag);
		unsigned char *buf_extension = kmalloc((EDID_LENGTH*(extension_flag+1)), GFP_KERNEL);
		if (!buf_extension)
		{
			printk("unable to allocate memory for EDID extension \n");
			return databuf;
		}
		if (extension_flag ==1)
		{
			ret=I2CRead_SW(I2C_ADRRESS,addr,EDID_LENGTH*2,buf_extension);
			if (!ret)
			{
				printk("read edid block1 ok\n");
				kfree(databuf);
				return buf_extension;					
			}
			else
			{
				printk("read edid block1 error\n");
				kfree(buf_extension);					
				return databuf;
			}			
		}
		else
		{
			for (i=0;i<=extension_flag/2;i++)
			{					
				ret = I2CWrite_SW(EDID_I2C_SEGMENT_ADDR, 0,1, &i);
				if (!ret)
				{
					ret=I2CRead_SW(I2C_ADRRESS,addr,EDID_LENGTH*2,buf_extension);
					if (!ret)
					{
						printk("read edid block1 ok\n");
						kfree(databuf);
						return buf_extension;					
					}
					else
					{
						printk("read edid block1 error\n");
						kfree(buf_extension);					
						return databuf;
					}			
					}
					else
					{
						printk("read edid segment error\n");
						kfree(buf_extension);					
						return databuf;
					}
			}
			kfree(databuf);
			return buf_extension;
		}
	}
	return databuf;		
}


unsigned char * edid_i2c_gpio_read()
{
	unsigned char *edid = NULL;
	int i=0;
	//printk("start read edid!!!!!!!!!!!!!!\n");
	edid = GPIO_I2C_read(0);
	/*for(i;i<EDID_LENGTH;i++)
	{
		printk("edid[%d]===================0x%x\n",i,*edid);
	}*/
	return edid;	
}
EXPORT_SYMBOL(edid_i2c_gpio_read);

static int __init edid_i2c_gpio_init(void)
{
	IICDATA_MULTFUNC_EN();
	IICCLK_MULTFUNC_EN();
	printk("act_readl(GPIO_MFCTL3)=============0x%x\n",act_readl(GPIO_MFCTL3));
	return 0;
}

static void __exit edid_i2c_gpio_exit(void)
{
	return 0;
}

MODULE_AUTHOR("LiangWenhua <@actions-micro.com>");
MODULE_DESCRIPTION("edid_i2c_gpio driver");
MODULE_LICENSE("GPL");

module_init(edid_i2c_gpio_init);
module_exit(edid_i2c_gpio_exit);
