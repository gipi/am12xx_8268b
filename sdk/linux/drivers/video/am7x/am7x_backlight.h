
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_backlight.h

@abstract: actions-mircro backlight head file.

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


#ifndef AM7X_BACKLIGHT_H
#define AM7X_BACKLIGHT_H

#define AM7X_BL_DEVNAME "am7x-backlight"
#define BL_CONFIG_FILE	"/am7x/case/data/backlight.bin"

/******** backlight adjust format ***************/
#define PWM0_DUTY_ADJUST			1
#define PWM1_DUTY_ADJUST			2
#define CMUFM_DUTY_ADJUST			3
#define PFM_FEEDBK_ADJUST			4
#define PWM_FEEDBK_ADJUST			5
#define NULL_ADJUST					6
/******** backlight switch format***************/
#define GPIO_SWICH					1
#define PFM_SWITCH					2
#define PWM_SWICTH					3


#define PWM_CTL_EN					1<<0					
#define PFM_CTL_EN					1<<11
#define CMU_FMCLK_EN				1<<5
#define PWM0_CTL_EN					1<<7
#define PWM1_CTL_EN					1<<7

struct am7x_bl_data{
	unsigned char bl_en;
	unsigned long bl_ctl;
	unsigned long ana_amu_ctl;
	unsigned long cmu_fmclk;	

	unsigned long pwm0_ctl;
	unsigned long pwm1_ctl;		
	unsigned char gpio;

	unsigned char switch_format;
	unsigned char adjust_format;


	int bl_status;
	struct backlight_device *bl_dev;
};

struct am7x_bl_config{
	unsigned char bl_en;
	unsigned char switch_format;
	unsigned char adjust_format;
	unsigned long bl_ctl;
	unsigned long ana_amu_ctl;
	unsigned long cmu_fmclk;	

	unsigned long pwm0_ctl;
	unsigned long pwm1_ctl;	
	unsigned char gpio;
}__attribute__((packed));


#endif
