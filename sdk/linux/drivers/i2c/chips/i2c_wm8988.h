/*****************************************************************************
**
**            ActionsMicro PROPRIETARY INFORMATION
**
**  (c) Copyright, ActionsMicro  Incorporated, 2015
**      All Rights Reserved.
**
**  Property of ActionsMicro Incorporated. Restricted Rights -
**  Use, duplication, or disclosure is subject to restrictions set
**  forth in ActionsMicro's program license agreement and associated documentation.
******************************************************************************/
#ifndef I2C_WM8988_H
#define I2C_WM8988_H

/*ioctl cmd */
#define AM8253_INIT_WM8988I2C	(0x1)
#define WRITE_REG_WM			(0x2)
#define STE_WM_SMRATE			(0x3)
#define SET_WM_VOL_R			(0x4)
#define SET_WM_VOL_L			(0x05)

/*adc sample patameter    when dac sample rate changes in am8253  i2c write changed dac to wm8988*/
#define ACT_SAMPLE_96K		0x0
#define ACT_SAMPLE_48K		0x1
#define ACT_SAMPLE_44K		0x9 //31bit 1   1: Audio clock source from HOSC0: Audio clock source from Audio PLL
#define ACT_SAMPLE_32K		0x2
#define ACT_SAMPLE_24K		0x3
#define ACT_SAMPLE_22K		0xb //
#define ACT_SAMPLE_16K		0x4
#define ACT_SAMPLE_12K		0x5
#define ACT_SAMPLE_11K		0xd //
#define ACT_SAMPLE_8K		0x6



struct wm8988_data{
	unsigned char reg;
	unsigned char val;
};



#endif /* #ifndef __wm8988 */

