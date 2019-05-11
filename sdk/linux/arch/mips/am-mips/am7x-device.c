/**
 * arch/mips/am-mips/am7x-device.c
 *
 *this file provides some fuctions for device operation based on actions-micro chips 
 *
 *author: yekai
 *date:2010-09-09
 *version:0.1
 */
#include <linux/module.h>
#include <am7x_dev.h>
#include <am7x_board.h>

void am_disable_dev_clk(unsigned int  num,AM7X_HWREG reg)
{
	am_get_multi_res(DEVICE_RES);
	act_andl(~(1<<num),reg);
	am_release_multi_res(DEVICE_RES);
}

void am_enable_dev_clk(unsigned int num,AM7X_HWREG reg)
{
	am_get_multi_res(DEVICE_RES);
	act_orl(1<<num,reg);
	am_release_multi_res(DEVICE_RES);
}

void am_reset_dev(unsigned int num, AM7X_HWREG reg)
{
	am_get_multi_res(DEVICE_RES);
	act_andl(~(1<<num),reg);
	act_orl(1<<num,reg);
	am_release_multi_res(DEVICE_RES);
}

EXPORT_SYMBOL(am_disable_dev_clk);
EXPORT_SYMBOL(am_enable_dev_clk);
EXPORT_SYMBOL(am_reset_dev);

