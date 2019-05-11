/* drivers/rtc/rtc-pt7c4337.c
 *
 * Copyright (C) 2009 Actions MicroEletronics Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *change log:
 *
 *	2011/07/06  scopeng@gmail.com   initial version 
*/

#include <linux/module.h>
#include <linux/init.h>


#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/ctype.h>


#include <am7x_i2c.h>


//#define DEBUG_PT
#ifdef DEBUG_PT
	#define PR(fmt...) printk(fmt)
#else
	#define PR(fmt...)
#endif

#define PT7C4337_DEVICE_NAME  "pt7c4337_rtc"
#define PT7C4337_I2C_ADDR	  0x68 	

#define SEC_REG_ADDR		0x0	
#define MIN_REG_ADDR		0x1
#define HOR_REG_ADDR		0x2
#define DAY_REG_ADDR		0x3
#define DAT_REG_ADDR		0x4
#define MON_REG_ADDR		0x5
#define YER_REG_ADDR		0x6
#define A1SEC_REG_ADDR		0x7
#define A1MIN_REG_ADDR		0x8
#define A1HOR_REG_ADDR		0x9
#define A1DAY_REG_ADDR	 	0xa
#define A2MIN_REG_ADDR		0xb
#define A2HOR_REG_ADDR		0xc
#define A2DAY_REG_ADDR		0xd
#define CTL_REG_ADDR		0xe
#define STA_REG_ADDR		0xf



static struct i2c_client *pt7c4337_client;

static struct i2c_board_info pt7c4337_info = {I2C_BOARD_INFO(PT7C4337_DEVICE_NAME,PT7C4337_I2C_ADDR)};


#define PT_SEC(sec)    (sec&0xf)+((sec&0x10)>>4)*10+((sec&0x20)>>5)*20+((sec&0x40)>>6)*40
#define PT_MIN(min)    (min&0xf)+((min&0x10)>>4)*10+((min&0x20)>>5)*20+((min&0x40)>>6)*40 
static inline unsigned int PT_HOR(unsigned char reg)   
{
	if(reg&0x40) // 12hour system
	{
		if(reg&0x20)  //PM
		{
			if(reg == 0x72)
				return 0+12;
			else 
				return (reg-0x60)+12;
		}
		else  //AM
		{
			if(reg == 0x52)
				return 0;
			else
				return (reg-0x40);
		}
	}

	return (reg&0xf)+((reg&0x10)>>4)*10+((reg&0x20)>>5)*20;

}

#define PT_DATE(date)		(date&0xf)+((date&0x10)>>4)*10+((date&0x20)>>5)*20	
#define PT_DAY(day)	 		(day&0x7)
#define PT_MON(mon)	 		(mon&0xf)+((mon&0x10)>>4)*10
#define PT_YEAR(year)		(year&0xf)+((year&0x10)>>4)*10+((year&0x20)>>5)*20+((year&0x40)>>6)*40+((year&0x80)>>7)*80 		

#define SEC_PT(sec)			(sec/40)*0x40+((sec%40)/20)*0x20+((sec%20)/10)*0x10+sec%10
#define MIN_PT(min)			(min/40)*0x40+((min%40)/20)*0x20+((min%20)/10)*0x10+min%10
#define HOR_PT(hor)			(hor/20)*0x20+((hor%20)/10)*0x10+hor%10
#define DATE_PT(date)		(date/20)*0x20+((date%20)/10)*0x10+date%10
#define DAY_PT(day)			day&0x7
#define MON_PT(mon)			(mon/10)*0x10+mon%10
#define YEAR_PT(year)		(year/80)*0x80+((year%80)/40)*0x40+((year%40)/20)*0x20+((year%20)/10)*0x10+year%10



static int pt7c4337_read_reg(unsigned char reg,unsigned char* value)
{
	struct i2c_msg		msg[2];

	msg[0].addr = pt7c4337_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = pt7c4337_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = value;

	return i2c_transfer(pt7c4337_client->adapter,msg,2);
}

static int pt7c4337_write_reg(unsigned char reg,unsigned char value)
{
	struct i2c_msg		msg;
	unsigned char buffer[2];

	buffer[0] = reg;
	buffer[1] = value;

	msg.addr = pt7c4337_client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = buffer;


	return i2c_transfer(pt7c4337_client->adapter,&msg,1);
}

static int pt7c4337_open_rtc(struct device *pdev)
{
	PR("%s %s\n",__FILE__,__FUNCTION__);
	return 0;	
}

static void pt7c4337_release_rtc(struct device *pdev)
{
}

static int pt7c4337_read_time(struct device *pdev, struct rtc_time *time)
{
	unsigned char value; 

	pt7c4337_read_reg(SEC_REG_ADDR,&value);
	time->tm_sec = PT_SEC(value);
	if(time->tm_sec >= 60)
		time->tm_sec = 0;
	PR("value:%x sec:%d\n",value,time->tm_sec);

	pt7c4337_read_reg(MIN_REG_ADDR,&value);
	time->tm_min = PT_MIN(value);
	if(time->tm_min >= 60)
		time->tm_min = 0;
	PR("value:%x min:%d\n",value,time->tm_min);

	pt7c4337_read_reg(HOR_REG_ADDR,&value);
	time->tm_hour = PT_HOR(value);
	if(time->tm_hour >= 24)
		time->tm_hour = 0;	
	PR("value:%x hor:%d\n",value,time->tm_hour);

	pt7c4337_read_reg(DAY_REG_ADDR,&value);
	time->tm_wday = PT_DATE(value);
	if(time->tm_wday >= 8)
		time->tm_wday = 1;		
	PR("value:%x wday:%d\n",value,time->tm_wday);

	pt7c4337_read_reg(DAT_REG_ADDR,&value);
	time->tm_mday = PT_DATE(value);
	if(time->tm_mday >= 32)
		time->tm_mday = 1;		
	PR("value:%x mday:%d\n",value,time->tm_mday);

	pt7c4337_read_reg(MON_REG_ADDR,&value);
	time->tm_mon= PT_DATE(value);
	if(time->tm_mon >= 13)
		time->tm_mon = 1;
	time->tm_mon -=1;  //fix for compatibility
	PR("value:%x tm_mon:%d\n",value,time->tm_mon);

	pt7c4337_read_reg(YER_REG_ADDR,&value);
	time->tm_year = PT_DATE(value);
	if(time->tm_year >= 100)
		time->tm_year = 1;	
	time->tm_year += 100;//fix for compatibility
	PR("value:%x tm_year:%d\n",value,time->tm_year);

	return 0;	
}

static int pt7c4337_set_time(struct device *pdev, struct rtc_time *time)
{
	unsigned char value; 	

	PR("&&&&&&&&&&&&&&&&");
	value = SEC_PT(time->tm_sec);
	PR("sec:%d value:%x\n",time->tm_sec,value);
	pt7c4337_write_reg(SEC_REG_ADDR,value);

	value = MIN_PT(time->tm_min);
	PR("min:%d value:%x\n",time->tm_min,value);
	pt7c4337_write_reg(MIN_REG_ADDR,value);
	
	value = HOR_PT(time->tm_hour);
	PR("tm_hour:%d value:%x\n",time->tm_hour,value);
	pt7c4337_write_reg(HOR_REG_ADDR,value);
	
	value = DAY_PT(time->tm_wday);
	PR("tm_wday:%d value:%x\n",time->tm_wday,value);
	pt7c4337_write_reg(DAY_REG_ADDR,value);

	value = DATE_PT(time->tm_mday);
	PR("tm_mday:%d value:%x\n",time->tm_mday,value);
	pt7c4337_write_reg(DAT_REG_ADDR,value);

	value = MON_PT(time->tm_mon);
	value += 1;//fix for compatibility
	PR("tm_mon:%d value:%x\n",time->tm_mon,value);	
	pt7c4337_write_reg(MON_REG_ADDR,value);

	value = YEAR_PT(time->tm_year%100);
	pt7c4337_write_reg(YER_REG_ADDR,value);
	

	return 0;
}

static int pt7c4337_read_alarm(struct device *pdev, struct rtc_wkalrm *wtime)
{
	unsigned char value;

	pt7c4337_read_reg(CTL_REG_ADDR,&value);
	wtime->enabled = value&0x1;

	pt7c4337_read_reg(A1SEC_REG_ADDR,&value);	
	wtime->time.tm_sec =  PT_SEC(value);

	pt7c4337_read_reg(A1MIN_REG_ADDR,&value);	
	wtime->time.tm_min = PT_MIN(value);

	pt7c4337_read_reg(A1HOR_REG_ADDR,&value);			
	wtime->time.tm_hour = PT_HOR(value);

	pt7c4337_read_reg(A1DAY_REG_ADDR,&value);	
	if(value&0x40)
		wtime->time.tm_wday = PT_DAY(value);
	else	
		wtime->time.tm_mday = PT_DATE(value);

	PR("al value:%x tm_Wday:%d\n",value,wtime->time.tm_wday);
	PR("al value:%x tm_Mday:%d\n",value,wtime->time.tm_mday);
	
	pt7c4337_read_reg(MON_REG_ADDR,&value);
	wtime->time.tm_mon= PT_DATE(value);
	if(wtime->time.tm_mon >= 13)
		wtime->time.tm_mon = 1;	
	wtime->time.tm_mon -= 1;//fix for compatibility
	PR("al value:%x tm_mon:%d\n",value,wtime->time.tm_mon);

	pt7c4337_read_reg(YER_REG_ADDR,&value);
	wtime->time.tm_year = PT_DATE(value);
	if(wtime->time.tm_year >= 100)
		wtime->time.tm_year = 1;	
	wtime->time.tm_year += 100;//fix for compatibility
	PR("al value:%x tm_year:%d\n",value,wtime->time.tm_year);
	
	return 0;
}

static int pt7c4337_set_alarm(struct device *pdev, struct rtc_wkalrm *wtime)
{
	unsigned char value;

	if(wtime->time.tm_sec>=0&&wtime->time.tm_sec<=59)
		value = SEC_PT(wtime->time.tm_sec);
	else
		value = 0x80;
	pt7c4337_write_reg(A1SEC_REG_ADDR,value);

	if(wtime->time.tm_min>=0&&wtime->time.tm_min<=59)
		value = MIN_PT(wtime->time.tm_min);
	else
		value = 0x80;
	pt7c4337_write_reg(A1MIN_REG_ADDR,value);

	if(wtime->time.tm_hour>=0&&wtime->time.tm_hour<=23)
		value = HOR_PT(wtime->time.tm_hour);
	else
		value = 0x80;
	pt7c4337_write_reg(A1HOR_REG_ADDR,value);


	PR("wtime->tm_wday:%d  wtime->tm_mday:%d\n",wtime->time.tm_wday,wtime->time.tm_mday);
	if(wtime->time.tm_wday>=1&&wtime->time.tm_wday<=7)
		value = (DAY_PT(wtime->time.tm_wday))|(0x40);
	else if(wtime->time.tm_mday>=1&&wtime->time.tm_mday<=31)
		value = DATE_PT(wtime->time.tm_mday);
	else
		value |= 0x80; 

	PR("al set value:%x tm_wday:%d\n",value,wtime->time.tm_wday);

	pt7c4337_write_reg(A1DAY_REG_ADDR,value);	

	pt7c4337_read_reg(CTL_REG_ADDR,&value);
	if(wtime->enabled)
		value |=0x1;
	else
		value &=0xe;
	pt7c4337_write_reg(CTL_REG_ADDR,value);


	return 0;
}



static const struct rtc_class_ops pt7c4337_rtc_ops = {
	.open = pt7c4337_open_rtc,
	.release = pt7c4337_release_rtc,
	.read_time = pt7c4337_read_time,
	.set_time	= pt7c4337_set_time,
	.read_alarm = pt7c4337_read_alarm,
	.set_alarm = pt7c4337_set_alarm,
};





static int pt7c4337_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int ret = 0;	
	struct rtc_device *rtc = NULL; 

	
	/** register device to rtc substem  **/
	rtc = rtc_device_register(PT7C4337_DEVICE_NAME,&pt7c4337_client->dev,&pt7c4337_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		ret = PTR_ERR(rtc);
		PR(KERN_ERR"%s: rtc_device_register failed, ret=%d\n", __func__, ret);
		goto exit;
	}


	dev_set_drvdata(&pt7c4337_client->dev,rtc);

#if 0 //for test
while(1)
{
	__u8 value =0;
	__u8 value2[4];
	int i = 0;
	for(i=0x0;i<0x10;i++){
	//	value = 0;
		pt7c4337_read_reg(i,&value);
		PR("reg %d:%x\n",i,value);
	}

}
#endif

exit:
	return ret;
}

static int pt7c4337_remove(struct i2c_client *client)
{
	struct rtc_device *rtc = dev_get_drvdata(&client->dev);

	PR("%s %s\n",__FILE__,__FUNCTION__);
	rtc_device_unregister(rtc);

	return 0;
}

static const struct i2c_device_id pt7c4337_id[] = {
	{ PT7C4337_DEVICE_NAME, 0 },
	{ }
};

static struct i2c_driver pt7c4337_driver = 
{
	.probe = pt7c4337_probe,
    .remove = pt7c4337_remove,
	.driver		= {
		.name	= PT7C4337_DEVICE_NAME,
	},
	.id_table = pt7c4337_id,
};

static int __init pt7c4337_init(void)
{
	int ret = 0;
	struct i2c_adapter *adap=NULL;

	adap = i2c_get_adapter(am_get_i2c_bus_id());
	if(!adap){
		PR(KERN_ERR"no available am i2c bus\n");
		ret = -ENXIO;
		goto exit;
	}

	pt7c4337_client = i2c_new_device(adap,&pt7c4337_info);
	if(!pt7c4337_client){
		PR(KERN_ERR"gen device error!\n");
		ret =  -ENXIO;
		goto i2c_register_err;
	}
	ret = i2c_add_driver(&pt7c4337_driver);
	if(ret)
		i2c_unregister_device(pt7c4337_client);
i2c_register_err:	
	i2c_put_adapter(adap);
exit:
	
	return ret;
}

static void __exit pt7c4337_exit(void)
{
	i2c_del_driver(&pt7c4337_driver);
	i2c_unregister_device(pt7c4337_client);
	i2c_put_adapter(pt7c4337_client->adapter);
}


module_init(pt7c4337_init);
module_exit(pt7c4337_exit);

MODULE_DESCRIPTION("rtc driver for pt4c4337 on linux platform");
MODULE_AUTHOR("zeng tao<scopengl@gmail.com>");
MODULE_LICENSE("GPL");
