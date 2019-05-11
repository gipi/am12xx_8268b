/*
 * at91_udc -- driver for at91-series USB peripheral controller
 *
 * Copyright (C) 2004 by Thomas Rathbone
 * Copyright (C) 2005 by HP Labs
 * Copyright (C) 2005 by David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>	/*for printk*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/slab.h> 	  /*for kmalloc*/
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>		
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <linux/spinlock.h>
#include <sys_msg.h>
#include "actions_regs.h"
#include "aotg_io.h"
#include "aotg_regs.h"
#include "am7x_uoc.h"
#include "am7x_platform_device.h"
#include <am7x_board.h>
#include <am7x_pm.h>
#include "gpio.h"
#include "sys_cfg.h"
#include "../../../include/linux/am7x_mconfig.h"

//#define HOST_TEST_EYEDGRAM

/*
*host device eye diagram test.
*steps:
*	1:use a u-disk and plug in to driver otg in TEST_PACKET mode
*	2.connect with the tester,and testing
*/

/*
 * This controller is simple and PIO-only.  It's used in many AT91-series
 * full speed USB controllers, including the at91rm9200 (arm920T, with MMU),
 * at91sam926x (arm926ejs, with MMU), and several no-mmu versions.
 *
 * This driver expects the board has been wired with two GPIOs suppporting
 * a VBUS sensing IRQ, and a D+ pullup.  (They may be omitted, but the
 * testing hasn't covered such cases.)
 *
 * The pullup is most important (so it's integrated on sam926x parts).  It
 * provides software control over whether the host enumerates the device.
 *
 * The VBUS sensing helps during enumeration, and allows both USB clocks
 * (and the transceiver) to stay gated off until they're necessary, saving
 * power.  During USB suspend, the 48 MHz clock is gated off in hardware;
 * it may also be gated off by software during some Linux sleep states.
 */
#ifndef __ACT_MODULE_CONFIG_H__ //if product is ezcast dongle,so should disable usb disconnect detect
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif
#ifdef MODULE_CONFIG_EZCAST_ENABLE
#define EZCAST_ENABLE MODULE_CONFIG_EZCAST_ENABLE
#else
#define EZCAST_ENABLE 0
#endif

#ifdef MODULE_CONFIG_FLASH_TYPE
#define FLASH_TYPE	MODULE_CONFIG_FLASH_TYPE
#else
#define FLASH_TYPE	0
#endif

#ifdef MODULE_CONFIG_EZWILAN_ENABLE
#define EZWIRE_TYPE	MODULE_CONFIG_EZWILAN_ENABLE
#else
#define EZWIRE_TYPE	0
#endif

#ifdef MODULE_CONFIG_EZMUSIC_ENABLE
#define EZMUSIC_ENABLE MODULE_CONFIG_EZMUSIC_ENABLE
#else
#define EZMUSIC_ENABLE 0
#endif
 #ifdef UOC_FILE_OPERATION
#include <linux/cdev.h>
//static void enable_plug_check(struct aotg_uoc* uoc,int enable);
static struct cdev uoc_dev;
#endif
#define	DRIVER_VERSION	"20/3/2010"
static const char driver_name[] = "am7x_uoc";
static struct  aotg_uoc  * the_controller ;

//static unsigned int detect_gpio = SYS_USB_DETECT_PIN;
static unsigned int detect_gpio=0;
/*
#if CONFIG_CHIP_ID ==1211
static unsigned int detect_gpio = 34;
#elif CONFIG_CHIP_ID == 1213
static unsigned int detect_gpio = 96;
#endif
*/
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
#ifdef MODULE_CONFIG_EZCASTPRO_MODE
#undef MODULE_CONFIG_EZCASTPRO_MODE
#endif
#endif
static int is_otg = 0;
static int default_role = 1;
static unsigned  int internal_resistor = 0;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
int error_count_wait_vfall = 0; //T932
// T166,William 20150814
static int usb_cur_inout_status = USB_CUR_STATUS_PLUG_OUT;
#endif
module_param(detect_gpio,uint ,S_IRUGO);
module_param(is_otg,bool,S_IRUGO);

module_param(default_role,bool,S_IRUGO);
module_param(internal_resistor,uint,S_IRUGO);


/*-------------------------------------------------------------------------*/
#define USB_RESET_TICKS		20   /*delay 10ms*/	
#ifdef CONFIG_AM_8251
#define EXTERN_EFUSE			0x5e  
#else
#define EXTERN_EFUSE			0x7f
#endif
#define	A_HOST_WAIT_TICKS	40

/*-------------------------------------------------------------------------*/
static void uoc_set_phy(u8 reg_addr, u8 value);
static int usb_reset(void);
static inline void port_event_inform(struct aotg_uoc * uoc,int portevent);
static void set_vbus_detect_threshold(unsigned int th);
static int aotg_id_force(struct aotg_uoc * uoc,int id);
static void enable_usb_irq(int enable);
static void clear_usb_irq(void);
static void config_wakeup_src(void);
static void enable_plug_check(struct aotg_uoc* uoc,int enable);
/*-------------------------------------------------------------------------*/

#ifdef COFIG_PROC_FOR_DEBUG_UOC
#include <linux/seq_file.h>
static const char debug_filename[] = "driver/uoc";
static int proc_uoc_show(struct seq_file *s, void *unused)
{
	struct aotg_uoc	*uoc = s->private;
	struct otg_transceiver *ptrans = &uoc->transceiver;
	
	seq_printf(s, "\n<am7x_uoc>@@--- %s: version %s\n",driver_name, DRIVER_VERSION);
	seq_printf(s, "\n<am7x_uoc>@@--- FSM: 0x%02x,OTGSTATUS:0x%02x USBEXTCTRL:0x%02x USBESTA:0x%02x\n", 
		aotg_readb(OTGSTATE) & 0x0f,
		aotg_readb(OTGSTATUS),
		aotg_readb(USBEXTCTRL),
		aotg_readb(USBESTA));
	
	seq_printf(s, "\n<am7x_uoc>@@---OTGIEN: 0x%02x: USBIEN:0x%02x,USBCS:0x%02x,OTGCTRL:0x%02x\n",
		aotg_readb(OTGIEN),
		aotg_readb(USBIEN),
		aotg_readb(USBCS),
		aotg_readb(OTGCTRL));
	
	seq_printf(s,"\n********************************\n");
	seq_printf(s, "\n<am7x_uoc>@@--- portstatus: 0x%x: portchange:0x%x\n",
		ptrans->port_status,
		ptrans->port_change);
	
	seq_printf(s,"\n********************************\n");
	return 0;
}

static int proc_uoc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_uoc_show, PDE(inode)->data);
}
static int proc_uoc_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)//(unsigned int cmd, unsigned int arg)
{
	printk("[%s %d] :%d\n",__FUNCTION__,__LINE__, cmd);
	switch (cmd) {
	case UOC_DISCONNECT:
		printk("[%s %d] :UOC_DISCONNECT\n",__FUNCTION__,__LINE__);
		act_writel((act_readl(0xb001009c)|(1<<3)), 0xb001009c);

		uoc_set_phy(0xf3,0x02);
		uoc_set_phy(0xf4,0x00);

		uoc_set_phy(0xf3,0x06);
		uoc_set_phy(0xf4,0x00);

		uoc_set_phy(0xe5,0x00);
		uoc_set_phy(0xe5,0x00);

		uoc_set_phy(0xf3,0x07);
		uoc_set_phy(0xf4,0x00);

		mdelay(100);
		uoc_set_phy(0xf4,0x01);
		uoc_set_phy(0xf3,0x17);
		uoc_set_phy(0xf3,0x37);

		uoc_set_phy(0xf5,0x01);
		uoc_set_phy(0xf6,0x00);
		break;
	case UOC_RECONNECT:
		uoc_set_phy(0xf3,0x00);
		uoc_set_phy(0xf4,0x00);
		uoc_set_phy(0xe5,0x00);
		act_writel(act_readl(CMU_CARDCLKSEL)&(~(1<<3)),CMU_CARDCLKSEL);
		uoc_set_phy(0xf5,0x00);
		uoc_set_phy(0xf6,0x00);
		break;	
	}
	return  0;

}


static const struct file_operations proc_ops = {
	.owner		= THIS_MODULE,
	.open		= proc_uoc_open,
	.ioctl 		= proc_uoc_ioctl,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void create_debug_file(struct aotg_uoc *uoc)
{
	uoc->pde = proc_create_data(debug_filename, 0, NULL, &proc_ops, uoc);
}

static void remove_debug_file(struct aotg_uoc *uoc)
{
	if (uoc->pde)
		remove_proc_entry(debug_filename, NULL);
}
#else
static  void create_debug_file(struct aotg_uoc *uoc) {}
static  void remove_debug_file(struct aotg_uoc *uoc) {}
#endif


/*----------------------------------------------------------------*/
static inline  int test_usb_clk(void)
{
	return ((aotg_readl(CMU_DEVCLKEN)  & (USBOTG_EN  | USBPHY_EN)) !=0 );
}

/**
*@brief open usb clocks
*   note: liucan,use follow method to judge whether clock is open
*	1	set phy reg 0x95 0
*	2	check VDSSTAT BIT4
*/
static  int  usb_clk_open(void)
{
	int cnt;
	//INFO("%s:%d\n",__FUNCTION__,__LINE__);
	aotg_writel(aotg_readl(CMU_DEVCLKEN) |USBOTG_EN |USBPHY_EN ,
			CMU_DEVCLKEN);

	for(cnt =0;cnt <10;cnt++){
		mdelay(5);
		uoc_set_phy(0x95,0);	
		if(aotg_readb(VDSTAT) & (1<<4)) 
			return 0;
	}
	
	return -EFAULT;
}


static  void  usb_clk_close(void)
{
	aotg_writel((aotg_readl(CMU_DEVCLKEN) & ~(USBOTG_EN |USBPHY_EN)),CMU_DEVCLKEN);
}


static inline void usb_cmu_reset(void)
{	
	aotg_writel((aotg_readl(CMU_DEVRST)&0xfffffdff),CMU_DEVRST);
	aotg_writel((aotg_readl(CMU_DEVRST) |0x200),CMU_DEVRST);
}

#ifdef UOC_FILE_OPERATION
static enum{
	DEFAULT_USBDET=-1,
	DISABLE_USBDET,
	ENABLE_USBDET
};
static int aotg_uoc_open(struct inode *inode, struct file *fd)
{
	int ret = 0;
	printk("[%s %d] open dev uoc-next\n",__FUNCTION__,__LINE__);
	return ret;
}

static int aotg_uoc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
//static int aotg_uoc_next_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	int ret=0;
	struct aotg_uoc	*uoc;
	uoc=the_controller;
	printk("[%s %d] usb detect  code:%d\n",__FUNCTION__,__LINE__, cmd);
	switch(cmd)
	{
		case DISABLE_USBDET:
			enable_plug_check(uoc,0);
			break;
		case ENABLE_USBDET:	
			enable_plug_check(uoc,1);
			break;
		default:
			printk(KERN_ERR"unknown command\n");			
			break;
	}
	return ret;
}

static int aotg_uoc_release(struct inode *inode, struct file *fd)
{
	int ret= 0;
		printk("[%s %d] close dev uoc-next\n",__FUNCTION__,__LINE__);
	return ret;
}

static struct file_operations uoc_operations = {
	.owner =	THIS_MODULE,
	.open 			=	aotg_uoc_open,
	.ioctl 	= 	aotg_uoc_ioctl,
	.release 			=	aotg_uoc_release
};
#endif
/**
*@brief set phy value to addr register
*
*@param[in] reg_addr: phy register addr , based on otg phy spec.
*@param[in] value	: phy value will be written to reg_addr
*   note: 	important!!write 3 times to make sure valid!
*/
static void uoc_set_phy(u8 reg_addr, u8 value)
{
	unsigned char addr_low,addr_high;
	unsigned char tmp;
	int i;

	addr_low =  reg_addr & 0x0f;
	addr_high =  (reg_addr >> 4) & 0x0f;    
	
	tmp = aotg_readb(VDCTRL);
	tmp &= 0x80;
	mdelay(1);

	aotg_writeb(value,VDSTAT);
	mdelay(1);
	
	for (i=0;i<3;i++){
		addr_low |= 0x10;    
		aotg_writeb(addr_low | tmp,VDCTRL); 
		mdelay(1);
		addr_low &= 0x0f;    
		aotg_writeb(addr_low | tmp,VDCTRL);
		mdelay(1);
		addr_low |= 0x10;    
		aotg_writeb(addr_low | tmp,VDCTRL);
		mdelay(1);
	}

	for (i=0;i<3;i++){
		addr_high |= 0x10;    
		aotg_writeb(addr_high | tmp,VDCTRL);
		mdelay(1);
		addr_high &= 0x0f;    
		aotg_writeb(addr_high | tmp,VDCTRL);         
		mdelay(1);
		addr_high |= 0x10;    
		aotg_writeb(addr_high | tmp,VDCTRL);
		mdelay(1);
	}
}

static int dev_pattern_check(struct aotg_uoc *_aotg)
{
	struct  aotg_uoc  	*uoc = _aotg;
	unsigned int val;
	int ret;
	unsigned int save_phy_ctl;
	unsigned char save_usbcs;

	save_usbcs = aotg_readb(USBCS);
	save_phy_ctl = aotg_readl(USB_PHY_CTL);
	/*discon DP,DM 500k ohm pull up resistor*/
	aotg_writel(aotg_readl(USB_PHY_CTL)&0xfffffff0,USB_PHY_CTL);

	/*disable 1.5k ohm Rpu*/
	uoc_set_phy(0xf3,0x20);
	uoc_set_phy(0xf4,0);

	/*pull down 15k ohm Rpd for DP,DM */
	aotg_writeb(aotg_readb(USBCS)|0x40,USBCS);

	/*check linestate0,linestate1 reset value*/
	val = aotg_readb(USBESTA)&0x30;
/*	if(val!=0)
	{
		printk("<am7x_uoc>@@---reset linestate0,1 error\n");
		ret = -1;
		goto exit;
	}
*/
	/*discon 15k ohm Rpd for DP,DM*/
	aotg_writel(aotg_readl(USB_PHY_CTL)|0x3,USB_PHY_CTL);

	/*pull up 500k ohm Rpu for DM for charge*/
	aotg_writel(aotg_readl(USB_PHY_CTL)|0x8,USB_PHY_CTL);
	mdelay(30);

	/*pull down 500k ohm to normal state*/
	aotg_writel(aotg_readl(USB_PHY_CTL)&0xfffffff7,USB_PHY_CTL);
	mdelay(5);

	/*check linestate0,linestate1 state*/
	val = (aotg_readb(USBESTA)&0x30)>>4;
	switch(val)
	{
		case 0x00:
			ret  = TYPE_PC_CONNECT;
			INFO("<am7x_uoc>@@---PC connect mode\n");
			break;
		case 0x02:
			ret  = TYPE_PC_NODPDM;
			INFO("<am7x_uoc>@@---power adapter mode\n");
			break;
		case 0x03:
			ret  = TYPE_ADP_CHARGE;
			INFO("<am7x_uoc>@@---battery charge mode\n");
			break;
		default:
			ret  = TYPE_NO_SUPPORT;
			//INFO("<am7x_uoc>@@---error line state\n");
			break;
	}
//exit:	
	mdelay(10);//bq add to test pc connect problem
	/*enable 1.5k ohm Rpu*/
	uoc_set_phy(0xf3,0);
	/*restore USB_PHY_CTL&USBCS*/
	aotg_writel(save_phy_ctl,USB_PHY_CTL);
	aotg_writeb(save_usbcs,USBCS);
	mdelay(5);
	uoc->btype = ret;
	return ret;
}
/*-------------------------------------------------------------------------*/
#ifdef DEBUG_UOC
static void dump_otg_regs(void)
{
	DMSG_UOC("<am7x_uoc>@@---otgirq=%02x,otgstate=%02x\r\notgctrl=%02x,otgstatus=%02x,otgien=%02x\n", \
			aotg_readb(OTGIRQ),aotg_readb(OTGSTATE),\
			aotg_readb(OTGCTRL), aotg_readb(OTGSTATUS),\
			aotg_readb(OTGIEN));
	DMSG_UOC("<am7x_uoc>@@---USBEIRQ=%02x USBESTA=%02x\r\nIVECT=%02x USBIEN=%02x\n",\
			aotg_readb(USBEIRQ),aotg_readb(USBESTA), \
			aotg_readb(IVECT),aotg_readb(USBIEN));
	//DMSG_UOC("INTC_MSK=%x\n",aotg_readl(INTC_MSK));
}

static const char *state_string(enum usb_otg_state state)
{
	switch (state) {
		case OTG_STATE_A_IDLE:		return "a_idle";
		case OTG_STATE_A_WAIT_VRISE:	return "a_wait_vrise";
		case OTG_STATE_A_WAIT_BCON:	return "a_wait_bcon";
		case OTG_STATE_A_HOST:		return "a_host";
		case OTG_STATE_A_SUSPEND:	return "a_suspend";
		case OTG_STATE_A_PERIPHERAL:	return "a_peripheral";
		case OTG_STATE_A_WAIT_VFALL:	return "a_wait_vfall";
		case OTG_STATE_A_VBUS_ERR:	return "a_vbus_err";
		case OTG_STATE_B_IDLE:		return "b_idle";
		case OTG_STATE_B_SRP_INIT1:	return "b_srp_init";
		case OTG_STATE_B_PERIPHERAL:	return "b_peripheral";
		case OTG_STATE_B_WAIT_ACON:	return "b_wait_acon";
		case OTG_STATE_B_HOST:		return "b_host";
		default:			return "UNDEFINED";
	}
}

static void check_state(struct aotg_uoc *uoc, const char *tag)
{
	enum usb_otg_state	state;
	unsigned char			fsm = aotg_readb(OTGSTATE)&0x0f ;
	switch (fsm) {
	/* a_state*/
	case 0x0:
		state = OTG_STATE_A_IDLE;
		break;
	case 0x01:
		state = OTG_STATE_A_WAIT_VRISE;		
		break;
	case 0X02:
		state = OTG_STATE_A_WAIT_BCON;
		break;
	case 0X03:
		state = OTG_STATE_A_HOST;
		break;
	case 0x04:
		state = OTG_STATE_A_SUSPEND;
		break;
	case 0x05:
		state = OTG_STATE_A_PERIPHERAL;
		break;
	case 0x06:
		state = OTG_STATE_A_VBUS_ERR;
		break;	
	case 0x07:
		state = OTG_STATE_A_WAIT_VFALL;
		break;
	/*b states */
	case 0x08:
		state = OTG_STATE_B_IDLE;
		break;
	case 0x09:
		state = OTG_STATE_B_PERIPHERAL;
		break;
	case 0x0a:
		state = OTG_STATE_B_WAIT_ACON;
		break;
	case 0x0b:
		state = OTG_STATE_B_HOST;
		break;
	default:
		state = OTG_STATE_UNDEFINED;
		break;
	}

	DMSG_UOC("<am7x_uoc>@@---otg: %s HARDFSM %s/%02x,transc_state %s ,orignal=%s\n",\
		tag,
		state_string(state),
		fsm,
		state_string(uoc->transceiver.state),
		state_string(uoc->prevstate));
	
}
#else
static void dump_otg_regs(void){};
static inline const char *state_string(enum usb_otg_state state){return NULL;};
static void check_state(struct aotg_uoc *uoc, const char *tag){};
#endif

/*--------------------------------------------------------*/
static void update_state(struct aotg_uoc * _aotg) 
{	
	unsigned char state,id;
	struct  aotg_uoc  	*uoc = _aotg;
	
	state = get_otg_state();
	id = get_otg_id();
	if(id) 
		set_b_fsm_defaults(uoc);
	else
		set_a_fsm_defaults(uoc);

	/*update the state and id status*/
	uoc->prevstate = uoc->transceiver.state;
	uoc->transceiver.state = state;
	uoc->id = id;
#if 1	
	switch(state) {
		case OTG_STATE_B_IDLE:
		case OTG_STATE_B_PERIPHERAL:
			if(!uoc->transceiver.gadget){
				DMSG_UOC("<am7x_uoc>@@---<RUN_FSM>update state ,pull down to hide\n");
				dplus_down();
			}
			break;		
		default:
			DMSG_UOC("<am7x_uoc>@@--- <RUN_FSM> OTG has already run,state is %02x,do nothing\n",state);
			break;
	}
#endif
	DMSG_UOC("<am7x_uoc>@@---OTGIRQ %02x,id %d, OTGSTATE %02x\n",aotg_readb(OTGIRQ),id,state);
}


static void auto_power_on(struct aotg_uoc *uoc)
{
	DMSG_UOC("<am7x_uoc>@@--- <POWER> power on\n");
	uoc->prevstate = uoc->transceiver.state; 	/*change state manually*/
	uoc->transceiver.state = OTG_STATE_A_WAIT_BCON;
	aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL); /*clear a_bus_drop*/
	aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL); /*a_bus_req*/	
}


static void aotg_a_idle(struct aotg_uoc * uoc)
{
	uoc->prevstate = uoc->transceiver.state;
	uoc->transceiver.state = get_otg_state();
	
	check_state(uoc,__FUNCTION__);
	switch(uoc->prevstate) {
		case OTG_STATE_A_WAIT_BCON:
			if(is_a_bus_drop) {
				DMSG_UOC("<am7x_uoc>@@---<A_IDLE>state transfer, caused by a_bus_drop\n");
			}
		//	auto_power_on(uoc);
			break;
		case OTG_STATE_A_SUSPEND:
			if(uoc->transceiver.host){
				DMSG_UOC("<am7x_uoc>@@---<SUPEND>notify for a suspend\n");
				port_event_inform(uoc,AOTG_VBUS_DROP_EVENT);
			}else 
				DMSG_UOC("<am7x_uoc>@@---<A_IDLE>no notify handler\n");
			if(is_a_bus_drop) {
				auto_power_on(uoc);
				DMSG_UOC("<am7x_uoc>@@---<A_IDLE>state transfer, caused by a_bus_drop\n");	
			}
			break;
		case OTG_STATE_A_VBUS_ERR:
			if(is_a_bus_drop) 
				DMSG_UOC("<am7x_uoc>@@---<A_IDLE>state transfer, caused by a_bus_drop\n");
			auto_power_on(uoc);
			break;
		default:
			break;
	}	
}

static void aotg_a_host(struct aotg_uoc *uoc)
{
	uoc->dual_role = AOTG_DUAL_ROLE_HOST;
	uoc->prevstate = uoc->transceiver.state;
	uoc->transceiver.state = get_otg_state();	
	check_state(uoc,__FUNCTION__);	
	if ( uoc->prevstate == uoc->transceiver.state) {
		dump_otg_regs();
		return;
	}
	
	switch(uoc->prevstate) {
		case OTG_STATE_A_WAIT_BCON:
			if(is_b_conn) {
				if(uoc->transceiver.host){
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>plug in\n");
					port_event_inform(uoc,AOTG_PLUGIN_EVENT);
				}
				else{
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>no notify handler\n");
				}
		    #ifdef CONFIG_USB_AM7X_HOST_ONLY_0
			    uoc->transceiver.state = OTG_STATE_A_HOST;
			#endif
			}
			else 
				DMSG_UOC("<am7x_uoc>@@---<A_HOST> odd b_conn status\n");
			break;
			
		case OTG_STATE_A_HOST:
			if(!is_a_bus_req){
				uoc->prevstate = uoc->transceiver.state;
				uoc->transceiver.state = OTG_STATE_A_SUSPEND;
				DMSG_UOC("<am7x_uoc>@@---<A_HOST> enter a_suspend state\n");
				break;
			}
			
			uoc->transceiver.state = OTG_STATE_A_WAIT_BCON;/*manual change*/
		#ifndef CONFIG_USB_AM7X_HOST_ONLY_0
			if(!is_b_conn) 
		#endif
			{
				DMSG_UOC("<am7x_uoc>@@---<A_HOST>leave A-HOST state\n");
				if(uoc->transceiver.host){
					port_event_inform(uoc,AOTG_PLUGOUT_EVENT);
				}else
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>no notify handler\n");
			}
			break;
			
		case OTG_STATE_A_SUSPEND:
			if(!is_b_conn && !is_a_set_b_hnp_en) {
				uoc->transceiver.state = OTG_STATE_A_WAIT_BCON;/*manual change*/
				if(uoc->transceiver.host){
					port_event_inform(uoc,AOTG_PLUGOUT_EVENT);
				}
				else 
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>no notify handler\n");
			}else {
				if(uoc->transceiver.host){
					port_event_inform(uoc,AOTG_RESUME_EVENT);
				}
				else 
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>no notify handler\n");
			}
			break;
			
		default:
			break;
	}
#ifndef CONFIG_USB_AM7X_HOST_ONLY_0
	update_state(uoc);
#endif

}



static void aotg_b_idle(struct aotg_uoc * aotg)
{
#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    printk("uoc: should not call %s under host only mode\n",__FUNCTION__);
    return;
#endif

	check_state(aotg,__FUNCTION__);
	
	set_b_fsm_defaults(aotg);
	aotg->prevstate = aotg->transceiver.state;
	aotg->transceiver.state = get_otg_state();
#if 1 	
	DMSG_UOC("<am7x_uoc>@@---<B_IDLE> state transfer from %02x ------>%02x\n",
			aotg->prevstate,aotg->transceiver.state);
	DMSG_UOC("<am7x_uoc>@@--- %s:prevstate%s\n",__FUNCTION__,state_string(aotg->prevstate));
#endif

	switch(aotg->prevstate) {
		case OTG_STATE_B_IDLE:
			if(aotg->transceiver.state == OTG_STATE_B_PERIPHERAL )
				enable_udc_interrupt();
			break;		
		case OTG_STATE_B_PERIPHERAL:
			disable_udc_interrupt();
			if(aotg->transceiver.gadget){
				port_event_inform(aotg,AOTG_SOFT_DISCONN_EVENT);
			}
			else{ 	
				DMSG_UOC("<am7x_uoc>@@---<B_IDLE>no notify handler\n");
			}
			break;
	#if 0		
		case OTG_STATE_A_IDLE:
		case OTG_STATE_A_HOST:
		case OTG_STATE_A_WAIT_BCON:
			if(aotg->transceiver.gadget){
				DMSG_UOC("#<am7x_uoc>#:<B_IDLE>ID SWITCH\n");	
				enable_udc_interrupt();
				//otg_peripheral_notify(AOTG_SOFT_DISCONN_EVENT);
			}
	#endif		
		default:
			DMSG_UOC("<am7x_uoc>@@---<B_IDLE>odd state\n");
			break;
	}
	update_state(aotg);
}


static void aotg_id_switch(struct aotg_uoc * aotg,int newid)
{
#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    printk("uoc: should not call %s under host only mode\n",__FUNCTION__);
    return;
#endif

	if(aotg->transceiver.default_a) {
		/*A switch to B*/
		/*make BUSREQ invalid to avoid B device doing  nonsense SRP */	
		DMSG_UOC("<am7x_uoc>@@---<ID_SWITCH>change to B device\n");
		switch(aotg->prevstate){
			case OTG_STATE_A_HOST:
				aotg->transceiver.state = OTG_STATE_A_WAIT_BCON;
				if(!is_b_conn) {
					DMSG_UOC("<am7x_uoc>@@---<A_HOST>leave A-HOST state\n");
					if(aotg->transceiver.host){
						port_event_inform(aotg,AOTG_PLUGOUT_EVENT);
					}else
						DMSG_UOC("<am7x_uoc>@@--- <A_HOST>no notify handler\n");
				}
			break;
			default:
				DMSG_UOC("<am7x_uoc>@@---<A_HOST>middle state not deal\n");
			break;
		}
		aotg_clearbits(OTGCTRL_BUSREQ,OTGCTRL);	
		mdelay(10);
	}else{
		/*B switch to A*/
		switch(aotg->transceiver.state) {
			case OTG_STATE_B_IDLE:
				DMSG_UOC("<am7x_uoc>@@---<ID_SWITCH>change to A device\n");
				aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL);
				break;
#if 0 				
			case OTG_STATE_B_DISCHRG1:
			case OTG_STATE_B_SRP_INIT1:
				DMSG_UOC("#<am7x_uoc>#: <ID_SWITCH> quit SRP,and change to a device\n");
				/*TODO: notify SRP stopping to userspace*/
				break;
#endif				
			case OTG_STATE_B_PERIPHERAL:
				DMSG_UOC("<am7x_uoc>@@---<ID_SWITCH>stop b_peripheral activity, and change to a device\n");
				if(aotg->transceiver.gadget){
					//otg_peripheral_notify(AOTG_ID_CHANGE_EVENT);
				}else {
					aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL);
					DMSG_UOC("<am7x_uoc>@@--- <ID_SWITCH>no notify handler\n");
				}
				break;
				
			default:
				DMSG_UOC("<am7x_uoc>@@---<ID_SWITCH>odd state %02x\n",aotg->transceiver.state);
				break;
		}
	} 

	update_state(aotg);
}


static void aotg_b_peripheral(struct aotg_uoc * aotg)
{

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    printk("uoc: should not call %s under host only mode\n",__FUNCTION__);
    return;
#endif
	aotg->dual_role = AOTG_DUAL_ROLE_PERIPHERAL;
	aotg->prevstate = aotg->transceiver.state;
	aotg->transceiver.state = get_otg_state();

#if 1
	check_state(aotg,__FUNCTION__);
	DMSG_OTG("<am7x_uoc>@@---%s:%s\n",__FUNCTION__,state_string(aotg->prevstate));
#endif
	switch(aotg->prevstate) {
		case OTG_STATE_B_IDLE:
			enable_udc_interrupt();
			break;		
		case OTG_STATE_B_PERIPHERAL:
			DMSG_UOC("<am7x_uoc>@@---<B_PERIPHERAL> already in b_peri state,do nothing\n");
			break;
#if 0 
			if(aotg->transceiver.gadget){
				//port_event_inform(AOTG_SOFT_DISCONN_EVENT);
			}
			else 
				DMSG_UOC("#<am7x_uoc>#:<B_PERIPHERAL>no notify handler\n");
			break;
#endif		

#if 0			
		case OTG_STATE_A_IDLE:
		case OTG_STATE_A_HOST:
		case OTG_STATE_A_WAIT_BCON:
			DMSG_UOC("#<am7x_uoc>####***<B_PERIPHERAL>id switch!!!\n");
			if(aotg->transceiver.gadget)	
				enable_udc_interrupt();
			break;	
#endif		

		default:
			DMSG_UOC("<am7x_uoc>@@---<B_PERIPHERAL> odd state\n");
			break;
	}
	update_state(aotg);
}


void aotg_a_vbus_err(struct aotg_uoc * aotg)
{
	aotg->prevstate = aotg->transceiver.state;
	aotg->transceiver.state = get_otg_state();
	
	check_state(aotg,__FUNCTION__);
	
	/*set a_bus_drop to make OTG state*/
	aotg_setbits(OTGCTRL_ABUSDROP,OTGCTRL); 
	mdelay(10);

	switch(aotg->prevstate) {
		case OTG_STATE_A_HOST:
		case OTG_STATE_A_SUSPEND:
			if(aotg->transceiver.host){
				port_event_inform(aotg,AOTG_VBUS_DROP_EVENT);
			}
			else 
				DMSG_UOC("<am7x_uoc>@@---<A_BUS_ERR>no notify handler\n");
			break;
		default:	
			break;
	}										 	
}

/*-------------------------------------------------------------------------*/
/*this functions can be call from interrupts*/
static inline void port_event_inform(struct aotg_uoc * uoc,int portevent)
{
	struct otg_transceiver *transceiver = aotg_to_transceiver(uoc);
	struct port_handler* phandler = (struct port_handler *)transceiver->port_handler;
	if(!phandler){
		DMSG_OTG("<am7x_uoc>@@---not event inform handler\n");	
		return;
	}
	DMSG_OTG("<am7x_uoc>@@---%s:portevent=0x%x\n",__FUNCTION__,portevent);
	phandler->function((void*)phandler->data,portevent);
}


int otg_set_porthandler(struct otg_transceiver *transceiver,struct port_handler *process)
{
	//struct aotg_uoc *uoc = transceiver_to_aotg(transceiver);
	if(!process && !transceiver->port_handler)
		return -ESHUTDOWN;
	else if(process && transceiver->port_handler)
		return -EBUSY;
	
	transceiver->port_handler = process;
	DMSG_UOC("<am7x_uoc>@@---Set port handler sucess!\n");
	return 0;
}
EXPORT_SYMBOL(otg_set_porthandler);

static irqreturn_t aotg_uoc_irq (int irq,void *_uoc)
{
	u8 irqvector;
	u8 otgint;
	u8 id;
	struct aotg_uoc 	*uoc = _uoc;

	/*clear USB irq*/
	aotg_setbits(USBEIRQ_USBIRQ,USBEIRQ);
	irqvector = aotg_readb(IVECT);
	if(irqvector == UIV_OTGIRQ){	
		/*clear irq pend*/
		id = get_otg_id();
		otgint= aotg_readb(OTGIEN) & aotg_readb(OTGIRQ);
		aotg_setbits(otgint,OTGIRQ);	
		
		DMSG_UOC("<am7x_uoc>@@---<OTG_IRQ> int %02x,id %d\n",otgint,id);
		switch(otgint) {
			case AOTG_IRQ_IDLE_AND_PERI:
			case AOTG_IRQ_IDLE:
			case AOTG_IRQ_IDLE_AND_HOST:
			case AOTG_IRQ_HOST_AND_IDLE_AND_PERI:	
				if( id == uoc->id){ 	
					/*none id switch*/  			
					if(uoc->id) 
						aotg_b_idle(uoc);
					else
						aotg_a_idle(uoc);								
				}else{
					/*id switch*/
					DMSG_UOC("<am7x_uoc>@@---id changed,id= %d,orig=%d\n",id,uoc->id);
					aotg_id_switch(uoc,id);
				}
				break;
			case AOTG_IRQ_PERIPH:
				printk("<am7x_uoc>@@---aotg periph irq\n");
				aotg_b_peripheral(uoc);
				break;
			case AOTG_IRQ_HOST:
        /*
         * Do not do the jump for host mode only.
         *
         * Modify by Simon Lee.
         */
	    #ifndef CONFIG_USB_AM7X_HOST_ONLY_0 //bluse:add for disconnect problem
				if((get_otg_state()==OTG_STATE_A_WAIT_BCON)){
					uoc->OtgJump++;
					if(uoc->OtgJump > 1){
						printk("uoc disable dconn\n");
						uoc_set_phy(0xe7,0x0f);
						uoc->OtgFlage=1;
						break;
					}
				}else if((uoc->OtgFlage==1)  &&(get_otg_state()==OTG_STATE_A_HOST) ){
					printk("uoc enable dcon\n");
					uoc->OtgFlage=0;
					uoc->OtgJump=0;
					uoc_set_phy(0xe7,0x1f);
				}	
			#endif
				printk("uoc  host irq\n");
				aotg_a_host(uoc);
				break;
			case AOTG_IRQ_HOST_AND_VERR:
			case AOTG_IRQ_VBUSERR:
				aotg_a_vbus_err(uoc);
				break;	
			default:
				break;
		}
	}else
		return IRQ_NONE;
	
	return IRQ_HANDLED;
}

/*resolve non standard adapater recognition for usb device problem*/
static int _NonStandardAdapter(struct aotg_uoc *uoc)
{
	int result=0,i;
	unsigned long flags;
	usb_reset();
	printk("otg_states:%02x\n",get_otg_state());
	spin_lock_irqsave(&uoc->lock,flags);
	aotg_id_force(uoc,ID_ROLE_A);
	spin_unlock_irqrestore(&uoc->lock, flags);
	mdelay(5);
	aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL);
	aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL);
	///udelay(5);
	for(i=0;i<25;i++){
		/*wait fsm change to be a host,check several times*/
		mdelay(50);
		if(get_otg_state()==OTG_STATE_A_HOST){
			result = 1;
			break;
		}
	}			
	return result;
}
/*-------------------------------------------------------------------------*/
static int dev_plugin_ahost(struct aotg_uoc * uoc)
{
	u8 estatus,is_bidle;
	int result = 0,line_pm=-1;
	
	estatus= aotg_readb(USBESTA) ;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
	is_bidle = ((get_otg_state() == OTG_STATE_B_IDLE) || (get_otg_state() == OTG_STATE_A_IDLE));// T932
#else
	is_bidle = (get_otg_state() == OTG_STATE_B_IDLE) ;
#endif
	if(/*!(estatus&(1<<6))&&*/((estatus & AOTG_DEVIN_MASK) != 0) &&  is_bidle){	
#if EZCAST_ENABLE
		line_pm=dev_pattern_check(uoc);
		printk("line_pm:%d\n",line_pm);
		if(line_pm==TYPE_ADP_CHARGE){
			result=_NonStandardAdapter(uoc);
			printk("result:%d state:%02x\n",result,get_otg_state());
		}else
				result = 1;
#else
			result = 1;
#endif
	}
	return result;
}

static void enable_plug_check(struct aotg_uoc* uoc,int enable)
{
    
#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    /*
     * For host only mode, do not detect.
     * Modify by Simon Lee.
     */
    uoc->enable_check = 0;
#else
	int is_enable = !!enable;
	uoc->enable_check = is_enable;
	if(is_enable)
		mod_timer(&uoc->timer,jiffies + 100);	
#endif

    return;
}

static void config_drv_vbus(void)
{
	u32 val;
	val = aotg_readl(0xB01C004C);
	val  =val & (~(3<<12));
	val  = val  | (3<<12);
	aotg_writel(val,0xB01C004C);
	DMSG_UOC("<am7x_uoc>@@---mfp3:%x\n",aotg_readl(0xB01C004C));
}


static void init_detect_gpio(int pin)
{
	/*here only config gpio 28*/
	u8 group,index;
	unsigned long regaddr;
	//INFO("<am7x_uoc>@@---detect gpio pin:%d\n",pin);
	
	group = pin / 32;
	index = pin % 32;
	
	regaddr = GPIO_31_0OUTEN + 12*group;
	aotg_writel(aotg_readl(regaddr) &~(1UL<<index), regaddr); //disable out
	regaddr += 4;
	aotg_writel(aotg_readl(regaddr) |(1UL<<index), regaddr); //enable in
}



static int get_detect_gpioval(int pin)
{
	u8 group = pin /32;
	u8 index  = pin % 32;
	return   (aotg_readl(GPIO_31_0DAT + group*12)>>index) & 0x01;		
}


static int is_dev_plugin(struct aotg_uoc *uoc)
{
	int host_in = 0;
	int is_otg  = uoc->is_otg;
	int i;

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    printk("uoc: should not call %s under host only mode\n",__FUNCTION__);
    return 1;
#endif
	if(is_otg){
		/*
		 * FIX_ME: user may plugout udisk ,but minAB
		 * is still plug ,then the id is not default_role ,but A
		 */
		if((get_otg_id() != default_role))
		{
			if((get_otg_state()==OTG_STATE_A_IDLE)||(get_otg_state()==OTG_STATE_B_IDLE)){
				/*force fsm to A_WAIT_BCON state,thus the ctrl can detect dp dm plug in or not*/
				aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL);
				aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL);
				for(i=0;i<A_HOST_WAIT_TICKS;i++){
					/*wait fsm change to be a host,check several times*/
					mdelay(50);
					if(get_otg_state()==OTG_STATE_A_HOST){
						host_in = 1;
						break;
					}
				}
				
			}
			else if(get_otg_state()==OTG_STATE_A_HOST)
				host_in = 1;
		}			
	}else
		host_in = dev_plugin_ahost(uoc);
	return host_in;
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
// T166,William 20150814: get current USB plug in/out status
int get_cur_usb_inout_status(void)
{
	return usb_cur_inout_status;
}
EXPORT_SYMBOL(get_cur_usb_inout_status);

// T166,William 20150814: set current USB plug in/out status
void set_cur_usb_inout_status(int status)
{
	usb_cur_inout_status = status;
}
EXPORT_SYMBOL(set_cur_usb_inout_status);
#endif

/*must realize usb hotplug to enable module install*/
static void plug_detect_handler(unsigned long param)
{
	int vbus_state;
	int is_otg;
	int is_clkopen;
	unsigned long flags;
	int detectpin,enable_check;
	int dtype;
	struct aotg_uoc * uoc = (struct aotg_uoc*)param;
	struct am_sys_msg  msg;

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    return;
#endif

	spin_lock_irqsave(&uoc->lock,flags);
	enable_check = uoc->enable_check;
	spin_unlock_irqrestore(&uoc->lock,flags);
	
	if(!enable_check){
		goto timer_mod;
	}

	is_clkopen = test_usb_clk();
	if(!is_clkopen ){
		if(usb_clk_open() !=0 )
			ERR("<am7x_uoc>@@---clk revalid open failed \n");
	}

	//check aplug
	if(is_dev_plugin(uoc)){
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
		// William 20150814: send "plug in" only when current status is "plug out"
		INFO("<am7x_uoc>@@---<HOST> Current status: %d\n", usb_cur_inout_status);
		if (usb_cur_inout_status == USB_CUR_STATUS_PLUG_OUT)
		{
			INFO("<am7x_uoc><HOST>plug in\n");
			msg.type = SYSMSG_USB;
			msg.subtype = HOST_USB_IN;
			msg.dataload = HOST_RAW;
			msg.reserved[0] =USB_GROUP0;
			am_put_sysmsg(msg);
			enable_plug_check(uoc,0);
			uoc->vbus_gpio.val=0; //bluse add for 1213 usb0 switch
			usb_cur_inout_status = USB_CUR_STATUS_PLUG_IN;
		}
		else
		{
			INFO("<am7x_uoc>@@---<HOST>Ignore plug in event because status is plug in now\n");
		}
#else
                INFO("<am7x_uoc><HOST>plug in\n");
                msg.type = SYSMSG_USB;
                msg.subtype = HOST_USB_IN;
                msg.dataload = HOST_RAW;
                msg.reserved[0] =USB_GROUP0;
                am_put_sysmsg(msg);
                enable_plug_check(uoc,0);
                uoc->vbus_gpio.val=0; //bluse add for 1213 usb0 switch
#endif
		goto exit;
	}
	/*else if(get_otg_id()!=default_role)
		goto exit;
	*/
	detectpin = uoc->vbus_gpio.detectpin;
	is_otg = uoc->is_otg;
	dtype = uoc->btype;
	/*check bplug*/
	if(is_otg) {
		vbus_state = aotg_readb(USBESTA) & (1<<6) ? 1 : 0 ;
	}else
		vbus_state = get_detect_gpioval(detectpin);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
	// T932
	if((aotg_readb(OTGSTATE) & OTG_STATE_A_WAIT_VFALL) == OTG_STATE_A_WAIT_VFALL){
		error_count_wait_vfall++;
		if(error_count_wait_vfall==5){
			DMSG_UOC("%s:%d:ERROR in a_wait_vfall state.\n",__func__,__LINE__);
			DMSG_UOC("but state is not changed over 1.1s.\n ");
			DMSG_UOC("Va_sess_vld should fail.\n");
			DMSG_UOC("Reset USB registers then state goes to idle automatically.\n\n");
			error_count_wait_vfall = 0;
			usb_reset();
		}
	}

	if((vbus_state != uoc->vbus_gpio.val)
		|| (((aotg_readb(USBESTA) & AOTG_DEVIN_MASK) == 0) // T932
		&& ((aotg_readb(OTGSTATE) & OTG_STATE_A_PERIPHERAL) == 0) // T932
		&& ((aotg_readb(OTGSTATE) & OTG_STATE_B_PERIPHERAL) == 0)) // T932
		){
#else
	if(vbus_state != uoc->vbus_gpio.val){
#endif
		uoc->vbus_gpio.val = vbus_state;	
		
		if(!vbus_state){ //inform udc for discon
//			INFO( "[am7x_uoc] <DEV>plug out\n");
			uoc->btype = TYPE_NO_DEVICE;
			if(dtype==TYPE_PC_CONNECT){
				if(uoc->transceiver.gadget)
					port_event_inform(uoc,AOTG_PERIPHERAL_DISCONN_EVENT);
				else{
					msg.type = SYSMSG_USB;
					msg.subtype = DEVICE_USB_OUT;
					msg.dataload = DEVICE_MASS_STORAGE;
					msg.reserved[0] =USB_GROUP0;
					am_put_sysmsg(msg);	
				}
			}else if(dtype==TYPE_PC_NODPDM){
				msg.type = SYSMSG_USB;
				msg.subtype = DEVICE_USB_OUT;
				msg.dataload = DEVICE_CON_NODPDM;
				msg.reserved[0] =USB_GROUP0;
				am_put_sysmsg(msg);
			}else if(dtype==TYPE_ADP_CHARGE){
				msg.type = SYSMSG_USB;
				msg.subtype = DEVICE_USB_OUT;
				msg.dataload = DEVICE_POWER_ADAPTER;
				msg.reserved[0] =USB_GROUP0;
				am_put_sysmsg(msg);
			}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
			if((aotg_readb(OTGSTATE) & OTG_STATE_B_IDLE) == 0) usb_reset();// T932
#endif			
		}else{
			INFO("[am7x_uoc] <DEV>plug in\n");
			dtype = dev_pattern_check(uoc);
			uoc->btype = dtype;
			if(dtype==TYPE_PC_CONNECT){
				if(!uoc->transceiver.gadget){
					msg.type = SYSMSG_USB;
					msg.subtype = DEVICE_USB_IN;
					msg.dataload = DEVICE_MASS_STORAGE;
					msg.reserved[0] =USB_GROUP0;
					am_put_sysmsg(msg);		
				}else{
					//INFO("<DEV>driver exist \n");
					port_event_inform(uoc,AOTG_SOFT_CONN_EVENT);	
				}	
			}else if(dtype==TYPE_PC_NODPDM){
				if(!uoc->transceiver.gadget){  
					msg.type = SYSMSG_USB;
					msg.subtype = DEVICE_USB_IN;
					msg.dataload = DEVICE_CON_NODPDM;
					msg.reserved[0] =USB_GROUP0;
					am_put_sysmsg(msg);		
				}
			}else if(dtype==TYPE_ADP_CHARGE){
				if(!uoc->transceiver.gadget){
					msg.type = SYSMSG_USB;
					msg.subtype = DEVICE_USB_IN;
					msg.dataload = DEVICE_POWER_ADAPTER;
					msg.reserved[0] =USB_GROUP0;
					am_put_sysmsg(msg);		
				}
			}
						
		}	
			
		goto timer_mod;
	}

exit:
	dump_otg_regs();
	/*recover orignal clk set*/
	if(!is_clkopen)
		usb_clk_close();
	
timer_mod:
	mod_timer(&uoc->timer,jiffies + 100);	
}


static int aotg_id_force(struct aotg_uoc * uoc,int id)
{	
	char role;
	unsigned value,success;
	unsigned long flags;
	
	value = !!id;
	role = value ? 'B' :'A';

	DMSG_UOC("<am7x_uoc>@@---<ID FORCE>force to %c \n",role);
	
	spin_lock_irqsave(&uoc->lock,flags);
	aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL) ; 
	
	if(value)
		aotg_setbits(0x03, USBEXTCTRL);		
	else
		aotg_setbits(0x01, USBEXTCTRL);

	mdelay(5);
	
	success = (get_otg_id() == value);	
	if(!success){	
		ERR("<am7x_uoc>@@---force ID->%c failed!!\n",role);
		spin_unlock_irqrestore(&uoc->lock,flags);
		return -EFAULT;
	}	
	
	aotg_clearbits(OTGCTRL_BUSREQ,OTGCTRL) ;
	spin_unlock_irqrestore(&uoc->lock,flags);
	
	DMSG_UOC("<am7x_uoc>@@---force ID->%c sucess!!\n",role); 	
	return  0;
}


static int aotg_set_host(struct otg_transceiver *transceiver, struct usb_bus *host) 
{
	int retval,i;
	unsigned long flags;
	struct aotg_uoc *aotg =  transceiver_to_aotg(transceiver);
#ifdef CONFIG_USB_AM7X_HOST_ONLY_0

    if(!host){
        if(!aotg->transceiver.host)
            return -ESHUTDOWN;
        aotg->transceiver.host = NULL;
        clear_usb_irq();
		enable_usb_irq(0);
    }
    else{
        if(aotg->transceiver.host)
			return -EBUSY;

        aotg->transceiver.host = host;

        aotg_writew(0xffff,VBUSDBCTIMER);
		aotg_writeb(0xff,TAAIDLBDIS);/*set TA_AIDL_BDIS timeout never generate*/
		aotg_writeb(0xff,TAWAITBCON);
        uoc_set_phy(0xe6,0xe2);
		uoc_set_phy(0x83,0x0e);//0x0e
		uoc_set_phy(0x84,0x01);
        
        clear_usb_irq();
		enable_usb_irq(1);
        update_state(aotg);
    }

#else
	
	if(!host){  
		if(!aotg->transceiver.host)
		 	return -ESHUTDOWN;
		aotg->transceiver.host = NULL;
		/*  
			if is otg ,when plugout ,
			id will be 1,
			no need to force id
		*/
		retval = usb_reset();
		if(retval){
			DMSG_UOC("<am7x_uoc>@@---%s:reset failed!\n",__FUNCTION__);
			return retval;
		}	
		if(!aotg->is_otg && (get_otg_id() != default_role)){//
			aotg_setbits(1<<6,USBCS);
			retval = aotg_id_force(aotg,default_role);	
			if(retval)
				return retval;
		}else if(aotg->is_otg){
			/*if otg ,then clear force id ,
			    recover its role id indicated*/
			aotg_clearbits(1<<0,USBEXTCTRL);
		}
		uoc_set_phy(0xe6,0xa2);
	#if 1   //bluse add disconnet debounce
		uoc_set_phy(0x83,0x0e);//0x0e
		uoc_set_phy(0x84,0x01);
	#endif
		clear_usb_irq();
		enable_usb_irq(0);
		
		/*disable power */
		if(aotg->is_otg){
			am_pm_ioctl(0,AM_SYS_TO_VBUS_OFF,NULL);
			mdelay(5);
			am_pm_ioctl(0,AM_DISABLE_VBUS,NULL);
			mdelay(5);
		}

		spin_lock_irqsave(&aotg->lock,flags);
		transceiver->port_status = 0;
		transceiver->port_change= 0;
		
		spin_unlock_irqrestore(&aotg->lock,flags);

		update_state(aotg);
		
		if(!aotg->is_otg)
			usb_clk_close();	

		enable_plug_check(aotg,1);
	}else{
		if(aotg->transceiver.host)
			return -EBUSY;

		retval = usb_clk_open();
		if(retval){
			ERR("<am7x_uoc>@@---%s:open usb clk failed!\n",__FUNCTION__);
			return retval;
		}
		
		/*make sure device still plugin*/
		if(aotg_readb(OTGSTATE)!=OTG_STATE_A_HOST){
			if(!is_dev_plugin(aotg)){
					usb_clk_close();
					enable_plug_check(aotg,1);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
					usb_reset();// T932
#endif
					return -ESHUTDOWN;
			}
		}
		retval = usb_reset();
		if(retval){
			DMSG_UOC("<am7x_uoc>@@---%s:reset failed!\n",__FUNCTION__);
			return retval;
		}	
		/*set FSM timeout val*/
		aotg_writew(0xffff,VBUSDBCTIMER);
		aotg_writeb(0xff,TAAIDLBDIS);/*set TA_AIDL_BDIS timeout never generate*/
		aotg_writeb(0xff,TAWAITBCON);/*set TA_WAIT_BCON timeout never generate*/	
		//aotg_writeb(0x28,TBVBUSDISPLS);/*set dischrg_pls_timeout 20ms*/
		
		/*enable power  */
		if(aotg->is_otg){
			am_pm_ioctl(0,AM_ENABLE_VBUS,NULL);
			mdelay(5);
			am_pm_ioctl(0,AM_SYS_TO_VBUS_ON,NULL);
			mdelay(5);
		}
		uoc_set_phy(0xe6,0xe2);
	#if 1   //bluse add disconnet debounce
		uoc_set_phy(0x83,0x0e);//0x0e
		uoc_set_phy(0x84,0x01);
	#endif
		clear_usb_irq();
		enable_usb_irq(1);

		if(!aotg->is_otg)
		{
			spin_lock_irqsave(&aotg->lock,flags);
			retval = aotg_id_force(aotg,ID_ROLE_A);	
			if(retval){
				spin_unlock_irqrestore(&aotg->lock,flags);
				return retval;
			}
			spin_unlock_irqrestore(&aotg->lock,flags);
		}		
		/*let otg into status:a-wait-bcon*/
		aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL);
		aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL);
		
		update_state(aotg);
		
	#ifdef HOST_TEST_EYEDGRAM
			int val = aotg_readb(HCPORTCTRL);
			val = ( val &0xe0) |0x08;
			aotg_writeb(val,HCPORTCTRL);
			INFO("<am7x_uoc>@@---eye figure test: portctl=0x%x\n",aotg_readb(HCPORTCTRL));
			while(1);
	#endif
		/*	
		  *	FIXME:wait host state to be A-HOST,
		  *	or it will be failed,especially when plugin/out quickly
		  */

		for( i =0; i < A_HOST_WAIT_TICKS; i++){
			if(get_otg_state() == OTG_STATE_A_HOST)
				break;
			msleep(50);
		}
		
		if(get_otg_state()  != OTG_STATE_A_HOST ){		
			INFO("<%s>OTG state fault:%02x\n",__FILE__,get_otg_state() );
			/*recover to default state*/
			retval = usb_reset();
			if(retval){
				ERR("<am7x_uoc>@@---%s:reset failed!\n",__FUNCTION__);
				return retval;
			}	
			
			dump_otg_regs();
			if( (get_otg_id() == default_role)){//!aotg->is_otg &&
				aotg_setbits(1<<6,USBCS);
				#if 0
				retval = aotg_id_force(aotg,default_role);	
				if(retval)
					return retval;
				#else		
				aotg_setbits(0x03, USBEXTCTRL);	
				mdelay(5);
				#endif
			}else if(aotg->is_otg){
				/*if otg ,then clear force id , recover its role id indicated*/
				aotg_clearbits(1<<0,USBEXTCTRL);
			}
			
			msleep(10);
			
			usb_clk_close();
			enable_plug_check(aotg,1);
			return -EFAULT;
		}
		/*check if device still in*/	
		aotg->transceiver.host = host;
	}
	aotg->OtgJump=0;//bluse: initial disconnect flage
#endif
	INFO(" <am7x_uoc %s >otgstatus=%x,otgstate=%x,otgctrl=%x usbesta=%x\n",__FUNCTION__,
		aotg_readb(OTGSTATUS),aotg_readb(OTGSTATE),aotg_readb(OTGCTRL),aotg_readb(USBESTA));
	return 0;
}



static int aotg_set_peripheral(struct otg_transceiver *transceiver, struct usb_gadget *gadget) 
{
	unsigned long flags;
	int retval;
	struct aotg_uoc *aotg = transceiver_to_aotg(transceiver);
#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    printk("uoc: should not call %s under host only mode\n",__FUNCTION__);
    return 0;
#endif

	DMSG_UOC("<am7x_uoc>@@---%s##\n",__FUNCTION__);
	if(!gadget){ 	
		if(!aotg->transceiver.gadget){
			ERR("<am7x_uoc>@@---gadget is NULL\n");
			return -ESHUTDOWN;
		}	
		
		aotg->transceiver.gadget = NULL;	

		/*config wake up src for host detect*/
		config_wakeup_src();
		set_vbus_detect_threshold(1);
		if(get_otg_id() != default_role){
			INFO("<am7x_uoc>@@---recover to orignal role\n");
			retval = aotg_id_force(aotg,default_role);
			if(retval !=0)
				return retval;	
		}
		
		clear_usb_irq();
		enable_usb_irq(0);

		usb_clk_close();
		
		spin_lock_irqsave(&aotg->lock,flags);
		transceiver->port_status = 0;
		transceiver->port_change= 0;
		spin_unlock_irqrestore(&aotg->lock,flags);			
	}else{ /*bus request for device*/

		if(aotg->transceiver.gadget){
			ERR("<am7x_uoc>@@---owned gadget before\n");
			return -EBUSY;
		}
		aotg->transceiver.gadget = gadget;

		retval = usb_clk_open();
		if(retval){
			ERR("<am7x_uoc>@@---usb clk revalid failed \n");
			return retval;
		}

		/*config wake up src for host detect*/
		config_wakeup_src();
		set_vbus_detect_threshold(1);
	//	aotg_clearbits(1<<6,USBCS);
		if(get_otg_id() != ID_ROLE_B){
			retval = aotg_id_force(aotg,ID_ROLE_B);
			if(retval !=0)
				return retval;	
		}
		
		clear_usb_irq();
		enable_usb_irq(1);	
	}

	update_state(aotg);	
	
	INFO("#<am7x_uoc %s>otgstatus=%x,otgstate=%x usbesta=%x\n",__FUNCTION__,aotg_readb(OTGSTATUS),aotg_readb(OTGSTATE),aotg_readb(USBESTA));
	return 0;
}


/*------------------------------------------------------*/
//for otg transceiver	
#ifdef SUPPORT_OTG_SWITCH
static int aotg_start_hnp(struct otg_transceiver *transceiver )
{
	return 0;	
}

static int aotg_start_srp(struct otg_transceiver *transceiver)
{
	return 0;
}
#endif

static int aotg_set_power(struct otg_transceiver *transceiver,int is_on)
{
	u8	state  = get_otg_state();
	struct aotg_uoc  	* aotg = transceiver_to_aotg(transceiver);
	DMSG_UOC("<am7x_uoc>@@---<TRANSCEVIER_POWER> set power :%d\n",is_on);

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
     return 0;
#endif

	if(is_on){
		if(!is_dev_plugin(aotg)){
			dump_otg_regs();
			enable_plug_check(aotg,1);
			DMSG_UOC("<am7x_uoc>@@---<TRANSCEVIER_POWER>dev non exist\n");
			return -ESHUTDOWN;
		}
		
		if(state == OTG_STATE_A_IDLE){			
			DMSG_UOC("<am7x_uoc>@@---<TRANSCEVIER_POWER> power on\n");
			aotg->transceiver.state = OTG_STATE_A_WAIT_BCON;
			aotg_clearbits(OTGCTRL_ABUSDROP,OTGCTRL); 
			aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL); 
		}else if ((state == OTG_STATE_A_HOST ) ||(state == OTG_STATE_A_WAIT_BCON)){
			DMSG_UOC("<am7x_uoc>@@---<TRANSCEVIER_POWER>aleady power on,do nothing\n");	
		}else{
			ERR("<am7x_uoc>@@---<TRANSCEVIER_POWER>inappropriate OTG state,hw: %02x  sw: %02x\n",
					state,aotg->transceiver.state);
			dump_otg_regs();
			return -EFAULT;
		}			
	}else{
		DMSG_UOC("<am7x_uoc>@@---<TRANSCEVIER_POWER> power off\n");
		aotg_clearbits(OTGCTRL_BUSREQ,OTGCTRL);
		aotg_setbits(OTGCTRL_ABUSDROP,OTGCTRL); /*a_bus_drop*/	
	}
	
	return 0;
}

static void  aotg_init_transceiver(struct otg_transceiver *transceiver)
{
	transceiver->default_a = 0;
	transceiver->gadget = NULL;
	transceiver->host = NULL;
	transceiver->label = driver_name;
	transceiver->port_status= 0;
	transceiver->port_change = 0;
	transceiver->set_power = aotg_set_power;
	transceiver->set_host = aotg_set_host;
	transceiver->set_peripheral = aotg_set_peripheral;
//	transceiver->set_suspend = aotg_set_suspend;
	transceiver->dev = &(transceiver_to_aotg(transceiver)->dev);
	
#ifdef SUPPORT_OTG_SWITCH
	transceiver->start_hnp = aotg_start_hnp;
	transceiver->start_srp = aotg_start_srp;
#else
	transceiver->start_hnp = NULL;
	transceiver->start_srp = NULL;
#endif
	transceiver->state = OTG_STATE_UNDEFINED;
}

struct otg_transceiver * otg_get_transceiver (void)
{		
       return aotg_to_transceiver (the_controller);
}
EXPORT_SYMBOL(otg_get_transceiver);

/*-----------------------------------------------------------*/
static void set_vbus_detect_threshold(unsigned int th)
{
	unsigned long  val;
	val = aotg_readl(USB_PHY_CTL);
	DMSG_UOC("<am7x_uoc>@@--- %s:old val:0x%lx\n",__FUNCTION__,val);

	val &=0xffffffcf;
	val |= (th << 4);
	aotg_writel(val, USB_PHY_CTL);
	DMSG_UOC("<am7x_uoc>@@--- %s:new val:0x%08x\n",__FUNCTION__,aotg_readl(USB_PHY_CTL));
}



static void clear_usb_irq(void)
{
	aotg_clear_pend(0xff,USBIRQ);
	aotg_writeb(aotg_readb(OTGIRQ),OTGIRQ);
	aotg_writeb(aotg_readb(USBEIRQ),USBEIRQ);
	aotg_writeb(aotg_readb(USBESTA),USBESTA);
}


static void enable_usb_irq(int enable)
{
	int  is_en = !!enable;
	if(is_en){ /*enable*/	
		aotg_writeb(0xff,OTGIEN);
		aotg_setbits(USBEIRQ_USBIRQEN,USBEIRQ);
	}else{ 	 /*disable*/
		aotg_writeb(0x00,OTGIEN);
		aotg_clearbits(USBEIRQ_USBIRQEN,USBEIRQ);
	}
}


static void config_wakeup_src(void)
{
#if 0
	aotg_writeb(0x2,CLKGATE);
   	aotg_writeb(0x04,USBEIRQ);
#endif	
}


static int usb_reset(void)
{
	int cnt;
	usb_cmu_reset();
	mdelay(5);
	aotg_setbits( USBERES_USBREST,USBERES);
	for(cnt=0;cnt < USB_RESET_TICKS;cnt++)
	{
		if(!(aotg_readb(USBERES)&0x01))
		{
			/*set phy */
			uoc_set_phy(0xe6,0x66);
			uoc_set_phy(0xe2,0x8a);//before:0x86
			uoc_set_phy(0xe3,0x64);
		  //bluse add disconnet debounce
			uoc_set_phy(0x83,0x0e);//0x0e
			uoc_set_phy(0x84,0x01);
		/*if the product is ezcast,wo should disable disconnct detect*/
		#if MODULE_CONFIG_EZCAST_ENABLE
			#if EZMUSIC_ENABLE
			//uoc_set_phy(0xe7,0x1f); 
			#else
//			printk("EZCast-0 disable disconn detect\n");
			uoc_set_phy(0xe7,0x0f); 
			#endif
		#else
			uoc_set_phy(0xe7,0x1f);  
		#endif
		
			return 0;
		}
		mdelay(5);
	}		

	DMSG_UOC("<am7x_uoc>@@--- <USB_RESET>cannot reset usb otg\n");
	return -EBUSY;
}



/*for sys to show device status*/
static ssize_t show_a_plug(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int plugstate = 0;
	if(get_otg_id() == ID_ROLE_A)
		plugstate = 1;
	return sprintf(buf, "%d\n", plugstate);
}


static ssize_t show_b_plug(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct aotg_uoc *uoc = container_of(dev,struct aotg_uoc,dev);
	int detectpin = uoc->vbus_gpio.detectpin;
	int plugstate = 0;
	int vbusvalid;
	/*only check whether we as a device plug in or not */

	if(uoc->is_otg){ /*check vbus*/
		vbusvalid = aotg_readb(USBESTA) & (1<<6) ? 1 : 0 ;
		if(vbusvalid && (get_otg_id() == ID_ROLE_B))
			plugstate = 1;
	}else
		plugstate = get_detect_gpioval(detectpin);

	return sprintf(buf, "%d\n", plugstate);
}

static ssize_t show_b_type(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct aotg_uoc *uoc = container_of(dev,struct aotg_uoc,dev);
	int btype = uoc->btype;
	
	return sprintf(buf, "%d\n", btype);
}


static DEVICE_ATTR(a_plug, 0444, show_a_plug, NULL);
static DEVICE_ATTR(b_plug, 0644, show_b_plug, NULL);
static DEVICE_ATTR(b_type, 0444, show_b_type, NULL);

static int aotg_uoc_enable(struct aotg_uoc * uoc,int enable)
{
	int i,retval = 0;
	u32 chipid[4];
	u32 efuse_val;

	if(enable){
		/*now we all used internal resistor*/
		if(0 == internal_resistor){
#ifndef CONFIG_AM_8251
			aotg_writel(EXTERN_EFUSE,USB_EFUSE_REG);
			printk("aotg usb efuse reg value:0x%x\n",aotg_readl(USB_EFUSE_REG));
#endif
		}else{
			efuse_val = aotg_readl(USB_EFUSE_REG);
			if(!(efuse_val & (1<<6))){
				INFO("<am7x_uoc>@@---use efuse val from CHIP ID\n");
				aotg_writel(0x301065,NOR_CHIPID);
				msleep(5);
				aotg_writel(0,NOR_CHIPID);
				for(i=0;i<4;i++)
					chipid[i] = aotg_readl(NOR_CHIPID);
				print_hex_dump(KERN_ALERT, "chip id: ", DUMP_PREFIX_OFFSET,
 						16, 1, chipid, 4*4, 1);
			}else{
				INFO("<am7x_uoc>@@---use efuse val from efuse reg\n");
				INFO("<am7x_uoc>@@---orignal val:%x\n", efuse_val & 0x3f);
			}

			/*FIXME:if needed ,we can set efuse val from module-paramter 
			  inter_resistor*/
		}
		set_vbus_detect_threshold(1); 
		
		/*usb reset*/
		retval = usb_reset();
		if(retval){
			DMSG_UOC("<am7x_uoc>@@--- %s:reset failed\n",__FUNCTION__);
			return retval;
		}	
		
		/*set phy */

		uoc_set_phy(0xe6,0xe0);
		uoc_set_phy(0xe2,0x80);
		#if MODULE_CONFIG_EZCAST_ENABLE
			#if EZMUSIC_ENABLE
			uoc_set_phy(0xe7,0x0b); //enable disconnect check		
			#endif
		#else
		//uoc_set_phy(0xe7,0x0b);
		#endif
		uoc_set_phy(0xe7,0x1f);
		mdelay(5);
		uoc_set_phy(0xe2,0x8a);	
		
#ifndef CONFIG_USB_AM7X_HOST_ONLY_0        
		/*hide ourself*/
		dplus_down();
		aotg_writew(0x2000,VBUSDBCTIMER);
#else
		act_writel((act_readl(GPIO_MFCTL1)|(0x1<<3)), GPIO_MFCTL1);
		act_writel((act_readl(0xb01c0018)|(0x1<<30)), 0xb01c0018);
		act_writel((act_readl(0xb01c0020)|(0x1<<30)), 0xb01c0020);
#endif

		/*clear all irq*/
		uoc->enable = 1;	
	}else{
		usb_clk_close();
		uoc->enable = 0;	
	}
	
	return 0;
}


static void __exit aotg_uoc_remove(void)
{	
	struct aotg_uoc *uoc = the_controller;

#ifdef UOC_FILE_OPERATION
	device_destroy(uoc->uoc_class,uoc->uoc_char);
	 unregister_chrdev_region(uoc->uoc_char, 2);
	 class_destroy(uoc->uoc_class);
#endif
	del_timer(&uoc->timer);
	if(uoc->transceiver.gadget){
		dplus_down();
		port_event_inform(uoc,AOTG_PERIPHERAL_DISCONN_EVENT);
	}

	if(uoc->transceiver.host){
		; /*fixme:inform host*/
	}
	
	aotg_uoc_enable(uoc,0);
	clear_usb_irq();
	enable_usb_irq(0);	

	device_remove_file(&uoc->dev, &dev_attr_b_plug);
	device_remove_file(&uoc->dev, &dev_attr_b_type);
	if(uoc->is_otg)
		device_remove_file(&uoc->dev, &dev_attr_a_plug);
	remove_debug_file(uoc);
	
	free_irq(IRQ_USB,uoc);
	kfree(uoc);
	uoc = the_controller = NULL;
}



static int __init aotg_uoc_init(void)
{
	struct aotg_uoc	*uoc;
	int		retval;	
	u8 		tmp;
	
	DMSG_UOC("<am7x_uoc>@@--- enter uoc_probe\n");
	uoc = (struct aotg_uoc *)kzalloc(sizeof(struct aotg_uoc),GFP_KERNEL);
	if(!uoc) {
		ERR("<am7x_uoc>@@--- <uoc_probe>alloc uoc failed!\n");
		return  -ENOMEM;
	}
	the_controller = uoc;

	/*set default value*/
	tmp = get_sys_info()->sys_gpio_cfg.usb_det;

	uoc->is_otg = is_otg = ((tmp == 0xff) ? 1 : 0);
	if(!is_otg)
		uoc->vbus_gpio.detectpin = detect_gpio = tmp;

#if (EZCAST_ENABLE && FLASH_TYPE && !EZWIRE_TYPE)
	uoc->is_otg = 0;
	uoc->vbus_gpio.detectpin = detect_gpio = 0xff;
#endif
	
	uoc->vbus_gpio.val = 0;
	uoc->prevstate = OTG_STATE_UNDEFINED;
	spin_lock_init(&uoc->lock);
	init_timer(&uoc->timer);
	aotg_init_transceiver(&uoc->transceiver);

	if((retval = usb_clk_open())!=0){
		INFO("<am7x_uoc>@@--- clk open failed\n");
		return retval;
	}

	/*request irq*/
	if(request_irq(IRQ_USB,
				aotg_uoc_irq,
				IRQF_SHARED |IRQF_DISABLED,
				driver_name,
				(void*)uoc) != 0 ) {
		ERR ("<am7x_uoc>@@--- uoc request irq %d failed\n",IRQ_USB);
		retval = -EBUSY;
		goto handle_err;
	}
	if(aotg_uoc_enable(uoc,1) < 0){
		ERR (" #<am7x_uoc>@@--- uoc enable failed\n");
		retval = -EFAULT;
		goto handle_err1;
	}		
#ifdef UOC_FILE_OPERATION  //register char driver
	int result;
	uoc->uoc_class= class_create(THIS_MODULE,"usb-uoc");
	if (IS_ERR(uoc->uoc_class)) {
		result = PTR_ERR(uoc->uoc_class);
		ERR("unable to create usb_gadget class %d\n", result);
		//goto failed;
	}

	if ( (result = alloc_chrdev_region(&uoc->uoc_char, 
			0, 1,"usb_uoc")) < 0) {
		ERR("alloc_chrdev_region %d\n", result);
		//class_destroy(uoc->uoc_class);
		//goto failed1;
	}

	device_create_drvdata(uoc->uoc_class, NULL,
				      uoc->uoc_char,
				      NULL, "usb_uoc");

	cdev_init(&uoc_dev, &uoc_operations);
	uoc_dev.owner = THIS_MODULE;
	result= cdev_add(&uoc_dev, uoc->uoc_char, 1);
	if (result) {
		ERR( "Failed to create char device\n");
		//goto failed2;
	}
#endif
	clear_usb_irq();
	enable_usb_irq(0);	

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    /*
     * Set otg to host mode only. The final stat will be a_wait_bcon.
     * 
     * Modify by Simon Lee.
     */

    /* Set 0x403[0]=1, 0x403[1]=0, 0x403[4]=1 to let otg go to the a_idle status */
    aotg_writeb(0x11,USBEXTCTRL);
    mdelay(1);
    /* Set 0x1BE[0]=1 to let otg go to the a_wait_bcon */
    aotg_setbits(0x1,OTGCTRL);
    mdelay(5);

    /* Set 0x1C2[7]=1 to disable the ta_wait_bcon timeout */
    aotg_setbits(0x80,TAWAITBCON);
    
    update_state(uoc);
#else
	/*kewen:force to default role,enable FORCE ID*/
	if(!uoc->is_otg ){
		if(get_otg_state()!=0x08){
		if(aotg_id_force(uoc,default_role) != 0){
			ERR (" <am7x_uoc>@@--- uoc id force failed\n");
			retval = -EFAULT;
			goto handle_err1;		
		}
		}
	}

	if(!uoc->is_otg){
		if(uoc->vbus_gpio.detectpin != 0xff)
			init_detect_gpio(uoc->vbus_gpio.detectpin);
	}else{
		/*config pin for drv-vbus*/
		config_drv_vbus();
	}
	update_state(uoc);
	usb_clk_close();
#endif 
	/*create proc for uoc*/
	create_debug_file(uoc);
	uoc->dev.parent = NULL;
	dev_set_drvdata(&uoc->dev, uoc);
	dev_set_name(&uoc->dev,"usb-uoc");
	if ((retval = device_register(&uoc->dev)) != 0) {
		ERR("<am7x_uoc>@@--- failed to register device: %d\n", retval);
		goto handle_err1;
	}

	retval = device_create_file(&uoc->dev,&dev_attr_b_plug);
	if(retval != 0){
		ERR ("<am7x_uoc>@@---  create sys entry b_plug failed!!\n");
		goto handle_err2;
	}

	retval = device_create_file(&uoc->dev,&dev_attr_b_type);
	if(retval != 0){
		ERR ("<am7x_uoc>@@---  create sys entry b_type failed!!\n");
		goto handle_err2;
	}

	if(!uoc->is_otg){
		retval = device_create_file(&uoc->dev,&dev_attr_a_plug) ;
		if(retval != 0){
			ERR ("<am7x_uoc>@@---  create sys entry a_plug failed!!\n");
			device_remove_file(&uoc->dev, &dev_attr_b_type);
			device_remove_file(&uoc->dev, &dev_attr_b_plug);
			goto handle_err2;
		}	
	}

	/*start timer detect plugin/out*/
	uoc->timer.function = plug_detect_handler; 
	uoc->timer.data = (unsigned long )uoc;
	uoc->timer.expires = jiffies + 100;  /*100 jiffes*/
	enable_plug_check(uoc,1);
	INFO("<am7x_uoc>@@--- %s: probe success!\n",driver_name);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
	INFO("%s: usb_reset execute\n",driver_name);
    usb_reset();
#endif

	return 0;
	
handle_err2:
	device_unregister(&uoc->dev);
handle_err1:
	free_irq(IRQ_USB,(void *)uoc);
handle_err:
	kfree(uoc);
	the_controller = uoc = NULL;
	return retval;
}

module_init(aotg_uoc_init);
module_exit(aotg_uoc_remove);


MODULE_DESCRIPTION("actions uoc driver");
MODULE_AUTHOR("liucan,2010");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:am7x_uoc");
