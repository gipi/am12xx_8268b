/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    interrupt.c - Linux network device driver interrupt processing.

    Author      Date        Description
    THa         01/05/04    Created file.
    THa         06/27/05    Updated for version 0.1.5.
    THa         06/29/05    Updated for Linux 2.6.
    THa         10/18/05    Updated for Renesas 7751R board.
    THa         01/13/06    Transfer dword-aligned data for performance.
    THa         04/03/06    Use TWO_NETWORK_INTERFACE instead of TWO_PORTS.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"
#include "device.h"
#include <linux/etherdevice.h>
#include <asm/mach-am7x/actions_regs.h>
#include <asm/mach-am7x/actions_io.h>

/* -------------------------------------------------------------------------- */

/*
    dev_rcv

    Description:
        This routine is called to receive a packet.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

static void dev_rcv (
    struct net_device* dev )
{
    int              packet_len;
    struct dev_priv* priv = (struct dev_priv*)netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    struct sk_buff*  skb;
    BYTE*            data;
    USHORT           wData;
#if defined( TWO_NETWORK_INTERFACE )  ||  !defined( NO_STATS )
    int              port = MAIN_PORT;
#endif

    do {
        packet_len = HardwareReceiveLength( pHardware );
//        printk("plen = %u, INT_PD=0x%08x, INT_MSK=0x%08x !\n", packet_len, 
//        	*(volatile int*)0xb0020000, *(volatile int*)0xb0020004);
        if ( !packet_len ) {
            goto next_packet;
        }

        /* Allocate extra bytes for 32-bit transfer. */
        skb = dev_alloc_skb( packet_len + 6 );
        if ( !skb ) {
            priv->stats.rx_dropped++;
	    printk("drop!\n");
            goto next_packet;
        }

        /* Align socket buffer in 4-byte boundary for better performance. */
        skb_reserve( skb, 2 );
        data = skb_put( skb, packet_len );

        HardwareReceiveBuffer( pHardware, data, packet_len );

        skb->dev = dev;
        skb->protocol = eth_type_trans( skb, dev );
        skb->ip_summed = CHECKSUM_NONE;

        /* Update receive statistics. */
#ifndef NO_STATS
        pHardware->m_cnCounter[ port ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]++;
        pHardware->m_cnCounter[ port ][ OID_COUNTER_DIRECTED_BYTES_RCV ] +=
            packet_len;
#endif
        priv->stats.rx_packets++;
        priv->stats.rx_bytes += packet_len;

        /* Notify upper layer for received packet. */
        dev->last_rx = jiffies;
        netif_rx( skb );

next_packet:
        HardwareReadRegWord( pHardware, REG_RX_MEM_INFO_BANK,
            REG_RX_MEM_INFO_OFFSET, &wData );
    } while ( wData );
}  /* dev_rcv */

/* -------------------------------------------------------------------------- */

/*
    dev_interrupt

    Description:
        This routine is called by upper network layer to signal interrupt.

    Parameters:
        int irq
            Interrupt number.

        void* dev_id
            Pointer to network device.

        struct pt_regs regs
            Pointer to registers.

    Return (None):
*/

irqreturn_t ks8842i_dev_interrupt (
    int             irq,
    void*           dev_id )
{
    WORD               IntEnable;
    struct net_device* dev = ( struct net_device* ) dev_id;
    struct dev_priv*   priv = (struct dev_priv*)netdev_priv(dev);
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;
#ifdef DBG
    int                port = MAIN_PORT;
#endif

    HardwareDisableInterrupt( pHardware );
    HardwareSaveRegs( pHardware );

#ifdef DEBUG_COUNTER
    pHardware->m_nGood[ COUNT_GOOD_INT ]++;
#endif
    HardwareReadInterrupt( pHardware, &IntEnable );

	while ( IntEnable ) {

#ifdef DEBUG_COUNTER
        pHardware->m_nGood[ COUNT_GOOD_INT_LOOP ]++;
#endif
        HardwareAcknowledgeInterrupt( pHardware, IntEnable );

        if ( ( IntEnable & INT_TX ) ) {
            ks8842i_transmit_done( dev );
        }

        if ( ( IntEnable & INT_TX_UNDERRUN ) ) {
            printk("!!!!!!!!!underun!!!!!!!!!!!!!!!!\n");
            ks8842i_transmit_done( dev );
#if 0
            /* Acknowledge the interrupt. */
            HardwareAcknowledgeUnderrun( pHardware );
#endif
            /* Wait for the transmit timeout to reset the hardware. */
            netif_stop_queue( dev );
        }

        if ( ( IntEnable & INT_RX ) ) {
//	 printk("enter dev_rcv!\n");

            /* Receive the packet. */
            dev_rcv( dev );

            /* Acknowledge the interrupt. */
            HardwareAcknowledgeReceive( pHardware );
        }

        if ( ( IntEnable & INT_PHY ) ) {
            SwitchGetLinkStatus( pHardware );
        }

#ifdef DBG
        if ( ( IntEnable & INT_RX_OVERRUN ) ) {
            DBG_PRINT( "!R"NEWLINE );

            pHardware->m_cnCounter[ port ][ OID_COUNTER_RCV_NO_BUFFER ]++;
            priv->stats.rx_errors++;
            priv->stats.rx_fifo_errors++;
        }

        if ( ( IntEnable & INT_RX_STOPPED ) ) {
            WORD wData;

            pHardware->m_wInterruptMask &= ~INT_RX_STOPPED;
            DBG_PRINT( "Rx stopped\n" );
	    printk("Rx stopped !\n");
            HardwareReadRegWord( pHardware, REG_RX_CTRL_BANK,
                REG_RX_CTRL_OFFSET, &wData );
            if ( ( wData & RX_CTRL_ENABLE ) ) {
                DBG_PRINT( "Rx disabled\n" );
		printk("Rx disabled\n");
            }
            HardwareAcknowledgeInterrupt( pHardware, INT_RX_STOPPED );
            break;
        }

        if ( ( IntEnable & INT_TX_STOPPED ) ) {
            WORD wData;

            pHardware->m_wInterruptMask &= ~INT_TX_STOPPED;
            DBG_PRINT( "Tx stopped\n" );
                printk("Tx stopped\n");

            HardwareReadRegWord( pHardware, REG_RX_CTRL_BANK,
                REG_TX_CTRL_OFFSET, &wData );
            if ( ( wData & TX_CTRL_ENABLE ) ) {
                DBG_PRINT( "Tx disabled\n" );
                printk("Tx disabled\n");

            }
            HardwareAcknowledgeInterrupt( pHardware, INT_TX_STOPPED );
            break;
        }
#endif
        HardwareReadInterrupt( pHardware, &IntEnable );
    }

    HardwareRestoreRegs( pHardware );
	if(pHardware->m_wInterruptMask != 0xe000)
		printk("in interrupt, i found m_wInterrupt =%x\n",pHardware->m_wInterruptMask);
    HardwareEnableInterrupt( pHardware );

//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
    return IRQ_HANDLED;
//#endif
}  /* dev_interrupt */
