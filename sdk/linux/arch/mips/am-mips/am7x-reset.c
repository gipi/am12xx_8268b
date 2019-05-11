/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * Reset the MIPS boards.
 *
 */
#include <linux/pm.h>
#include <linux/module.h>

#include <asm/io.h>
#include <asm/reboot.h>
#include <asm/mips-boards/generic.h>

#include <actions_regs.h>
#include <actions_io.h>
#include <am7x_board.h>
#include <am7x_gpio.h>

extern void am_setup_watchdog(__u8 wdtime);
extern void am_machine_halt(void);

static void mips_machine_restart(char *command)
{
	unsigned int __iomem *softres_reg =
		ioremap(SOFTRES_REG, sizeof(unsigned int));
	am_setup_watchdog(0);
	__raw_writel(GORESET, softres_reg);
}

static void mips_machine_halt(void)
{
	unsigned int __iomem *softres_reg =
		ioremap(SOFTRES_REG, sizeof(unsigned int));

	__raw_writel(GORESET, softres_reg);
	am_machine_halt();
}


void mips_reboot_setup(void)
{
	_machine_restart = mips_machine_restart;
	_machine_halt = mips_machine_halt;
	pm_power_off = mips_machine_halt;
}


void am_machine_halt(void)
{
	unsigned char sys_power = get_sys_info()->sys_gpio_cfg.sys_power;

	if(sys_power!=255)
		am_set_gpio(sys_power,1);
}

void am_setup_watchdog(__u8 wdtime)
{
	if(wdtime<0x8)
	{
		local_irq_disable();
		am7x_writel(0x61|(wdtime<<1),WD_CTL);
    		am7x_writel(0x30|(wdtime<<1),WD_CTL);

		while(1);
	}
	else			//disable wd
		am7x_writel(am7x_readl(WD_CTL)&(~0x10),WD_CTL);
}

void am_clear_watchdog(void)
{
	am7x_writel(am7x_readl(WD_CTL) | 0x1,WD_CTL);
}

EXPORT_SYMBOL(am_setup_watchdog);
EXPORT_SYMBOL(am_clear_watchdog);

