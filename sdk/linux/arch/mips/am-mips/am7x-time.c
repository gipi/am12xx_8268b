/*
 * Pan Ruochen, reachpan@actions-micro.com
 * Copyright (C) 2010 Actions MicroEltronics, Inc.  All rights reserved.
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
 * Setting up the clock on the AM7XXX boards.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/delay.h>

#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/hardirq.h>
#include <asm/irq.h>
#include <asm/div64.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <asm/mips-boards/generic.h>
#include <asm/mips-boards/prom.h>

#include "actions_io.h"
#include "am7x-freq.h"
		
#define CFG_RTC_T0_VAL            0x26
#define CFG_RTC_T1_VAL            0x24
#define CFG_TIMER_PERIODIC        0x4

#define AM7X_TIMER_BITS           24
#define TIMER1_MAX_COUNT          0x00ffffff

#define  RATING                   200

#if 0
static void test_jiffies(void)
{
	int tick = 0, vv = 0;
	unsigned long start_jif = jiffies, cur_jif, next_jif;

	printk(KERN_DEBUG"rtc_t0=%x\n",act_readl(TIMER0_CTL));
	for(;;) {
		unsigned long tick = act_readl(TIMER0_VALUE);
		unsigned long status;

		while( tick > act_readl(TIMER0_VALUE) ) ;

		status = act_readl(TIMER0_CTL);
		BUG_ON( (status & 1) == 0 );
		act_writel(status, TIMER0_CTL);
		if(++vv % 1000 == 0)
			printk(KERN_DEBUG"okay...\n");
	}

	/* jiffies depends on HW timer irq */
	write_c0_status(read_c0_status()|1);
	for(tick=0; tick<10; ) {
		cur_jif  = jiffies;
		next_jif = start_jif + HZ;
		if( time_after(cur_jif, next_jif) ) {
			++tick;
			start_jif = jiffies;
			printk(KERN_DEBUG"%d seconds, jif=%lu\n", tick, cur_jif);
		} 
	}
	write_c0_status(read_c0_status()&~1);
}
#endif

/*
 *  Timer0 -- Used as the clock event device
 *  Timer1 -- Used as the clock source
 */

static unsigned long  hwclock_freq;

static void __init setup_am7x_timer(void)
{
	unsigned long timer_clk, latch;
#if CONFIG_AM_CHIP_ID==1203
	timer_clk  = get_pclk();
#elif CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	timer_clk = 24*_1M;
#endif
	latch = timer_clk / HZ;

	printk(KERN_DEBUG"APB CLOCK: %lu.%02lu\n", timer_clk/_1M, timer_clk%(_1M/100));
	printk(KERN_DEBUG"HZ=%d, latch=%lu\n", HZ, latch);

/* set up HW timer0 & timer1 */
	act_writel(latch, TIMER0_VALUE);
	act_writel(TIMER1_MAX_COUNT, TIMER1_VALUE);
	
	hwclock_freq = timer_clk;
}

static inline void start_timer(void)
{
	act_writel(CFG_RTC_T0_VAL, TIMER0_CTL);
	act_writel(CFG_RTC_T1_VAL, TIMER1_CTL);
//	test_jiffies();
}

static void am7x_set_mode(enum clock_event_mode mode,
                           struct clock_event_device *evt)
{
	AM7X_HWREG rtc_ctl = TIMER0_CTL;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		act_writel(CFG_RTC_T0_VAL, rtc_ctl);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		BUG_ON(0);
//		act_andl(~CFG_TIMER_PERIODIC, rtc_ctl);
		break;

	case CLOCK_EVT_MODE_SHUTDOWN:
		act_writel(0, rtc_ctl);
		break;

	case CLOCK_EVT_MODE_UNUSED:	/* shuddup gcc */
	case CLOCK_EVT_MODE_RESUME:
		BUG_ON(0);
	}
}

static int am7x_next_event(unsigned long delta, struct clock_event_device *cd)
{
    act_writel(delta, TIMER0_VALUE);
    return 0;
}

static void clear_timer0_irq_status(void)
{
	unsigned int val;
	val = act_readl(TIMER0_CTL);
	act_writel(val, TIMER0_CTL);
}

static struct clock_event_device am7x_clockevent = {
	.name		= "am7x",
	.features	= CLOCK_EVT_FEAT_PERIODIC,
	.rating		= RATING,
	.irq		= IRQ_TIMER0,
	.set_mode	= am7x_set_mode,
	.set_next_event = am7x_next_event,
};

static irqreturn_t am7x_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd = dev_id;
	clear_timer0_irq_status();
	cd->event_handler(cd);
//	static int count; if(++count == 2*HZ) {printk("2s\n"); count=0;}
	return IRQ_HANDLED;
}

static struct irqaction am7x_timer0_irqaction = {
	.handler = am7x_timer_interrupt,
	.flags   = IRQF_DISABLED,
	.mask    = CPU_MASK_CPU0,
	.name    = "timer0",
};

#if 0
static cycle_t am7x_clocksource_read(void)
{
	static unsigned long last_count;
	unsigned long cv, ret;
	int flags;
	cv   = act_readl(RTC_T0);
	ret  = latch_per_jiffy - cv;
	ret += cs_jiffies;
	local_irq_save(flags);
	if( act_readl(RTC_T0CTL) & 1 ) { /* counter reload ? */
		ret += latch_per_jiffy;
	} else
		if( (act_readl(INTC_MSK)&0x400) && 
			(unsigned long)(ret - last_count) > 3*latch_per_jiffy )
		{
			printk("ret=%lu, last_count=%lu", ret, last_count);
			panic("die...\n");
		}
	local_irq_restore(flags);
	last_count = ret;
	return ret;
}
#endif

static cycle_t am7x_clocksource_read(void)
{
	unsigned long ret = act_readl(TIMER1_VALUE);
	ret = TIMER1_MAX_COUNT - ret;
	return ret;
}

static struct clocksource clocksource_am7x = {
	.name	= "am7x-hrclock",
	.rating	= RATING,
	.read	= am7x_clocksource_read,
	.mask	= CLOCKSOURCE_MASK(AM7X_TIMER_BITS),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS
};

void __init plat_time_init(void)
{
	struct clocksource *cs = &clocksource_am7x;
	struct clock_event_device *cd = &am7x_clockevent;
	struct irqaction *action = &am7x_timer0_irqaction;
	unsigned int cpu = smp_processor_id();
	unsigned int clock = HZ;

	if (num_possible_cpus() > 1)
		return;
	
	setup_am7x_timer();

	/* register new clock source */
	clocksource_set_clock(cs, hwclock_freq);
/*---------
	printk("cs: mult=%u, shift=%u\n", cs->mult,cs->shift);
	printk("hwclock_freq = %lu\n", hwclock_freq);
//--------*/
	clocksource_register(cs);

	/* register new clock event device */
	cd->cpumask = cpumask_of_cpu(cpu);
	clockevent_set_clock(cd, clock);
	cd->max_delta_ns    = clockevent_delta2ns(clock, cd);
    cd->min_delta_ns    = clockevent_delta2ns(1, cd);
	clockevents_register_device(cd);
/*--------//
	printk("cd: mult=%u, shift=%u\n", cd->mult,cd->shift);
	printk("cs: %lu --> %lld\n", hwclock_freq, cyc2ns(cs,(cycle_t)hwclock_freq));
	printk("cd: %lu --> %lu\n",  clock, clockevent_delta2ns(clock,cd));
//--------*/

	action->dev_id = cd;
	setup_irq(IRQ_TIMER0, action);
	start_timer();
}


