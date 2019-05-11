/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device.h - Linux network device driver header.

    Author      Date        Description
    THa         01/05/04    Created file.
    THa         09/27/04    Updated for PCI version.
    THa         05/10/05    Updated for two port devices.
    THa         06/27/05    Updated for version 0.1.5.
    THa         06/29/05    Updated for Linux 2.6.
    THa         07/07/05    Changed /proc entries.
    THa         04/03/06    Use TWO_NETWORK_INTERFACE instead of TWO_PORTS.
    THa         05/31/06    Add ethtool support.
   ---------------------------------------------------------------------------
*/


#ifndef __DEVICE_H
#define __DEVICE_H


#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/mii.h>


/* -------------------------------------------------------------------------- */

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
#include <asm/hitachi_7751se.h>

static inline void mach_led(int position, int value)
{
	volatile unsigned short* p = (volatile unsigned short*)PA_LED;

	if (value) {
		*p |= (position<<8);
	} else {
		*p &= ~(position<<8);
	}
}  /* mach_led */
#endif

/* -------------------------------------------------------------------------- */

typedef struct {
    struct timer_list* pTimer;
    void*              pData;
    unsigned int       cnCount;
    int                nCount;
    int                nTime;
} TTimerInfo, *PTTimerInfo;

/* -------------------------------------------------------------------------- */

typedef struct {
    wait_queue_head_t       wqhCounter;
    unsigned long           time;
    int                     fRead;
} COUNTER_INFO, *PCOUNTER_INFO;

/* -------------------------------------------------------------------------- */

struct dev_info {
    spinlock_t              lockHardware;
    wait_queue_head_t       wqhHardware;
    struct proc_dir_entry*  proc_main;
    struct proc_dir_entry*  proc_port[ 2 ];
    struct semaphore        proc_sem;

    HARDWARE                hw;

    int                     id;

    TTimerInfo              MonitorTimerInfo;
    struct timer_list       timerMonitor;

    COUNTER_INFO            Counter[ TOTAL_PORT_NUM ];

#ifdef TWO_NETWORK_INTERFACE
    struct net_device*      pDev[ 2 ];
#endif
};


struct dev_priv {
    struct net_device_stats stats;

    struct dev_info         dev_info;
    struct dev_info*        pDevInfo;

    spinlock_t              lock;

    struct mii_if_info      mii_if;

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    struct mii_if_info      mii_if_2;
#endif
    u32                     advertising;

    /* debug message level */
    u32                     msg_enable;

    struct sk_buff*         skb_waiting;

    int                     media_state;
    int                     opened;

#ifdef TWO_NETWORK_INTERFACE
    int                     port;
#endif
};

/* -------------------------------------------------------------------------- */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
void

#else
irqreturn_t
#endif
ks8842i_dev_interrupt (
    int             irq,
    void*           dev_id);

/* -------------------------------------------------------------------------- */

void ks8842i_transmit_alloc_done (
    struct net_device* dev );

void ks8842i_transmit_done (
    struct net_device* dev );

void ks8842i_transmit_empty (
    struct net_device* dev );

int ks8842i_dev_transmit (
    struct sk_buff*    skb,
    struct net_device* dev );

void ks8842i_dev_transmit_timeout (
    struct net_device *dev );

#define DEV_INTERRUPT_PTR     ks8842i_dev_interrupt

#define TRANSMIT_ALLOC_DONE   ks8842i_transmit_alloc_done
#define TRANSMIT_DONE         ks8842i_transmit_done
#define TRANSMIT_EMPTY        ks8842i_transmit_empty
#define DEV_TRANSMIT          ks8842i_dev_transmit
#define DEV_TRANSMIT_TIMEOUT  ks8842i_dev_transmit_timeout

/* -------------------------------------------------------------------------- */

int AcquireHardware (
    struct dev_info* priv,
    int              in_intr,
    int              wait );

void ReleaseHardware (
    struct dev_info* priv,
    int              in_intr );

/* -------------------------------------------------------------------------- */

#endif
