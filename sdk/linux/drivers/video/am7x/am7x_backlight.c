/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_backlight.c

@abstract: actions-mircro backlight main source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco backlight Control Device driver;
 *  include pwm,pfm 	
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/delay.h>

#include "am7x_backlight.h"

#include "actions_io.h"
#include "actions_regs.h"
#include "am7x_gpio.h"


static int bl_enable = 0;

static void RegBitSet(int val,int reg,int msb,int lsb){    
	unsigned int mask = 0xFFFFFFFF;
	unsigned int old  = act_readl(reg);
	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);		       
}  

static int am7x_bl_disable(struct am7x_bl_data *devdata)
{
	if(devdata->bl_status == 0)
		return 0; //already disable

	if(devdata->gpio!=255)
		am_set_gpio(devdata->gpio,0);
		
	if(devdata->bl_ctl&PWM_CTL_EN)
		RegBitSet(0,BL_CTL,0,0);
	if(devdata->bl_ctl&PFM_CTL_EN)
		RegBitSet(0,BL_CTL,11,11);
	if(devdata->pwm0_ctl&PWM0_CTL_EN)
		RegBitSet(0,PWM0_CTL,7,7);
	if(devdata->pwm1_ctl&PWM1_CTL_EN)
		RegBitSet(0,PWM1_CTL,7,7);	


	if(devdata->cmu_fmclk&CMU_FMCLK_EN)
		RegBitSet(0,CMU_FM_PMUCLK,5,5);




	devdata->bl_status = 0; //set to disable
	
	return 0;
}

static int am7x_bl_enable(struct am7x_bl_data *devdata)
{
	if(devdata->bl_status == 1)
		return 0; //already enable	

	if(devdata->gpio!=255)
		am_set_gpio(devdata->gpio,1);
		
	if(devdata->cmu_fmclk&CMU_FMCLK_EN)
		RegBitSet(1,CMU_FM_PMUCLK,5,5);

	if(devdata->bl_ctl&PFM_CTL_EN){
		RegBitSet(1,BL_CTL,11,11);
		mdelay(100);
	}
	
	if(devdata->bl_ctl&PWM_CTL_EN){
		RegBitSet(1,BL_CTL,0,0);
	    mdelay(100);
	}

	if(devdata->pwm0_ctl&PWM0_CTL_EN)
		RegBitSet(1,PWM0_CTL,7,7);
	if(devdata->pwm1_ctl&PWM1_CTL_EN)
		RegBitSet(1,PWM1_CTL,7,7);


	devdata->bl_status = 1;	
	
	return 0;
}

static int am7x_bl_config(struct backlight_device* bl,int brightness)
{
	
	struct am7x_bl_data *devdata = dev_get_drvdata(&bl->dev);		
	int feekbackvol = bl->props.max_brightness - brightness;

	switch(devdata->adjust_format)
	{
		case PWM_FEEDBK_ADJUST:
			RegBitSet(feekbackvol,BL_CTL,6,2);
			break;
		case PFM_FEEDBK_ADJUST:
			RegBitSet(feekbackvol,BL_CTL,18,14);	
			break;
		case PWM0_DUTY_ADJUST:
			RegBitSet(brightness,PWM0_CTL,5,0);
			break;
		case PWM1_DUTY_ADJUST:
			RegBitSet(brightness,PWM1_CTL,5,0);
			break;
		case CMUFM_DUTY_ADJUST:
			RegBitSet(brightness/4,CMU_FM_PMUCLK,11,8);
			break;			
		default:
			break;
	}

	return 0;
}

static int am7x_bl_get_brightness(struct backlight_device* bl)
{
	return bl->props.brightness;
}

static int am7x_bl_update_status(struct backlight_device* bl)
{
	struct am7x_bl_data *devdata = dev_get_drvdata(&bl->dev);	

	int brightness = bl->props.brightness;

	if(brightness >  bl->props.max_brightness)
		brightness =  bl->props.max_brightness;	
	
//	if (bl->props.power != FB_BLANK_UNBLANK)
//		brightness = 0;

	if(brightness == 0)
		am7x_bl_disable(devdata);
	else{
		am7x_bl_config(bl,brightness);
		am7x_bl_enable(devdata);
	}

	printk("reg bl_ctl:%x\n",act_readl(BL_CTL));
	printk("reg cmu_fmclk:%x\n",act_readl(CMU_FM_PMUCLK));
	

	return 0;
}

static struct backlight_ops am7x_bl_ops = {
	.get_brightness = am7x_bl_get_brightness,
	.update_status	= am7x_bl_update_status,
};

static int am7x_bl_hardwareinit(struct am7x_bl_data *devdata)
{
	int ret = 0;
	unsigned int bl_ctl,cmu_fmclk,ana_amu_ctl;

	bl_ctl = devdata->bl_ctl&(~(PWM_CTL_EN));
	bl_ctl = bl_ctl&(~(PFM_CTL_EN));
	cmu_fmclk = devdata->cmu_fmclk&(~(CMU_FMCLK_EN))&(~(0x3));
	cmu_fmclk |=act_readl(CMU_FM_PMUCLK);
	ana_amu_ctl = devdata->ana_amu_ctl;
	
	act_writel(ana_amu_ctl,AMU_CTL);
	act_writel(bl_ctl,BL_CTL);
	act_writel(cmu_fmclk,CMU_FM_PMUCLK);
	act_writel(devdata->pwm0_ctl,PWM0_CTL);
	act_writel(devdata->pwm1_ctl,PWM1_CTL);

	if(devdata->cmu_fmclk&CMU_FMCLK_EN)
		act_writel(act_readl(CMU_FM_PMUCLK)|CMU_FMCLK_EN,CMU_FM_PMUCLK);	
	
	return ret;
}


static int read_config_file(void* buf)
{
	struct file *fp;
	mm_segment_t fs; 
	
	unsigned int buf_len;

	/** read config file **/	
	fs = get_fs(); 
	set_fs(KERNEL_DS); 
	fp = filp_open(BL_CONFIG_FILE, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
		printk("open %s error\n",BL_CONFIG_FILE); 
		return -1; 
	}
	vfs_llseek(fp,0,SEEK_END);
	buf_len = sizeof(struct am7x_bl_config);	

	vfs_llseek(fp,48,SEEK_SET);
	vfs_read(fp,(char*)buf,buf_len,&fp->f_pos);
	set_fs(fs);
	filp_close(fp,NULL);	



	return 0;
}


static int init_bl_data(struct am7x_bl_data** devdata)
{
	struct am7x_bl_config bl_config;
	int ret = 0;

	*devdata =kzalloc(sizeof(struct am7x_bl_data),GFP_KERNEL);
	if(*devdata==NULL)
	{
		ret = -ENOMEM;
		goto dev_data_fail;
	}
	
	if(read_config_file(&bl_config)!=0)
	{
		ret = -EIO;
		goto config_fail;
	}

	bl_enable = (*devdata)->bl_en = bl_config.bl_en;
	(*devdata)->switch_format = bl_config.switch_format;
	(*devdata)->adjust_format = bl_config.adjust_format;
	(*devdata)->bl_ctl = bl_config.bl_ctl;
	(*devdata)->ana_amu_ctl = bl_config.ana_amu_ctl;
	(*devdata)->cmu_fmclk = bl_config.cmu_fmclk;
	(*devdata)->pwm0_ctl = bl_config.pwm0_ctl;
	(*devdata)->pwm1_ctl = bl_config.pwm1_ctl;
	(*devdata)->gpio = bl_config.gpio;
	(*devdata)->bl_status = 0; //disable at first

	

config_fail:
	
dev_data_fail:
	return ret;
}

int check_boot_backlight(void)
{
	int ret = 0;
	if((act_readl(BL_CTL)&0x1)||act_readl(BL_CTL)&(1<<11) \
		||(act_readl(PWM0_CTL)&(1<<7))||(act_readl(PWM1_CTL)&(1<<7)))
		ret = 1;

	printk("boot_backlight:%d\n",ret);
	return ret ;
}

static int  am7x_backlight_remove(struct platform_device *pdev)
{
	printk("am7x: Backlight unloaded\n");
	return 0;
}

static int am7x_backlight_probe(struct platform_device *pdev)
{
	struct backlight_device *bl;
	struct am7x_bl_data *devdata = NULL;
	int retval,boot_backlight;
	char name[12];

	snprintf(name, sizeof(name), "am7xbl%d",1);
	retval = init_bl_data(&devdata);
	if(retval)
		goto fail;

	if(bl_enable == 0)
		return 0;	

		
	bl = backlight_device_register(name, &pdev->dev, devdata, &am7x_bl_ops);
	if (IS_ERR(bl)) {
		printk(KERN_WARNING "am7x: Backlight registration failed\n");
		retval = PTR_ERR(bl);	
		goto fail;
	}
	devdata->bl_dev = bl;

	boot_backlight = check_boot_backlight();

	#if CONFIG_AM_CHIP_ID==1213	  
	RegBitSet(0,BL_CTL,0,0);
	RegBitSet(0,BL_CTL,11,11);
	RegBitSet(1,GPIO_MFCTL1,31,30);
	RegBitSet(1,GPIO_MFCTL2,1,0);
	am7x_bl_hardwareinit(devdata);
	#endif


	if(boot_backlight == 0)
		am7x_bl_hardwareinit(devdata);
			
	bl->props.max_brightness = 32-1;//FB_BACKLIGHT_LEVELS - 1;
	bl->props.brightness = 0;
	bl->props.power = FB_BLANK_POWERDOWN;
	if(boot_backlight == 0)
		backlight_update_status(bl);
	platform_set_drvdata(pdev, bl);
	dev_set_drvdata(&bl->dev,devdata);

	return 0;
fail:
	
	am7x_backlight_remove(pdev);
	return retval;
}



#ifdef CONFIG_PM

static int am7x_backlight_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct backlight_device *bl;
	struct am7x_bl_data *devdata;
	
	if(bl_enable == 0)
		return 0;	

	bl = platform_get_drvdata(pdev);
	devdata = dev_get_drvdata(&bl->dev);

	am7x_bl_disable(devdata);

	return 0;
}

static int am7x_backlight_resume(struct platform_device* pdev)
{
	struct backlight_device *bl;
	
	if(bl_enable == 0)
		return 0;	

	bl = platform_get_drvdata(pdev);


	am7x_bl_update_status(bl);
	return 0;
}

#else
#define am7x_backlight_suspend	NULL
#define am7x_backlight_resume	NULL
#endif

static struct platform_driver am7x_backlight_driver = {
	.driver		= {
		.name	= AM7X_BL_DEVNAME,
		.owner	= THIS_MODULE,
	},
	.probe		= am7x_backlight_probe,
	.remove		= am7x_backlight_remove,
	.suspend	= am7x_backlight_suspend,
	.resume		= am7x_backlight_resume,
};


static struct platform_device *am7x_backlight_device;

static int  __init am7x_bl_init(void)
{
	int ret;
 	ret = platform_driver_register(&am7x_backlight_driver);
	if(!ret)
	{
		am7x_backlight_device = platform_device_register_simple(\
			AM7X_BL_DEVNAME,0,NULL,0);
		if (IS_ERR(am7x_backlight_device)) {
			platform_driver_unregister(&am7x_backlight_driver);
			ret = PTR_ERR(am7x_backlight_device);
		}	
	}
	return ret;
}

static void __exit am7x_bl_exit(void)
{
	platform_device_unregister(am7x_backlight_device);
	platform_driver_unregister(&am7x_backlight_driver);
}


module_init(am7x_bl_init);
module_exit(am7x_bl_exit);

MODULE_DESCRIPTION("Am7x  Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:Am7x-backlight");

