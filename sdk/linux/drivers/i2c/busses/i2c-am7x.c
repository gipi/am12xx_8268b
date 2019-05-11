/*
    i2c-am7x.c
    This module mainly supports I2C for actions-micro AM7xxx chips

    Author: yekai
    Date:2010-08-20
    Version:0.1
*/
//#include <linux/init.h>
#include <linux/module.h>
//#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <asm/delay.h>
#include <actions_io.h>
#include <am7x_i2c.h>
#include <am7x-freq.h>
#include <am7x_dev.h>
#ifdef CONFIG_AM_8251
#include <am7x_board.h>
#endif
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include"../../../../../scripts/mconfig.h"
#endif

struct am7x_i2c_data{
	unsigned int i2c_base;
	unsigned int i2c_mode;
	unsigned int i2c_state;
};

static struct am7x_i2c_data am_i2c_data = {
	.i2c_base = AM_I2C_BASE,
	.i2c_mode = AM_I2C_NORMAL,
	.i2c_state = AM_I2C_BUS_IDLE,
};
static inline void am_set_i2c_ctl(unsigned int val, unsigned int reg_base)
{
	am7x_writel(val,reg_base+AM_I2C_CTL_OFFSET);
}

static inline void am_set_i2c_clk(unsigned int div, unsigned int reg_base)
{
	am7x_writel(div,reg_base+AM_I2C_CLKDIV_OFFSET);
}

static inline void am_set_i2c_addr(__u16 addr,unsigned int  reg_base,unsigned int mode)
{
	am7x_writel(addr<<1|(mode&0x1),reg_base+AM_I2C_ADDR_OFFSET);
}

static inline void am_set_i2c_data(unsigned int val, unsigned int reg_base)
{
	am7x_writel(val,reg_base+AM_I2C_DAT_OFFSET);
}

static inline void am_set_i2c_state(unsigned int val,unsigned int reg_base)
{
	am7x_writel(val,reg_base+AM_I2C_STAT_OFFSET);
}

static inline unsigned int am_get_i2c_ctl(unsigned int reg_base)
{
	return am7x_readl(reg_base+AM_I2C_CTL_OFFSET);
}

static inline unsigned char am_get_i2c_data(unsigned int reg_base)
{
	return (unsigned char)am7x_readl(reg_base+AM_I2C_DAT_OFFSET);
}

static inline unsigned int am_get_i2c_state(unsigned int reg_base)
{
	return am7x_readl(reg_base+AM_I2C_STAT_OFFSET);
}

static int am_i2c_init_hardware(void)
{
	
	
	/* init i2c hardware here, multi pad etc.*/
	am_enable_dev_clk(23, CMU_DEVCLKEN);
	
	//am7x_writel((am7x_readl(GPIO_MFCTL2)&0xffff00ff)|0x5500,GPIO_MFCTL2);
	//am7x_writel((am7x_readl(GPIO_MFCTL5)&0x0fffffff)|0x50000000,GPIO_MFCTL5);
#if (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
	am7x_writel((am7x_readl(GPIO_MFCTL0)&(~(0x03 <<1)))|(0x3 <<1),GPIO_MFCTL0);//get pin for scl and sda 8258
#else
#ifdef CONFIG_AM_8251
 
	am7x_writel((am7x_readl(GPIO_MFCTL4)&(~(0x07 <<16)))|(0x2 <<16),GPIO_MFCTL4);//get pin for scl
#endif  

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	printk("Probox use I2c1 for mac_address !!!/n");
	am7x_writel(((am7x_readl(GPIO_MFCTL4)&(~(0x0f<<7)))|(0xa<<7)),GPIO_MFCTL4);
#endif
#endif

	am_reset_dev(27,CMU_DEVRST);
	am_set_i2c_ctl(IIC_RESET, am_i2c_data.i2c_base);
	
	am_set_i2c_clk(get_pclk()/(16*am_i2c_data.i2c_mode)+1,am_i2c_data.i2c_base);
	printk(KERN_DEBUG"i2c clk div = %x @ %ld\n",am7x_readl(am_i2c_data.i2c_base+AM_I2C_CLKDIV_OFFSET),get_pclk());

	return 0;
}

static int am_i2c_start(unsigned int i2c_base,unsigned int dev_addr,unsigned int flags)
{
	unsigned int cmd=0,start_flags=0,stop_flags=0;

	start_flags = flags&AM_I2C_START_MASK;
	stop_flags = flags&AM_I2C_STOP_MASK;

	if(start_flags == AM_I2C_START_NO_REP){
		switch(stop_flags){
		case AM_I2C_STOP_ACK:
			cmd = IIC_REC_ACK_CMD;
			break;
		case AM_I2C_STOP_NO_ACK:
			cmd = IIC_REC_NO_ACK_CMD;
			break;
		case AM_I2C_STOP:
			cmd = IIC_STOP_CMD;
			break;
		default:
			cmd = IIC_SEND_NO_REP_CMD;
			break;
		}
	}else{
		am_set_i2c_addr(dev_addr,i2c_base,flags&AM_I2C_READ);
		printk(KERN_DEBUG"i2c addr 0x%x\n",am7x_readl(i2c_base+AM_I2C_ADDR_OFFSET));
		if(start_flags == AM_I2C_START)
			cmd = IIC_SEND_START_CMD;
		else if(start_flags == AM_I2C_START_REPEAT)
			cmd = IIC_SEND_REP_CMD;
		else{
			printk(KERN_ERR"start flags error 0x%x!\n",flags);
			return -EINVAL;
		}
	}
	am_set_i2c_ctl(cmd,i2c_base);
	printk(KERN_DEBUG"i2c ctl 0x%x,cmd=%x\n",am_get_i2c_ctl(i2c_base),cmd);
	
	return 0;
}

static int am_i2c_wait_ack(unsigned int i2c_base,unsigned int flags)
{
	int timeout = AM_I2C_TIMEOUT;
	unsigned int stat=0;
	
	if((flags&AM_I2C_STOP_MASK)==AM_I2C_STOP){
		do{
			stat = am_get_i2c_state(i2c_base);
			if(stat&AM_I2C_STPD_MASK){
				am_set_i2c_state(stat|AM_I2C_STPD_MASK,i2c_base);
				break;
			}
		}while(--timeout);

		goto WAIT_END;
	}
	
	do{
		stat = am_get_i2c_state(i2c_base);
		if(stat&AM_I2C_TRC_MASK){
			am_set_i2c_state(stat|AM_I2C_TRC_MASK,i2c_base);
			break;
		}
	}while(--timeout);
	if(!timeout)
		goto WAIT_END;
	
	if(!(flags&AM_I2C_READ)){
		do{
			stat = am_get_i2c_ctl(i2c_base);
			if(stat&AM_I2C_ACK_MASK){
				break;
			}
		}while(--timeout);
	}
	
WAIT_END:
	if(timeout){
		return 0;
	}
	else{
		printk(KERN_ERR"i2c ack timeout, ctl = %x,stat = %x\n",am_get_i2c_ctl(i2c_base),stat);
		return -EBUSY;
	}
}

static int am_i2c_read(__u16 device,  __u8 *buf, __u16 len , __u32 flags)
{
	int ret=0;
	
	if(!buf || !len)
		return -EINVAL;

	/* send address data */
	am_i2c_start(am_i2c_data.i2c_base, device, AM_I2C_READ|flags);
	ret = am_i2c_wait_ack(am_i2c_data.i2c_base, AM_I2C_WRITE);
	if(ret)
		goto READ_FAIL;

	while(len){
		if((len==1)&&(flags&AM_I2C_LAST))
			am_i2c_start(am_i2c_data.i2c_base,device, AM_I2C_READ|AM_I2C_START_NO_REP|AM_I2C_STOP_NO_ACK);
		else
			am_i2c_start(am_i2c_data.i2c_base,device, AM_I2C_READ|AM_I2C_START_NO_REP|AM_I2C_STOP_ACK);
		ret = am_i2c_wait_ack(am_i2c_data.i2c_base, AM_I2C_READ);
		if(ret){
			goto READ_FAIL;
		}else{
			*buf++ = am_get_i2c_data(am_i2c_data.i2c_base);
			len--;
		}
	}
	return 0;

READ_FAIL:
	printk(KERN_DEBUG"i2c read timeout\n");

	return ret;
}

static int am_i2c_write(__u16 device,  __u8 *buf, __u16 len ,__u32 flags)
{
	int ret = 0;

	if(!buf || !len)
		return -EINVAL;

	am_i2c_start(am_i2c_data.i2c_base, device, AM_I2C_WRITE|flags);
	ret = am_i2c_wait_ack(am_i2c_data.i2c_base, AM_I2C_WRITE);
	if(ret)
		goto WRITE_FAIL;

	while(len){
		am_set_i2c_data(*buf++,am_i2c_data.i2c_base);
		am_i2c_start(am_i2c_data.i2c_base,device, AM_I2C_WRITE|AM_I2C_START_NO_REP);
		ret = am_i2c_wait_ack(am_i2c_data.i2c_base, AM_I2C_WRITE);
		if(ret){
			goto WRITE_FAIL;
		}else{
			len--;
		}
	}
	return 0;

WRITE_FAIL:
	printk(KERN_DEBUG"i2c write timeout\n");
	return ret;
}

static int do_am_i2c_xfer(struct i2c_adapter *adap,struct i2c_msg *msgs, int num)
{
	unsigned int cur_num = 0, xfer_flags=0;
	struct am7x_i2c_data *pdata=0;
	int ret = 0;

	pdata = (struct am7x_i2c_data *)adap->algo_data;
#if (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
	unsigned int gpio_mf0;
    gpio_mf0 = am7x_readl(GPIO_MFCTL0);
	am_get_multi_res(MAC_I2C);
	am7x_writel((am7x_readl(GPIO_MFCTL0)&(~(0x01 << 2)))|(0x1 << 2),GPIO_MFCTL0);//get pin for sda 8258
#else
#ifdef CONFIG_AM_8251
	unsigned int gpio_mf1;
	gpio_mf1 = am7x_readl(GPIO_MFCTL1);
	am_get_multi_res(MAC_I2C);
	am7x_writel((am7x_readl(GPIO_MFCTL1)&(~(0x03 << 26)))|(0x2 << 26),GPIO_MFCTL1);//get pin for sda
#endif 	
#endif
	while(cur_num<num){
		if(pdata->i2c_state == AM_I2C_BUS_IDLE)
			xfer_flags = AM_I2C_START;
		else
			xfer_flags = AM_I2C_START_REPEAT;
		if((cur_num+1)==num)
			xfer_flags |= AM_I2C_LAST;
		msgs += cur_num;
		
		if(msgs->flags&I2C_M_RD){
			/*I2C read*/
			pdata->i2c_state = AM_I2C_BUS_READ;
			ret = am_i2c_read(msgs->addr,msgs->buf,msgs->len,xfer_flags);
			if(ret)
				break;
		}else{
			/*I2C write*/
			pdata->i2c_state = AM_I2C_BUS_WRITE;
			ret = am_i2c_write(msgs->addr,msgs->buf,msgs->len,xfer_flags);
			if(ret)
				break;
		}
		cur_num++;
	}

	am_i2c_start(pdata->i2c_base,msgs->addr,AM_I2C_READ|AM_I2C_START_NO_REP|AM_I2C_STOP);
	am_i2c_wait_ack(pdata->i2c_base, AM_I2C_STOP);
#if (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
  
	 am7x_writel(gpio_mf0,GPIO_MFCTL0);
	 am_release_multi_res(MAC_I2C);
#else
#ifdef CONFIG_AM_8251		
	am7x_writel(gpio_mf1,GPIO_MFCTL1);
	am_release_multi_res(MAC_I2C);
#endif
#endif

	pdata->i2c_state = AM_I2C_BUS_IDLE;

	if(ret)
		return ret;
	
	return cur_num;
}

static int am_xfer(struct i2c_adapter *adap,struct i2c_msg *msgs, int num)
{
	int i,ret=0;

	for(i=0;i<adap->retries;i++){
		ret = do_am_i2c_xfer(adap,msgs,num);

		if (ret != -EAGAIN)
			return ret;
		else
			udelay(100);
	}
	
	return ret;
}

static u32 am_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm am7x_algorithm = {
	.functionality	= am_func,
	.master_xfer	= am_xfer,
};

static struct i2c_adapter am_adapter = {
	.owner		= THIS_MODULE,
	.class		= I2C_CLASS_HWMON | I2C_CLASS_SPD,
	.algo		= &am7x_algorithm,
	.algo_data	= &am_i2c_data,
	.retries 		= 2,
	.name		= "am7x i2c",
};

int am_get_i2c_bus_id(void)
{
	return i2c_adapter_id(&am_adapter);
}
EXPORT_SYMBOL(am_get_i2c_bus_id);

static int __init i2c_am_init(void)
{
	int ret;

	am_i2c_init_hardware();
	ret = i2c_add_adapter(&am_adapter);

	return ret;
}

static void __exit i2c_am_exit(void)
{
	i2c_del_adapter(&am_adapter);
}

MODULE_AUTHOR("Ye Kai<yekai@actions-micro.com>");
MODULE_DESCRIPTION("I2C driver");
MODULE_LICENSE("GPL");

module_init(i2c_am_init);
module_exit(i2c_am_exit);

