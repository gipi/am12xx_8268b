///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <it6681.c>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2013/12/06
//   @fileversion: ITE_IT6682_1.01
//******************************************/
/*
 * MHL support
 *
 * Copyright (C) 2013 ITE Tech. Inc.
 * Author: Hermes Wu <hermes.wu@ite.com.tw>
 *
 * MHL TX driver for IT6681
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/time.h>
#include "it6681_cfg.h"
#include "it6681_arch.h"
#include "it6681_debug.h"
#include "it6681_def.h"
#include "it6681_io.h"
#include <linux/gpio.h>
#include <linux/it6681.h>
#include "it6681_drv.h"
#include <actions_regs.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <am7x_i2c.h>

static int dependence = 0;
module_param(dependence,int,S_IRUGO);
static ushort mhl_adopter_id = 0x245;
module_param(mhl_adopter_id,ushort,S_IRUGO);
static ushort mhl_device_id = 0x6681;
module_param(mhl_device_id,ushort,S_IRUGO);

unsigned char mhl_adopter_id_h,mhl_adopter_id_l;
unsigned char mhl_device_id_h,mhl_device_id_l;

#define GPIO_I2C_FOR_CTL	0
#define GPIO_USB_MHL_SWITCH 134
#define GPIO_ENVBUS 135
static struct device *it6681dev;


static int init_it6681_loop_kthread(void);
static int it6811_irq_handler_kthread(void *data);
static irqreturn_t it6811_irq_thread(int irq, void *data);

struct it6681_data
{
    struct it6681_platform_data	*pdata;
    struct it6681_dev_data *ddata;
    int irq;
    int dev_inited;
    atomic_t int_active;
    struct mutex lock;	
    wait_queue_head_t it6681_wq;
    wait_queue_head_t it6681_irq_wq;
    struct task_struct *it6681_timer_task;
    struct task_struct *it6681_irq_task;
    atomic_t it6681_timer_event;

#if _SUPPORT_RCP_
    struct input_dev *rcp_input;
#endif

#if _SUPPORT_UCP_
    struct input_dev *ucp_input;
#endif

#if _SUPPORT_UCP_MOUSE_
    struct input_dev *ucp_mouse_input;
#endif
};

struct mutex I2C_lock;

#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))                                                           
static void RegBitSet(int val,int reg,int msb,int lsb)                                            
{                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
} 

unsigned char am_get_gpio(loff_t num);
void am_set_gpio(loff_t num,unsigned char value);
#define M_SCLK_HIGH()		am_set_gpio(52,1)
#define M_SCLK_LOW()		am_set_gpio(52,0)
#define M_SDA_HIGH()		am_set_gpio(53,1)
#define M_SDA_LOW()		am_set_gpio(53,0)
#define M_SDAIN()			am_get_gpio(53)  
#define M_SCLIN()			am_get_gpio(52) 
#define IICCLK_MULTFUNC_EN()		act_writel((act_readl(GPIO_MFCTL3)&0xffffcfff|0x00004000),GPIO_MFCTL3)
#define IICDATA_MULTFUNC_EN() 	act_writel((act_readl(GPIO_MFCTL3)&0xfffe7fff|0x00020000),GPIO_MFCTL3)

#define M_SCLKH()			M_SCLK_HIGH()
#define M_SCLKL()			M_SCLK_LOW()
#define M_SDAH()			M_SDA_HIGH()
#define M_SDAL()			M_SDA_LOW()
#define EDID_LENGTH			0x80
#define I2C_ACK      0
#define I2C_NACK    1
unsigned char *databuf;
static int delay(void) 
{
	udelay(1);
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
static int  getSendAck(int  iAckType)
{
	int  iAck;
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
    	int  i=0;
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
    	int  i=0;
    	int  iRetVal=0;
	IICDATA_MULTFUNC_EN();	
	M_SDAH();	
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
		
		delay();
		IICCLK_MULTFUNC_EN();
		M_SCLKL();    //LOW
		delay();
	}
    return iRetVal;
}
static int I2CWrite_SW(unsigned long  devAddr, unsigned long  reg, unsigned long  numWrite,unsigned char  *pBuf)
{
    	int  i, status;
    	int  retVal = 0;
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
			return 1;
		}
        	for (i=0; i<numWrite; i++)
		{
			status = M_I2CWrite8Bit(pBuf[i]);
			if (status == I2C_NACK)
			{
				M_I2CStop();
				return 1;
			}
		}
		retVal = 0;
	}
	M_I2CStop();
	return retVal;
}
 unsigned char * I2CRead_SW(unsigned long  devAddr, unsigned long  reg,unsigned long  numRead,unsigned char * pBuf)
{
	//printk("start I2CRead_SW!!!!!!!!!!!!!!\n");
	int  i=0,status;
	//unsigned char *databuf = kmalloc(EDID_LENGTH, GFP_KERNEL);
	//pBuf=databuf;
	//memset(databuf,0,EDID_LENGTH);
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
			//printk("%s %d\n",__FILE__,__LINE__);
			return 1;
		}
		M_I2CStart();
		status = M_I2CWrite8Bit(devAddr |0x00000001);
		//printk("read status3============%d\n",status);
		if (status == I2C_NACK)
		{
			M_I2CStop();
			//printk("%s %d\n",__FILE__,__LINE__);
			return -1;
		}
		for(i;i<numRead-1;i++)
		{
			*pBuf= M_I2CRead8Bit();
			//printk("*buf[%d]==============0x%x\n",i,*pBuf);
			getSendAck(0);
			pBuf++;			
		}
		*pBuf= M_I2CRead8Bit();
		//printk("*buf[%d]==============0x%x\n",(numRead-1),*pBuf);
		getSendAck(2);
		
		M_I2CStop();	
		//printk("*databuf[%d]==============0x%x\n",i,*databuf); 
	}
	M_I2CStop();	
	return pBuf; 

}
void delay1ms(unsigned short ms)
{
    msleep(ms);
}

static int i2c_write_reg(struct i2c_client *client, unsigned int offset, u8 value)
{
    int ret;
#if GPIO_I2C_FOR_CTL	
	mutex_lock(&I2C_lock);
    ret = I2CWrite_SW((client->addr <<1),offset,1,&value);
	mutex_unlock(&I2C_lock);
#else
    ret = i2c_smbus_write_byte_data(client, offset, value);
	if (ret < 0)
	{
        pr_err("IT6681 -- %s error %d, offset=0x%02x, val=0x%02x\n", __FUNCTION__, ret, offset, value);
    }
#endif
    return ret;
}

static int i2c_read_reg(struct i2c_client *client, unsigned int offset, u8 *value)
{
	int ret;

	if (!value)
		return -EINVAL;
#if GPIO_I2C_FOR_CTL	
	//pr_err("IT6681 -- %s client->addr 0x%x, offset=0x%02x\n", __FUNCTION__, client->addr, offset);
	mutex_lock(&I2C_lock);
    I2CRead_SW((client->addr <<1),offset,1,value);
	mutex_unlock(&I2C_lock);
#else
	ret = i2c_smbus_write_byte(client, offset);
	if (ret < 0)
	{
        pr_err("IT6681 -- %s write error %d, offset=0x%02x\n", __FUNCTION__, ret, offset);
		return ret;
    }

	ret = i2c_smbus_read_byte(client);
	if (ret < 0)
	{
        pr_err("IT6681 -- %s read error %d, offset=0x%02x\n", __FUNCTION__, ret, offset);
		return ret;
    }

	*value = ret & 0x000000FF;
#endif	
	//pr_err("IT6681 -- %s client->addr 0x%x, offset=0x%02x value=0x%x\n", __FUNCTION__, client->addr, offset, *value);

	return 0;
}

void hdmirxwr( unsigned char offset, unsigned char value )
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    i2c_write_reg(it6681data->pdata->hdmi_rx_client ,(unsigned int)offset,value);

    return ;
}

unsigned char hdmirxrd( unsigned char offset )
{
    unsigned char value=0x00;
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    i2c_read_reg(it6681data->pdata->hdmi_rx_client,(unsigned int)offset,&value);
	//pr_err("IT6681 -- %s value=0x%x\n", __FUNCTION__, value);
    return value;
}

void hdmitxbrd( unsigned char offset, void *buffer, unsigned char length )
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);
        int ret =0;
#if GPIO_I2C_FOR_CTL	
		I2CRead_SW((it6681data->pdata->hdmi_tx_client->addr <<1),offset,length,buffer);
#else
        ret = i2c_smbus_read_i2c_block_data(it6681data->pdata->hdmi_tx_client, offset, length, (u8*)buffer);
#endif
        if (ret < 0)
        {
        pr_err("IT6681 -- %s read error %d, offset=0x%02x\n", __FUNCTION__, ret, offset);     
    }           
}

void hdmitxwr( unsigned char offset, unsigned char value )
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    i2c_write_reg(it6681data->pdata->hdmi_tx_client ,(unsigned int)offset,value);

    return ;
}

unsigned char hdmitxrd( unsigned char offset )
{
    unsigned char value=0x00;
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    i2c_read_reg(it6681data->pdata->hdmi_tx_client,(unsigned int)offset,&value);

    return value;
}

void mhltxwr( unsigned char offset, unsigned char value )
{
	struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

	i2c_write_reg(it6681data->pdata->mhl_client ,(unsigned int)offset,value);
}

unsigned char mhltxrd( unsigned char offset )
{
    unsigned char value;
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    i2c_read_reg(it6681data->pdata->mhl_client,(unsigned int)offset,&value);

    return value;
}

void hdmirxset( unsigned char offset, unsigned char mask, unsigned char wdata )
{
    unsigned char temp;

    temp = hdmirxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&wdata);
    hdmirxwr(offset, temp);

    return ;
}

void hdmitxset( unsigned char offset, unsigned char mask, unsigned char wdata )
{
    unsigned char temp;

    temp = hdmitxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&wdata);
    hdmitxwr(offset, temp);

    return ;
}

void mhltxset( unsigned char offset, unsigned char mask, unsigned char wdata )
{
    unsigned char temp;

    temp = mhltxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&wdata);
    mhltxwr(offset, temp);

    return;
}

void set_operation_mode( unsigned char mode )
{
    if ( mode == MODE_USB )
    {
        // switch to USB mode
        //gpio_set_value( GPIO_USB_MHL_SWITCH, 1 );
    }
    else
    {
        // switch to MHL mode
        //gpio_set_value( GPIO_USB_MHL_SWITCH, 0 );
    }
}

void set_vbus_output( unsigned char enable )
{
    if ( enable == 0 )
    {
        // disable vbus output
       // gpio_set_value( GPIO_ENVBUS, 1 );
    }
    else
    {
        // enable vbus output
        //gpio_set_value( GPIO_ENVBUS, 0 );
    }
}

unsigned long it6681_get_tick_count(void) //unit:msec, system tick
{
    struct timespec tt;

    tt = CURRENT_TIME;

    return ((tt.tv_sec*1000) + (tt.tv_nsec/1000000L));
}

void SetLED_MHL_Out( char Val )
{
    //LED_MHL_Out = !Val;
}
void SetLED_PathEn( char Val )
{
    //LED_MHL_PathEn = !Val;
}

void SetLED_MHL_CBusEn( char Val )
{
    //LED_MHL_CbusEn = !Val;
}

void SetLED_HDMI_InStable( char Val )
{
    //LED_HDMI_In_Stable = !Val;
}
EXPORT_SYMBOL(it6681_read_edid);

int it6681_read_edid( void *it6681_dev_data, void *pedid, unsigned short max_length)
{
    pr_err("%s ++, pedid=%p, len=%u\n", __FUNCTION__, pedid, max_length);

    if ( pedid )
    {
        if ( max_length > 512 )
        {
            max_length = 512;
        }

        memcpy( pedid, it6681_edid_buf, max_length );

        return 0;
    }

    return -1;
}

#if _SUPPORT_RCP_

void rcp_report_event( unsigned char key)
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

	MHL_MSC_DEBUG_PRINTF(("rcp_report_event key: %d\n", key));
	input_report_key(it6681data->rcp_input, (unsigned int)key+1, 1);
	input_report_key(it6681data->rcp_input, (unsigned int)key+1, 0);
	input_sync(it6681data->rcp_input);

}
void mhl_RCP_handler(struct it6681_dev_data *it6681)
{
    rcp_report_event(it6681->rxmsgdata[1]);
}
#endif

#if _SUPPORT_UCP_

void ucp_report_event( unsigned char key)
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

	MHL_MSC_DEBUG_PRINTF(("ucp_report_event key: %d\n", key));
	input_report_key(it6681data->ucp_input, (unsigned int)key+1, 1);
	input_report_key(it6681data->ucp_input, (unsigned int)key+1, 0);
	input_sync(it6681data->ucp_input);

}

void mhl_UCP_handler(struct it6681_dev_data *it6681)
{
    ucp_report_event(it6681->rxmsgdata[1]);
}
#endif

#if _SUPPORT_UCP_MOUSE_
void ucp_mouse_report_event( unsigned char key,int x,int y)
{
	struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

    /* Report relative coordinates */
    input_report_rel(it6681data->ucp_mouse_input, REL_X, x);
    input_report_rel(it6681data->ucp_mouse_input, REL_Y, y);

    /* Report key event */
    input_report_key(it6681data->ucp_mouse_input, BTN_LEFT,   ( key&0x01));
    input_report_key(it6681data->ucp_mouse_input, BTN_MIDDLE, ((key>>1)&0x01));
    input_report_key(it6681data->ucp_mouse_input, BTN_RIGHT,  ((key>>2)&0x01));

    input_sync(it6681data->ucp_mouse_input);
}

void mhl_UCP_mouse_handler( unsigned char key, int x, int y)
{
    ucp_mouse_report_event(key, x, y);
}
#endif

/*
static int it6681_timer_kthread(void *data)
{
	struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

	atomic_set(&it6681data->it6681_timer_event ,1);

	for(;;){

		wait_event_interruptible_timeout(it6681data->it6681_wq,0,100*HZ/1000);//100ms
        if(atomic_read(&it6681data->it6681_timer_event) == 1){

            hdmitx_irq(it6681data->ddata);
			#if _SUPPORT_HDCP_
            Hdmi_HDCP_handler(it6681data->ddata);
			#endif
			//pr_err("it6681_timer_kthread() -->hdmitx_irq();  \n");
		}
		if(kthread_should_stop())
			break;
	}

	return 0;
}
*/

static int it6681_loop_kthread(void *data)
{
    struct it6681_data *it6681data = dev_get_drvdata(it6681dev);
    int doRegDump=0;
    int loopcount=0 ;
    int pp_mode;
    int hdcp_mode;
    int ret;

    // HDCP mode : 0 = disable, 1 = enable
    hdcp_mode = 0;
    // packed pixel mode : 0 = auto, 1 = force
	pp_mode = 0;

	atomic_set(&it6681data->it6681_timer_event ,1);

    it6681_fwinit();

    it6681_set_hdcp( hdcp_mode );
	if ( pp_mode == 1 )
	{
		pr_err("Force packed pixel mode\n");
		it6681_set_packed_pixel_mode( 2 );
	}
	else
	{
	    pr_err("auto packed pixel mode\n");
	    it6681_set_packed_pixel_mode( 1 );
    }

    it6681data->dev_inited = 1;

#ifdef CONFIG_AM_8251
	//EXTINT2
	RegBitSet(1,GPIO_MFCTL0,30,30);
	//RegBitSet(3,INTC_EXTCTL01,10,9);//falling edge-triggered
	RegBitSet(1,INTC_EXTCTL23,10,9);//low level active
	RegBitSet(1,INTC_EXTCTL23,8,8);
#else
	//EXTINT0
	RegBitSet(1,GPIO_MFCTL4,20,19);
	//RegBitSet(3,INTC_EXTCTL01,10,9);//falling edge-triggered
	RegBitSet(1,INTC_EXTCTL01,10,9);//low level active
	RegBitSet(1,INTC_EXTCTL01,8,8);
#endif	
	enable_irq(it6681data->irq);
    while(1)
    {
        wait_event_interruptible_timeout(it6681data->it6681_wq,0,100*HZ/1000); //100ms

        if(kthread_should_stop()) break;

        //it6681_irq();
        it6681_poll();

        if ( doRegDump )
        {
            loopcount ++ ;
            if( (loopcount % 1000) == 1 )
            {
                DumpHDMITXReg() ;
            }
            if( loopcount >= 1000 )
            {
                loopcount = 0 ;
            }
        }
    }

	return 0;
}

static int it6811_irq_handler_kthread(void *data)
{
	struct it6681_data *it6681data = data;
	
	atomic_set(&it6681data->int_active ,0);
	disable_irq(it6681data->irq);

    while(data)
    {
        RegBitSet(1,INTC_EXTCTL01,0,0);
        enable_irq(it6681data->irq);
		//pr_err("IT6681 -- %s wait irq enable\n", __FUNCTION__);
        wait_event_interruptible(it6681data->it6681_irq_wq, atomic_read(&it6681data->int_active));

       // pr_err("IT6681 -- %s resume\n", __FUNCTION__);
        if(kthread_should_stop())
        {
			break;
        }

    	if ( data && it6681data->dev_inited )
    	{
        	mutex_lock(&it6681data->lock);
        	it6681_irq();
        	//pr_err("IT6681 -- %s call irq\n", __FUNCTION__);
        	mutex_unlock(&it6681data->lock);
    	}
    	else
	    {
	        //pr_err("IT6681 -- %s data error\n", __FUNCTION__);
	    }
	    atomic_set(&it6681data->int_active ,0);
		
	//pr_err("IT6681 -- %s finish irq handle\n", __FUNCTION__);
    }	
   // pr_err("IT6681 -- %s --\n", __FUNCTION__);

	return 0;
}

static irqreturn_t it6811_irq_thread(int irq, void *data)
{
    struct it6681_data *it6681data = data;
       // pr_err("IT6681 -- %s ++\n", __FUNCTION__);		
 	
    
    if ( data && it6681data->dev_inited && (0==atomic_read(&it6681data->int_active)) ) 
    {
        disable_irq_nosync(it6681data->irq);
        atomic_set(&it6681data->int_active ,1);
        wake_up(&it6681data->it6681_irq_wq);   
        //pr_err("IT6681 -- %s wakeup\n", __FUNCTION__);
        /*mutex_lock(&it6681data->lock);
        	it6681_irq();
        	//pr_err("IT6681 -- %s call irq\n", __FUNCTION__);
        	mutex_unlock(&it6681data->lock);*/
    }
    else{
       //pr_err("IT6681 -- %s data %p %d %d\n", __FUNCTION__, data,it6681data->dev_inited, atomic_read(&it6681data->int_active)  );
       //enable_irq(it6681data->irq);
    }

    return IRQ_HANDLED;

}

static int init_it6681_loop_kthread(void)
{
	struct it6681_data *it6681data = dev_get_drvdata(it6681dev);

	pr_err("IT6681 -- %s ++\n", __FUNCTION__);

	//init_waitqueue_head(&it6681data->it6681_wq);
	it6681data->it6681_timer_task = kthread_create(it6681_loop_kthread,NULL,"it6681_loop_kthread");
    it6681data->it6681_irq_task = kthread_create(it6811_irq_handler_kthread, it6681data, "it6811_irq_handler_kthread");
    wake_up_process(it6681data->it6681_timer_task);
    wake_up_process(it6681data->it6681_irq_task);


	return 0;
}

static int __devinit it6681_hdmi_rx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
	struct it6681_data *ddata;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

#if _SUPPORT_RCP_
	struct input_dev *rcpinput;
#endif

#if _SUPPORT_UCP_
	struct input_dev *ucpinput;
#endif

#ifdef _SUPPORT_UCP_MOUSE_
	struct input_dev *mouse_ucpinput;
#endif

    pr_err("IT6681 -- %s ++\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
		pr_err(" i2c_check_functionality()    FAIL!!!) \n");
		return -EIO;
	}

	ddata = kzalloc(sizeof(struct it6681_data), GFP_KERNEL);

	if (!ddata)
    {
		pr_err("IT6681 -- failed to allocate driver data\n");
		return -ENOMEM;
	}

    ddata->ddata = get_it6681_dev_data();
    ddata->pdata = client->dev.platform_data;

    ddata->pdata->hdmi_rx_client = client;
    atomic_set(&ddata->it6681_timer_event ,0);
	ddata->it6681_timer_task = NULL;
	ddata->it6681_irq_task = NULL;
    ddata->dev_inited = 0;
	mutex_init(&ddata->lock);
	mutex_init(&I2C_lock);

    i2c_set_clientdata(client, ddata);

	it6681dev = &client->dev;
	ddata->irq = client->irq;
	init_waitqueue_head(&ddata->it6681_wq);
	init_waitqueue_head(&ddata->it6681_irq_wq);

	#if 0
    // init GPIO begin
    // This is only necessary for IT6682 with external USB switch
    ret = gpio_request( GPIO_USB_MHL_SWITCH, "USB_MHL_SWITCH");

    if (ret<0)
        pr_err("IT6681 -- %s: gpio_request failed for gpio %d\n", __func__, GPIO_USB_MHL_SWITCH);
    else
        gpio_direction_output( GPIO_USB_MHL_SWITCH, 1 );

    ret = gpio_request( GPIO_ENVBUS, "EnVBUS");

    if (ret<0)
        pr_err("IT6681 -- %s: gpio_request failed for gpio %d\n", __func__, GPIO_ENVBUS);
    else
        gpio_direction_output(GPIO_ENVBUS, 0);
    // init GPIO end
	#endif
    // init interrupt pin
    #if 1

	/*ret = request_threaded_irq(client->irq, NULL, it6811_irq_thread,
                               IRQF_TRIGGER_LOW | IRQF_ONESHOT|IRQF_DISABLED,
                               "it6681", ddata);//client-irq must assign to 
                               // gpio, kent*/
      // request_irq(de_irq_no, de_isr,IRQF_DISABLED, "de",NULL);
       //pr_err("@@@@@@@@@@@@@@@@@@@client->irq=%d\n",client->irq);
       ret = request_irq(client->irq, it6811_irq_thread,
                              //IRQF_TRIGGER_LOW | IRQF_DISABLED,
                              IRQF_DISABLED,
                               "it6681", ddata);//client-irq must assign to 
                               // gpio, kent*/
      // pr_err("@@@@@@@@@@@@@@@@@@@ret=0x%x\n",ret);
	if (ret < 0)
		goto err_exit;

	disable_irq(client->irq);
    #endif

#if _SUPPORT_RCP_
    rcpinput = input_allocate_device();
	if (!rcpinput)
    {
	    pr_err("IT6681 --failed to allocate RCP input device.\n");
		ret =  -ENOMEM;
		goto err_exit;
	}

	set_bit(EV_KEY, rcpinput->evbit);
	bitmap_fill(rcpinput->keybit, KEY_MAX);

	ddata->rcp_input = rcpinput;

	input_set_drvdata(rcpinput, ddata);
	rcpinput->name = "it6681_rcp";
	rcpinput->id.bustype = BUS_I2C;

	ret = input_register_device(rcpinput);
	if (ret < 0)
    {
        pr_err("IT6681 --fail to register RCP input device. ret = %d (0x%08x)\n", ret, ret);
		goto err_exit_rcp;
	}
#endif

#if _SUPPORT_UCP_
	ucpinput = input_allocate_device();
	if (!ucpinput)
    {
		pr_err("IT6681 --failed to allocate UCP input device.\n");
		ret =  -ENOMEM;

		#if _SUPPORT_RCP_
		goto err_exit_rcp;
		#endif

		goto err_exit;
	}

	set_bit(EV_KEY, ucpinput->evbit);
	bitmap_fill(ucpinput->keybit, KEY_MAX);

	ddata->ucp_input = ucpinput;

	input_set_drvdata(ucpinput, ddata);
	ucpinput->name = "it6681_ucp";
	ucpinput->id.bustype = BUS_I2C;

	ret = input_register_device(ucpinput);
	if (ret < 0)
    {
		pr_err("IT6681 --fail to register UCP input device. ret = %d (0x%08x)\n", ret, ret);
		goto err_exit_ucp;
	}
#endif

#ifdef _SUPPORT_UCP_MOUSE_
	mouse_ucpinput = input_allocate_device();
	if (!mouse_ucpinput)
    {
		pr_err("IT6811 --failed to allocate UCP  MOUSE input device.\n");
		ret =  -ENOMEM;

		#ifdef _SUPPORT_UCP_
		goto err_exit_ucp;
		#endif

		#ifdef _SUPPORT_RCP_
		goto err_exit_rcp;
		#endif

		goto err_exit;
	}

    /* Announce that the virtual mouse will generate relative coordinates */
    set_bit(EV_REL, mouse_ucpinput->evbit);
    set_bit(REL_X, mouse_ucpinput->relbit);
    set_bit(REL_Y, mouse_ucpinput->relbit);
    set_bit(REL_WHEEL, mouse_ucpinput->relbit);

    /* Announce key event */
    set_bit(EV_KEY, mouse_ucpinput->evbit);
    set_bit(BTN_LEFT, mouse_ucpinput->keybit);
    set_bit(BTN_MIDDLE, mouse_ucpinput->keybit);
    set_bit(BTN_RIGHT, mouse_ucpinput->keybit);

	ddata->ucp_mouse_input = mouse_ucpinput;

	input_set_drvdata(mouse_ucpinput, ddata);
	mouse_ucpinput->name = "it6811_ucp_mouse";
	mouse_ucpinput->id.bustype = BUS_I2C;

	ret = input_register_device(mouse_ucpinput);
	if (ret < 0)
    {
		pr_err("IT6811 --fail to register UCP MOUSE input device. ret = %d (0x%08x)\n", ret, ret);
		goto err_exit_ucp_mouse;
	}
#endif

    pr_err("IT6681 -- %s --\n", __FUNCTION__);

	return 0;

#if _SUPPORT_UCP_MOUSE_
err_exit_ucp_mouse:
	input_free_device(mouse_ucpinput);
#endif

#if _SUPPORT_UCP_
err_exit_ucp:
	input_free_device(ucpinput);
#endif

#if _SUPPORT_RCP_
err_exit_rcp:
	input_free_device(rcpinput);
#endif

err_exit:
	kfree(ddata);

    pr_err( "IT6811 %s--, ret=%d ( 0x%08x )\n", __FUNCTION__, ret, ret );

    return ret;
}

static int __devinit it6681_hdmi_tx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct it6681_platform_data *pdata = client->dev.platform_data;

    pr_err("%s ++\n", __FUNCTION__);
	pdata->hdmi_tx_client = client;

	return 0;
}


static int __devinit it6681_mhl_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	struct it6681_platform_data *pdata = client->dev.platform_data;

    pr_err("%s ++\n", __FUNCTION__);
	pdata->mhl_client = client;

	return 0;
}


static int __devexit it6681_hdmi_rx_remove(struct i2c_client *client)
{
    struct it6681_data *it6681data;

    pr_err("%s ++\n", __FUNCTION__);

    it6681data = i2c_get_clientdata(client);

    if(it6681data->ddata->Aviinfo != NULL)
    {
        kfree(it6681data->ddata->Aviinfo);
    }

    kfree(it6681data);

	return 0;
}

static int __devexit it6681_hdmi_tx_remove(struct i2c_client *client)
{
    pr_err("%s ++\n", __FUNCTION__);
	return 0;
}

static int __devexit  it6681_mhl_remove(struct i2c_client *client)
{
    pr_err("%s ++\n", __FUNCTION__);
	return 0;
}
EXPORT_SYMBOL(it6681_i2c_poweroff);
int it6681_i2c_poweroff(void)
{
    unsigned char tmp;
	if(dependence==1){
		printk("it6681_i2c_poweroff do nothing...");
		return 0;
	}
    tmp = mhltxrd(0x0f);
    
    if ( 0 == (tmp & 0x01) ) { // if mhl mode, pull down CBUS
        mhltxset(0x0f, 0x10, 0x10);
        hdmitxset(0x64, 0x80, 0x80);
        msleep(20);
        hdmitxset(0x64, 0x80, 0x00);
    }
        return 0;
}

static int it6681_power_off_cb(struct notifier_block *nb,
                unsigned long event, void *unused)
{
        printk(KERN_INFO "it6681: rebooting cleanly.\n");
        switch (event) {
        case SYS_RESTART:
        case SYS_HALT:
        case SYS_POWER_OFF:
                it6681_i2c_poweroff();
                return NOTIFY_OK;
        }
        return NOTIFY_DONE;
}
struct notifier_block it6681_reboot_notifier = {
        .notifier_call = it6681_power_off_cb,
};


static const struct i2c_device_id it6681_hdmi_rx_id[] = {
	{"it6681_hdmi_rx", 0},
	{}
};

static const struct i2c_device_id it6681_hdmi_tx_id[] = {
	{"it6681_hdmi_tx", 0},
	{}
};

static const struct i2c_device_id it6681_mhl_id[] = {
	{"it6681_mhl", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, it6681_hdmi_rx_id);
MODULE_DEVICE_TABLE(i2c, it6681_hdmi_tx_id);
MODULE_DEVICE_TABLE(i2c, it6681_mhl_id);

static struct it6681_platform_data pdata_it6681;

static struct i2c_board_info  panda_i2c2_it6681[] = {
	{
		I2C_BOARD_INFO("it6681_hdmi_rx", IT6681_HDMI_RX_ADDR>>1),
                .platform_data = &pdata_it6681,
#ifdef CONFIG_AM_8251
				.irq = IRQ_EXT23,
#else
				.irq = IRQ_EXT,
#endif				
	},
	{
		I2C_BOARD_INFO("it6681_hdmi_tx", IT6681_HDMI_TX_ADDR>>1),
                .platform_data = &pdata_it6681,
	},
	{
		I2C_BOARD_INFO("it6681_mhl", IT6681_MHL_ADDR>>1),
                .platform_data = &pdata_it6681,
	},
};

static struct i2c_driver it6681_hdmi_rx_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "it6681_hdmi_rx",
	},
	.id_table = it6681_hdmi_rx_id,
	.probe = it6681_hdmi_rx_i2c_probe,
	.remove = __devexit_p(it6681_hdmi_rx_remove),

};

static struct i2c_driver it6681_hdmi_tx_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "it6681_hdmi_tx",
	},
	.id_table = it6681_hdmi_tx_id,
	.probe = it6681_hdmi_tx_i2c_probe,
	.remove = __devexit_p(it6681_hdmi_tx_remove),

};

static struct i2c_driver it6681_mhl_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "it6681_mhl",
	},
	.id_table = it6681_mhl_id,
	.probe = it6681_mhl_i2c_probe,
	.remove = __devexit_p(it6681_mhl_remove),
};


static int __init it6681_init(void)
{
	int ret;
    struct i2c_adapter *adap=NULL;
    struct i2c_board_info  i2c2_it6681_info;
     struct i2c_client *temp_client;

	 if (dependence)
	 {
	 	//lcm模块依赖edid函数，因此针对不需要it6681工作时，只加载，不工作。
	 	pr_err("it6681_init Just insmod ko\n");
		return 0;
	 }
	mhl_adopter_id_h = (mhl_adopter_id&0xff00)>>8;
	mhl_adopter_id_l = (mhl_adopter_id&0xff);
	mhl_device_id_h = (mhl_device_id&0xff00)>>8;
	mhl_device_id_l = (mhl_device_id&0xff);
	//初始化之前reset it6681 模块，SYSRSTN(GPIO25) LOW-->HIGH(1~2ms)
	//EXTINT0
	RegBitSet(0,GPIO_MFCTL0,23,22);
	RegBitSet(1,GPIO_31_0OUTEN,25,25);
	RegBitSet(0,GPIO_31_0DAT,25,25);
	mdelay(2);
	RegBitSet(1,GPIO_31_0DAT,25,25);
	/**********************************************************/
    adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		printk(KERN_ERR"no available am i2c bus\n");
		return  -1;
	}
    
       i2c2_it6681_info = panda_i2c2_it6681[0];
       temp_client = i2c_new_device(adap,&i2c2_it6681_info);
           if(!temp_client){
		printk(KERN_ERR"gen device error!\n");
		return -1;
	}
	((struct it6681_platform_data *)(i2c2_it6681_info.platform_data))->hdmi_rx_client= temp_client;


    i2c2_it6681_info = panda_i2c2_it6681[1];
       temp_client = i2c_new_device(adap,&i2c2_it6681_info);
           if(!temp_client){
		printk(KERN_ERR"gen device error!\n");
		return -1;
	}
    ((struct it6681_platform_data *)(i2c2_it6681_info.platform_data))->hdmi_tx_client= temp_client;
    
	
    i2c2_it6681_info = panda_i2c2_it6681[2];
       temp_client = i2c_new_device(adap,&i2c2_it6681_info);
           if(!temp_client){
		printk(KERN_ERR"gen device error!\n");
		return -1;
	}
	((struct it6681_platform_data *)(i2c2_it6681_info.platform_data))->mhl_client =temp_client;


    pr_err("it6681_init ++\n");

	ret = i2c_add_driver(&it6681_hdmi_rx_i2c_driver);
	if (ret < 0)
	{
	    pr_err("i2c_add_driver it6681_hdmi_rx_i2c_driver fail\n");
		return ret;
	}
	
	ret = register_reboot_notifier(&it6681_reboot_notifier);
    if (ret)
	{
        pr_err("it6681_init  register reboot notifier fail!!!!!!!!!\n");
    }

	ret = i2c_add_driver(&it6681_hdmi_tx_i2c_driver);
	if (ret < 0)
		goto err_exit0;

	ret = i2c_add_driver(&it6681_mhl_i2c_driver);
	if (ret < 0)
		goto err_exit1;

	if(ret <0)
		goto err_exit2;


	init_it6681_loop_kthread();
    pr_err("it6681_init done\n");

	return 0;

err_exit2:
	i2c_del_driver(&it6681_mhl_i2c_driver);
	pr_err("i2c_add_driver hdmitx_ini fail\n");
err_exit1:
	i2c_del_driver(&it6681_hdmi_tx_i2c_driver);
	pr_err("i2c_add_driver it6681_mhl_i2c_driver fail\n");
err_exit0:
	i2c_del_driver(&it6681_hdmi_tx_i2c_driver);
	pr_err("i2c_add_driver it6681_hdmi_tx_i2c_driver fail\n");

	return ret;
}

static void __exit it6681_exit(void)
{
    i2c_del_driver(&it6681_mhl_i2c_driver);
	i2c_del_driver(&it6681_hdmi_tx_i2c_driver);
	i2c_del_driver(&it6681_hdmi_rx_i2c_driver);
}
MODULE_AUTHOR("YangJY <@actions-micro.com>");
MODULE_DESCRIPTION("am7x it6681 hdmi2mhl driver");
MODULE_LICENSE("GPL");

module_init(it6681_init);
module_exit(it6681_exit);
