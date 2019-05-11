/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_battery.c

@abstract: actions mirco 1211 soc battery device driver.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Power Management
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#include <am_types.h>
#include <am7x_gpio.h>
#include <actions_regs.h>
#include <actions_io.h>
#include <am7x_board.h>
#include <am7x_pm.h>

#define AM7X_BATTERY_DEVICENAME	 "am7x_battery_device"

static struct platform_device  *am7x_battery_device;

static unsigned long freq = 125; //4*250 = 1000ms 
static unsigned char charge_io = 255;
static unsigned char charge_io_valid_status = 1;
static unsigned char use_pmu = 0;
static int avg_vol = 0;
static int times = 0;//vol 10 time update once,in case of unstable	
#define CHECK_TIME		10


struct am7x_battery_device_info{
	struct power_supply *bat;
	int current_vol;
	int current_status;
	int current_temp;
	struct mutex work_lock;
	struct timer_list battery_timer;
};

static struct am7x_battery_device_info *am7x_battery_info;

static enum power_supply_property am7x_bat_main_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
};


static int badc_vol_table[] = {
	30000,
	30769,
	31538,
	32307,
	33076,
	33846,
	34615,
	35384,
	36153,
	36923,
	37692,
	38461,
	39230,
	40000,
	40769,
	41538,
};

#define PMU_VOL_BASE 26675
#define PMU_VOL_STEP 275

#define PMU_TEMP_BASE 245
#define PMU_TEMP_STEP 31


static int update_battery_status(void)
{
	int old_status,new_status;
	
	old_status = am7x_battery_info->current_status;
//	charge_io_num = 27;

	if(use_pmu == 0)
	{	
    	if(charge_io>=96)
        {
            //add by zhoujian  if charge_io>=96, io num is invalid
           new_status = POWER_SUPPLY_STATUS_DISCHARGING;
        }
        else
        {
            if(am_get_gpio(charge_io) == charge_io_valid_status)
    			new_status = POWER_SUPPLY_STATUS_CHARGING;
    		else
    			new_status = POWER_SUPPLY_STATUS_DISCHARGING;
        }
	}
	else
	{
		char reg = 0;
		am_pm_ioctl(AM_CHARGE_STA,AM_PM_GET,&reg);
		if(reg&(1<<6))
			new_status = POWER_SUPPLY_STATUS_CHARGING;
		else
			new_status = POWER_SUPPLY_STATUS_DISCHARGING;
	}


	if(old_status!=new_status) // status changed 
	{
		am7x_battery_info->current_status = new_status;
		return 1;
	}
	
	return 0;
}

static int update_battery_vol(void)
{
	int old_value,new_value;	

	old_value = am7x_battery_info->current_vol;

	if(use_pmu == 0)
		new_value = badc_vol_table[0xf&(act_readl(TBADC_VALUE)>>4)];
	else{
		unsigned char reg[2];
		am_pm_ioctl(AM_BAT_VOL,AM_PM_GET,&reg);
		new_value = (reg[1]&0x3f)* PMU_VOL_STEP + PMU_VOL_BASE;
	}

	avg_vol += new_value;
	times++;

	if(times == CHECK_TIME){
		avg_vol /= times; 
		if(old_value!=new_value)
		{
			am7x_battery_info->current_vol = avg_vol; //do not convert 
			avg_vol = 0;
			times = 0;
			return 1;
		}
		avg_vol = 0;
		times = 0;
	}
	return 0;
}

static int update_battery_temp(void)
{
	int old_value = 0,new_value = 0;
	char reg = 0;

	if(use_pmu == 0)
		return 0;
	
	am_pm_ioctl(AM_BAT_TEMP,AM_PM_GET,&reg);
	new_value = (reg&0x3f)*PMU_TEMP_STEP + PMU_TEMP_BASE;
	old_value = am7x_battery_info->current_temp;
	if(old_value!=new_value)
	{
		am7x_battery_info->current_temp = new_value; //do not convert 
		return 1;
	}
	return 0;
}

//am1211 hardware init for adc controller 
static int am7x_battery_hardware_init(void)
{
	act_writel(act_readl(CMU_SPCLK)|1<<11,CMU_SPCLK);// open dadc clock
	//default clock is 1khz
	act_writel(act_readl(AMU_CTL)|1<<10,AMU_CTL);// enable badc module
	return 0;
}

static int am7x_battery_hareware_exit(void)
{
	act_writel(act_readl(CMU_SPCLK)&(~(1<<11)),CMU_SPCLK);// close dadc clock
	//default clock is 1khz
	act_writel(act_readl(AMU_CTL)&(~(1<<10)),AMU_CTL);// disable badc module
	return 0;
}

static int am7x_bat_get_property(struct power_supply *bat_ps,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	mutex_lock(&am7x_battery_info->work_lock);
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = am7x_battery_info->current_status;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = am7x_battery_info->current_vol;
			break;
		case POWER_SUPPLY_PROP_TEMP:
			val->intval = am7x_battery_info->current_temp;
			break;
		default:
			mutex_unlock(&am7x_battery_info->work_lock);
			return -EINVAL;
	}
	mutex_unlock(&am7x_battery_info->work_lock);
	return 0;
}


struct power_supply bat_am7x = {
	.name			= "am7x-battery",
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties		= am7x_bat_main_props,
	.num_properties		= ARRAY_SIZE(am7x_bat_main_props),
	.get_property		= am7x_bat_get_property,
//	.external_power_changed = am7x_bat_external_power_changed,
	.use_for_apm		= 1,
};

static void am7x_battery_work(unsigned long arg)
{
#if 1
	int status_change = 0,vol_change = 0,temp_change = 0;

	status_change = update_battery_status();
	vol_change = update_battery_vol();
	temp_change = update_battery_temp();
	if(vol_change||status_change)
		power_supply_changed(am7x_battery_info->bat);
#endif
	mod_timer(&am7x_battery_info->battery_timer,jiffies + freq);
}


static int am7x_battery_probe(struct platform_device *pdev)
{
	int err = 0;

	am7x_battery_info = kzalloc(sizeof(*am7x_battery_info),GFP_KERNEL);
	if(!am7x_battery_info)
	{
		err = -ENOMEM;
		goto exit;
	}

	am7x_battery_info->bat = &bat_am7x;
	am7x_battery_info->current_status = POWER_SUPPLY_STATUS_UNKNOWN;
	am7x_battery_info->current_vol = 0;
	am7x_battery_info->current_temp = 0;
	mutex_init(&am7x_battery_info->work_lock);
	am7x_battery_info->battery_timer.function = am7x_battery_work;
	am7x_battery_info->battery_timer.expires = jiffies + freq;
	init_timer(&am7x_battery_info->battery_timer);

#if (CONFIG_AM_CHIP_ID == 1213)
	charge_io = 255;
#else
	charge_io = get_sys_info()->sys_gpio_cfg.charge_io;
#endif

	charge_io_valid_status = get_sys_info()->sys_gpio_cfg.charge_io_valid_status;
	use_pmu = get_sys_info()->sys_gpio_cfg.use_pmu;
	printk("charge io:%d : charging valid status %d\n",charge_io,charge_io_valid_status);
	printk("use pmu:%d\n",use_pmu);

	if(use_pmu == 0)
		am7x_battery_hardware_init();
	
	err = power_supply_register(&pdev->dev,am7x_battery_info->bat);
	if(likely(!err))
		add_timer(&am7x_battery_info->battery_timer);
	else
		kfree(am7x_battery_info);

	platform_set_drvdata(pdev,am7x_battery_info);
	
exit:	
	return err;
}

static int am7x_battery_remove(struct platform_device *pdev)
{
	//am7x_battery_probe must have been success 
	del_timer(&am7x_battery_info->battery_timer);
	power_supply_unregister(am7x_battery_info->bat);
	kfree(am7x_battery_info);

	return 0;
}

#ifdef CONFIG_PM

static int am7x_battery_suspend(struct platform_device* pdev,pm_message_t state)
{
	if(use_pmu == 0)
		am7x_battery_hareware_exit();
	return 0;
}

static int am7x_battery_resume(struct platform_device* pdev)
{
	if(use_pmu == 0)
		am7x_battery_hardware_init();
	return 0;
}

#endif

static struct platform_driver am7x_battery_driver = {
	.driver = {
		.name = AM7X_BATTERY_DEVICENAME,
	},
	.probe	  = am7x_battery_probe,
	.remove   = am7x_battery_remove,
	.suspend  = am7x_battery_suspend,
	.resume	  = am7x_battery_resume,
};

static int __init am7x_battery_init(void)
{
	int err;
	
#if (CONFIG_AM_CHIP_ID == 1213)
	return 0;
#endif

	err = platform_driver_register(&am7x_battery_driver);
	if(!err){
		am7x_battery_device = platform_device_register_simple(AM7X_BATTERY_DEVICENAME, 0,
								NULL, 0);

		if (!(am7x_battery_device)) {
			platform_driver_unregister(&am7x_battery_driver);
			err = -ENOMEM;
			goto exit;
		}
	}
	
exit:
	return err;
}

static void __exit am7x_battery_exit(void)
{
#if (CONFIG_AM_CHIP_ID == 1213)
	return 0;
#endif

	platform_driver_unregister(&am7x_battery_driver);
	if(am7x_battery_device)
		platform_device_unregister(am7x_battery_device);
}

module_init(am7x_battery_init);
module_exit(am7x_battery_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("am7x battery driver");
