/* drivers/rtc/rtc-am7x.c
 *
 * Copyright (C) 2009 Actions MicroEletronics Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Driver for the AM7531/AM7331 RTC
 *
 * Changelog:
 *
 *  2009/11/04: Pan Ruochen <reachpan@actions-micro.com>
 *				- Port the driver from v3020 and rtc-test
 *
 * 2010/04/23: add read_alarm and set_alarm functions by yekai
 * 2010/09/09: add alarm irq and implement platform resource
 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/bcd.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <actions_io.h>
#include <am7x_platform_device.h>
#include <sys_rtc.h>
#include <irq.h>
#include <am7x_dev.h>

#define DEBUG_AM_RTC     0
#if     DEBUG_AM_RTC
#define DBG_RTC_MSG(format,args...)   printk(format,##args)
#else
#define DBG_RTC_MSG(format,args...)   do {} while (0)
#endif

struct am_rtc {
	struct resource *mem;
	void __iomem *ioaddress;
	struct rtc_device *rtc;
};

static struct am_rtc am7x_rtc;

struct am7x_rtc_dhms {
	unsigned int sec  : 6;
	unsigned int x1   : 2;
	unsigned int min  : 6;
	unsigned int x2   : 2;
	unsigned int hour : 5;
	unsigned int x3   : 3;
	unsigned int day  : 3;
	unsigned int x4   : 5;
};

struct am7x_rtc_ymd {
	unsigned int date  : 5;
	unsigned int x1    : 3;
	unsigned int mon   : 4;
	unsigned int x2    : 4;
	unsigned int year  : 7;
	unsigned int x3    : 1;
	unsigned int cent  : 7;
	unsigned int x4    : 1;
};

static void disable_rtc(void)
{
	RTC_WRITE(RTC_READ(RTC_CTL)&~(RTC_EN|RTC_NORMAL),RTC_CTL);
	RTC_WRITE((RTC_READ(RTC_CTL)&~RTC_EN)|RTC_NORMAL,RTC_CTL);
}

static void enable_rtc(void)
{
	RTC_WRITE(RTC_READ(RTC_CTL)|RTC_EN|RTC_NORMAL,RTC_CTL);
}

static int am7x_read_time(struct device *dev, struct rtc_time *dt)
{
	struct am7x_rtc_dhms dhms;
	struct am7x_rtc_ymd  ymd;

	*(unsigned int *)&dhms = RTC_READ(RTC_DHMS);
	*(unsigned int *)&ymd  = RTC_READ(RTC_YMD);

	dt->tm_hour = dhms.hour;
	dt->tm_min  = dhms.min;
	dt->tm_sec  = dhms.sec;
	dt->tm_wday = dhms.day;

	dt->tm_year = ymd.year + (100 * ymd.cent) - 1900;
	dt->tm_mon  = ymd.mon - 1;
	dt->tm_mday = ymd.date;
#if 0
	printk("\n%s : Read RTC values\n",__func__);
	printk("tm_hour: %i\n",dt->tm_hour);
	printk("tm_min : %i\n",dt->tm_min);
	printk("tm_sec : %i\n",dt->tm_sec);
	printk("tm_year: %i\n",dt->tm_year);
	printk("tm_mon : %i\n",dt->tm_mon);
	printk("tm_mday: %i\n",dt->tm_mday);
	printk("tm_wday: %i\n",dt->tm_wday);
#endif
	return 0;
}


static int am7x_set_time(struct device *dev, struct rtc_time *dt)
{
	struct am7x_rtc_dhms dhms;
	struct am7x_rtc_ymd  ymd;
	unsigned int year;

#if 0
	printk("\n%s : Setting RTC values\n",__func__);
	printk("tm_year: %i\n",dt->tm_year);
	printk("tm_mon: %i\n",dt->tm_mon);
	printk("tm_mday: %i\n",dt->tm_mday);
	printk("tm_wday:%i\n",dt->tm_wday);
	printk("tm_hour: %i\n",dt->tm_hour);
	printk("tm_min : %i\n",dt->tm_min);
	printk("tm_sec : %i\n",dt->tm_sec);
#endif
	*(unsigned int *)&dhms = 0;
	*(unsigned int *)&ymd  = 0;

	dhms.hour = dt->tm_hour;
	dhms.min  = dt->tm_min;
	dhms.sec  = dt->tm_sec;
	dhms.day  = dt->tm_wday;

	year = dt->tm_year + 1900;
	ymd.year = year % 100;
	ymd.cent = year / 100;
	ymd.mon  = dt->tm_mon + 1;
	ymd.date = dt->tm_mday;

	disable_rtc();
	RTC_WRITE(*(unsigned int *)&dhms, RTC_DHMS);
	RTC_WRITE(*(unsigned int *)&ymd,  RTC_YMD);
	enable_rtc();

	return 0;
}

/*As there is no century value in rtc alarm reg, year var may not be the set value */
static int am7x_read_alarm(struct device *dev, struct rtc_wkalrm *wa)
{
	struct am7x_rtc_dhms dhms;
	struct am7x_rtc_ymd  ymd;
	unsigned int  rtc_reg=0;

	DBG_RTC_MSG("read rtc alarm\n");
	rtc_reg = RTC_READ(RTC_CTL);
	wa->enabled = rtc_reg&RTC_ALARM_IRQ_EN;
		
	*(unsigned int *)&dhms = RTC_READ(RTC_DHMSALM);
	*(unsigned int *)&ymd  = RTC_READ(RTC_YMDALM);
	wa->time.tm_sec = dhms.sec;
	wa->time.tm_min = dhms.min;
	wa->time.tm_hour = dhms.hour;
	wa->time.tm_mday = ymd.date;
	wa->time.tm_mon = ymd.mon-1;
	wa->time.tm_year = ymd.year;
	
	rtc_reg = RTC_READ(RTC_STATUS);
	wa->pending = rtc_reg&RTC_ALARM_PD;
	
	return 0;
}

static int am7x_set_alarm(struct device *dev, struct rtc_wkalrm *wa)
{
	struct am7x_rtc_dhms dhms;
	struct am7x_rtc_ymd  ymd;
	unsigned int  rtc_reg=0,rtc_base;

	DBG_RTC_MSG("set rtc alarm\n");

	*(unsigned int *)&dhms = 0;
	*(unsigned int *)&ymd  = 0;
	dhms.sec = wa->time.tm_sec;
	dhms.min = wa->time.tm_min;
	dhms.hour = wa->time.tm_hour;
	ymd.date = wa->time.tm_mday;
	ymd.mon = wa->time.tm_mon + 1;
	ymd.year = wa->time.tm_year%100;

	RTC_WRITE(*(unsigned int *)&dhms, RTC_DHMSALM);
	RTC_WRITE(*(unsigned int *)&ymd,RTC_YMDALM);

	rtc_base = (unsigned int)(am7x_rtc.ioaddress);
	if((wa->enabled)!=RTC_ALARM_UNDO){
	#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID==1220
		/* enable alarm 0*/
		rtc_reg =RTC_READ(rtc_base+RTC_ALARM_CTL_OFF);
		rtc_reg =  (wa->enabled==RTC_ALARM_ENABLE)?(rtc_reg|RTC_ALARM_EN):(rtc_reg&~RTC_ALARM_EN);
		RTC_WRITE(rtc_reg,rtc_base+RTC_ALARM_CTL_OFF);;
	#endif
		/*enable alarm irq*/
		//disable_rtc();
		rtc_reg = RTC_READ(rtc_base+RTC_CTL_OFF);
		rtc_reg = (wa->enabled==RTC_ALARM_ENABLE)?(rtc_reg|RTC_ALARM_IRQ_EN):(rtc_reg&~RTC_ALARM_IRQ_EN);
		RTC_WRITE(rtc_reg,rtc_base+RTC_CTL_OFF);
		//enable_rtc();
	}
	
	return 0;
}

static irqreturn_t am7x_rtc_alarmirq(int irq, void *id)
{
	struct am_rtc *rdev = (struct am_rtc *)id;

	DBG_RTC_MSG("[rtc]irq\n");
	RTC_IRQ_CLEAR_PENDING(RTC_ALARM_PD,(unsigned int)(rdev->ioaddress)+RTC_STAT_OFF);
	rtc_update_irq(rdev->rtc, 1, RTC_AF | RTC_IRQF);
	return IRQ_HANDLED;
}

static int am7x_open_rtc(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct am_rtc  *rtc_dev = platform_get_drvdata(pdev);
	int ret;
	
	ret = request_irq(IRQ_RTC, am7x_rtc_alarmirq, IRQF_DISABLED,  "am7x-rtc alarm", rtc_dev);
	if(ret){
		printk(KERN_ERR"am7x-rtc alarm irq error\n");
	}

	return ret;
}

static void am7x_release_rtc(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct am_rtc  *rtc_dev = platform_get_drvdata(pdev);

	free_irq(IRQ_RTC,rtc_dev);
}

static const struct rtc_class_ops am7x_rtc_ops = {
	.open = am7x_open_rtc,
	.release = am7x_release_rtc,
	.read_time = am7x_read_time,
	.set_time	= am7x_set_time,
	.read_alarm = am7x_read_alarm,
	.set_alarm = am7x_set_alarm,
};

static int __init rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	int retval = -EBUSY;

	am7x_rtc.mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!am7x_rtc.mem)
		return -EINVAL;

//am_printf("virt_to_phys(0x%08x)=0x%08x\n", RTC_CTL, virt_to_phys(RTC_CTL));

#if 1
	am7x_rtc.ioaddress = ioremap(am7x_rtc.mem->start, am7x_rtc.mem->end - am7x_rtc.mem->start + 1);
	if (am7x_rtc.ioaddress == NULL) {
		printk("%s: ioremap failed\n", __func__);
		goto err_chip;
	}
#endif

	rtc = rtc_device_register("rtc-am7xxx", &pdev->dev, &am7x_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		retval = PTR_ERR(rtc);
		printk("%s: rtc_device_register failed, ret=%d\n", __func__, retval);
		goto err_io;
	}
	am7x_rtc.rtc = rtc;
	platform_set_drvdata(pdev, &am7x_rtc);
	
	am_enable_dev_clk(2, CMU_DEVCLKEN); /* Enable RTC module */
	
	return 0;

err_io:
	iounmap(am7x_rtc.ioaddress);
err_chip:
	release_resource(am7x_rtc.mem);
	
	return retval;
}

static int rtc_remove(struct platform_device *pdev)
{
	struct am_rtc *chip = platform_get_drvdata(pdev);
	struct rtc_device *rtc = chip->rtc;

	platform_set_drvdata(pdev,NULL);
	if (rtc)
		rtc_device_unregister(rtc);
	iounmap(chip->ioaddress);
	release_resource(chip->mem);
	return 0;
}

static struct platform_driver rtc_device_driver = {
	.probe      	= rtc_probe,
	.remove		= rtc_remove,
	.driver		= {
		.name	= AM7X_DEV_NAME_RTC,
		.owner	= THIS_MODULE,
	},
};

static int __init am7x_rtc_init(void)
{
//	return platform_driver_register(&rtc_device_driver);
	int retval = platform_driver_probe(&rtc_device_driver, rtc_probe);
	if(retval < 0)
		printk("%s failed, err=%d\n",__func__,retval);
	return retval;
}

static __exit void am7x_rtc_exit(void)
{
	platform_driver_unregister(&rtc_device_driver);
}

module_init(am7x_rtc_init);
module_exit(am7x_rtc_exit);

MODULE_DESCRIPTION("AM7XXX RTC");
MODULE_AUTHOR("Pan Ruochen <reachpan@actions-micro.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:am7xxx");

