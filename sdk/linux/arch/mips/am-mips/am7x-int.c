/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 2001, 2004 MIPS Technologies, Inc.
 * Copyright (C) 2001 Ralf Baechle
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
 * Routines for generic manipulation of the interrupts found on the MIPS
 * Malta board.
 * The interrupt controller is located in the South Bridge a PIIX4 device
 * with two internal 82C95 interrupt controllers.
 */
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel_stat.h>
#include <linux/kernel.h>
#include <linux/random.h>

#include <asm/traps.h>
#include <asm/irq_cpu.h>
#include <asm/irq_regs.h>
#include <asm/mips-boards/malta.h>
#include <asm/mips-boards/maltaint.h>
#include <asm/mips-boards/piix4.h>
#include <asm/gt64120.h>
#include <asm/mips-boards/generic.h>
#include <asm/mips-boards/msc01_pci.h>
#include <asm/msc01_ic.h>
#include <asm/gic.h>
#include <asm/gcmpregs.h>

#include <actions_io.h>
#include <irq.h>

static DEFINE_SPINLOCK(am7x_lock);

void enable_am7x_irq(unsigned int irq)
{
	unsigned int mask = 0,reg;
	unsigned long flags;

	if(irq<32){
		reg = INTC_MSK;
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	}else{
		irq -= 32;
		reg = INTC_MSKH;
#endif
	}
	mask = 1 << (irq - AM7X_IRQ_START);
	spin_lock_irqsave(&am7x_lock, flags);
	mask |= act_readl(reg);
	act_writel(mask, reg);
	spin_unlock_irqrestore(&am7x_lock, flags);
}

void disable_am7x_irq(unsigned int irq)
{
	unsigned int mask = 0,reg;
	unsigned long flags;

	if(irq<32){
		reg = INTC_MSK;
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	}else{
		irq -= 32;
		reg = INTC_MSKH;
#endif
	}
	mask = ~(1 << (irq - AM7X_IRQ_START));
	spin_lock_irqsave(&am7x_lock, flags);
	mask &= act_readl(reg);
	act_writel(mask, reg);
	spin_unlock_irqrestore(&am7x_lock, flags);
}

static struct irq_chip am7x_irq_type = {
	.name = "AM7X",
	.ack = disable_am7x_irq ,
	.mask = disable_am7x_irq,
	.mask_ack = disable_am7x_irq,
	.unmask = enable_am7x_irq,
};

void dump_irq_desc(void)
{
	int i;
	extern struct irq_desc irq_desc[];
	for(i=0; i<32; i++)
		am_printf("irq_desc[%02d]: 0x%08x\n", i, irq_desc[i]);
	am_printf("\n");
}


static void __init am7x_irq_init(void)
{
	int i;
	unsigned long status_set;
	act_writel(0x000, INTC_CFG0);
	act_writel(0x000, INTC_CFG1);
	act_writel(0x400, INTC_CFG2);


/* Diable external interrupts */
	act_writel(0, INTC_EXTCTL01);
	act_writel(0, INTC_EXTCTL23);

	mips_cpu_irq_init();
#if 1
	for (i = AM7X_IRQ_START; i <= AM7X_IRQ_END; i++)
		set_irq_chip_and_handler(i, &am7x_irq_type, handle_level_irq);
#endif
//	status_set = ST0_IE | ST0_IM;
	status_set = STATUSF_IP2 | STATUSF_IP6; /* don't want count/compare interrupt on IP7 */
	change_c0_status(status_set, status_set); /* set interrupt masks */
}


static inline int clz(unsigned long x)
{
	__asm__(
	"	.set	push					\n"
	"	.set	mips32					\n"
	"	clz	%0, %1					\n"
	"	.set	pop					\n"
	: "=r" (x)
	: "r" (x));

	return x;
}


/*
 * IRQs on the Malta board look basically (barring software IRQs which we
 * don't use at all and all external interrupt sources are combined together
 * on hardware interrupt 0 (MIPS IRQ 2)) like:
 *
 *	MIPS IRQ	Source
 *      --------        ------
 *             0	Software (ignored)
 *             1        Software (ignored)
 *             2        Combined hardware interrupt (hw0)
 *             3        Hardware (ignored)
 *             4        Hardware (ignored)
 *             5        Hardware (ignored)
 *             6        Hardware (ignored)
 *             7        R4k timer (what we use)
 *
 * We handle the IRQ according to _our_ priority which is:
 *
 * Highest ----     R4k Timer
 * Lowest  ----     Combined hardware interrupt
 *
 * then we just return, if multiple IRQs are pending then we will just take
 * another exception, big deal.
 */
asmlinkage void plat_irq_dispatch(void)
{
	unsigned int intc_pendings;
	unsigned int irq;
	
	while((intc_pendings = act_readl(INTC_PD) & act_readl(INTC_MSK))!=0){
		irq = __ffs(intc_pendings);
		
		do_IRQ(irq);
	}

#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	while((intc_pendings = act_readl(INTC_PDH) & act_readl(INTC_MSKH))!=0){
		irq = __ffs(intc_pendings);
		
		do_IRQ(irq+32);
	}
#endif
}

void __init arch_init_irq(void)
{
	am7x_irq_init();
}

void malta_be_init(void)
{
}

int malta_be_handler(struct pt_regs *regs, int is_fixup)
{
	/* This duplicates the handling in do_be which seems wrong */
	int retval = is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL;
	return retval;
}
