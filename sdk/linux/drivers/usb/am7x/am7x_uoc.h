/*
 * Copyright (C) 2004 by Thomas Rathbone, HP Labs
 * Copyright (C) 2005 by Ivan Kokshaysky
 * Copyright (C) 2006 by SAN People
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
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef AOTG_UOC_H
#define AOTG_UOC_H

#include <linux/usb/otg.h>
#include <linux/timer.h>

/*bq:add open uoc file operations api*/
//#define UOC_FILE_OPERATION 

/*for proc fs */
#define COFIG_PROC_FOR_DEBUG_UOC

#define	AOTG_DUAL_ROLE_PERIPHERAL	1
#define	AOTG_DUAL_ROLE_HOST			0

#define  AOTG_IRQ_IDLE   				OTGIRQ_IDLE
#define  AOTG_IRQ_HOST  				OTGIRQ_LOCSOF
#define  AOTG_IRQ_PERIPH				OTGIRQ_PERIPH
#define  AOTG_IRQ_VBUSERR  			OTGIRQ_VBUSEER
#define  AOTG_IRQ_SPRDET        		OTGIRQ_SRPDET
#define  AOTG_IRQ_HOST_AND_PERI	OTGIRQ_PERIPH | OTGIRQ_LOCSOF
#define  AOTG_IRQ_HOST_AND_VERR	OTGIRQ_LOCSOF | OTGIRQ_VBUSEER
#define  AOTG_IRQ_IDLE_AND_HOST    OTGIRQ_IDLE | OTGIRQ_LOCSOF
#define  AOTG_IRQ_IDLE_AND_PERI    	OTGIRQ_IDLE | OTGIRQ_PERIPH
#define  AOTG_IRQ_HOST_AND_IDLE_AND_PERI		OTGIRQ_IDLE | OTGIRQ_LOCSOF | OTGIRQ_PERIPH

/*for usb plugin/out detect */
#define AOTG_DEVIN_MASK			(USBESTA_LINE0 | USBESTA_LINE1 )

/*for usb plug status*/
#define B_DEV_START			0x11
#define B_DEV_ATTATCH		0x22
#define A_DEV_START			0x33
#define A_DEV_ATTATCH		0x44

/*support type*/
#define ONLY_B					0x11
#define ONLY_A					0x22
#define A_AND_B				0x33

#define ID_ROLE_A			0
#define ID_ROLE_B			1

#define UOC_DISCONNECT                  1
#define UOC_RECONNECT                    2

/*device type define*/
enum b_device_type { 
	TYPE_NO_SUPPORT=-1,
	TYPE_NO_DEVICE, 
	TYPE_PC_CONNECT, 
	TYPE_PC_NODPDM, 
	TYPE_ADP_CHARGE
}; 

struct usb_vbusgpio{
	unsigned  detectpin;   /*0 means vbus check ,others for gpio*/
	unsigned   val;	     /*plug val*/	
};

struct aotg_uoc {
	spinlock_t				lock;
#ifdef COFIG_PROC_FOR_DEBUG_UOC
	struct proc_dir_entry 		*pde;	
#endif
	struct    device			dev;
	struct otg_transceiver		transceiver;
	enum usb_otg_state		prevstate;
	
	u8						dual_role;
	u8 						is_otg;
	 /*bluse add for disconnect flage*/
	u8   						OtgFlage;  //bluse add for disconnect flage
	u8						OtgJump;//record otg state jump 
	unsigned					enable :1;
	unsigned					id : 1;
	unsigned 					enable_check:1;

	enum b_device_type		btype;
	struct 	delayed_work 	work; 
	/*used for gpio*/
	struct usb_vbusgpio		vbus_gpio;
	struct timer_list 			timer;
#ifdef UOC_FILE_OPERATION  //bq add for ioctrl operations
	dev_t 			uoc_char;
	struct class * uoc_class;
	dev_t 			uoc_next_char;
	struct class * uoc_next_class;
#endif
};

// T166,William 20150814: for jugement of massive plug inout events
#define USB_CUR_STATUS_PLUG_OUT	0
#define USB_CUR_STATUS_PLUG_IN	1

static inline struct aotg_uoc *  transceiver_to_aotg(struct otg_transceiver *transceiver) {
	return container_of(transceiver,struct aotg_uoc,transceiver);
}

static inline struct otg_transceiver * aotg_to_transceiver(struct aotg_uoc *aotg) {
	return &aotg->transceiver;
}

static inline void set_a_fsm_defaults (struct aotg_uoc *aotg)
{
       	aotg->transceiver.default_a = 1;
	aotg->dual_role = AOTG_DUAL_ROLE_HOST;	   
}

static inline void set_b_fsm_defaults (struct aotg_uoc *aotg)
{
       	aotg->transceiver.default_a = 0;
	aotg->dual_role = AOTG_DUAL_ROLE_PERIPHERAL;
}
#endif

