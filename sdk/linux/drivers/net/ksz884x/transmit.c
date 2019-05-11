/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    transmit.c - Linux network device driver transmit processing.

    Author      Date        Description
    THa         01/05/04    Created file.
    THa         06/27/05    Updated for version 0.1.5.
    THa         10/18/05    Updated for Renesas 7751R board.
    THa         01/13/06    Transfer dword-aligned data for performance.
    THa         03/15/06    Correct 10Mbit transmit stop problem.
    THa         04/03/06    Use TWO_NETWORK_INTERFACE instead of TWO_PORTS.
    THa         06/05/06    Make sure interrupts are enabled before processing
                            them.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"
#include "device.h"
#include <asm/irq.h>
#include <asm/mach-am7x/actions_regs.h>
#include <asm/mach-am7x/actions_io.h>
#ifdef DBG
#if 0
#define DEBUG_TX_DATA
#endif
#if 0
#define SIMULATE_TIMEOUT
#define SEND_STOP_COUNT  30
#endif
#endif

#if 1
#define SKB_WAITING
#endif

/* -------------------------------------------------------------------------- */

#ifdef SIMULATE_TIMEOUT
static int sendcount = 0;
#endif

/*
    send_packet

    Description:
        This function is used to send a packet out to the network.

    Parameters:
        struct sk_buff* skb
            Pointer to socket buffer.

        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int send_packet (
    struct sk_buff*    skb,
    struct net_device* dev )
{
    struct dev_priv* priv = (struct dev_priv*)netdev_priv( dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              len;

#if defined( TWO_NETWORK_INTERFACE )  ||  !defined( NO_STATS )
    int              port = MAIN_PORT;
#endif
    int              rc = 1;

#ifdef TWO_NETWORK_INTERFACE
    /* Indicate which port has sent packet. */
    port = priv->port;
    pHardware->m_bPortTX = DEV_TO_HW_PORT( port );
    pHardware->m_fPortTX |= port + 1;
#endif

#if ETH_ZLEN > 60
    len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
#else
    /* Hardware will pad the length to 60. */
    len = skb->len;
#endif

    HardwareSetTransmitLength( pHardware, len );

#ifdef SH_32BIT_ALIGNED
    memcpy( pHardware->m_bLookahead, skb->data, len );
    HW_WRITE_BUFFER( pHardware, REG_DATA_OFFSET, pHardware->m_bLookahead,
        len );

#else
    HW_WRITE_BUFFER( pHardware, REG_DATA_OFFSET, skb->data, len );
#endif
    HW_WRITE_BYTE( pHardware, REG_TXQ_CMD_OFFSET, TXQ_CMD_ENQUEUE_PACKET );

#if 0
    if ( HardwareSendPacket( pHardware ) )
#endif
    {
        rc = 0;

        /* Update transmit statistics. */
#ifndef NO_STATS
        pHardware->m_cnCounter[ port ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]++;
        pHardware->m_cnCounter[ port ][ OID_COUNTER_DIRECTED_BYTES_XMIT ] +=
            len;
#endif
        priv->stats.tx_packets++;
        priv->stats.tx_bytes += len;

        dev->trans_start = jiffies;

#ifdef SIMULATE_TIMEOUT
        ++sendcount;
        if ( SEND_STOP_COUNT == sendcount  ) {
            DBG_PRINT( "stop sending:%lu\n", jiffies );
            netif_stop_queue( dev );
        }
#endif
    }

#ifdef SKB_WAITING
    priv->skb_waiting = NULL;
#endif

#ifdef DEBUG_TX_DATA
    {
        int    nLength;
        UCHAR* bData = ( PUCHAR ) skb->data;

        for ( nLength = 0; nLength < len; nLength++ )
        {
            DBG_PRINT( "%02x ", bData[ nLength ]);
            if ( ( nLength % 16 ) == 15 )
            {
                DBG_PRINT( NEWLINE );
            }
        }
        DBG_PRINT( NEWLINE );
    }
#endif

    return( rc );
}  /* send_packet */


#ifdef SKB_WAITING
/*
    send_next_packet

    Description:
        This function is used to send the next packet waiting to be sent.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int send_next_packet (
    struct net_device* dev )
{
    struct dev_priv* priv = (struct dev_priv*)netdev_priv( dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              rc = 1;

    if ( priv->skb_waiting  &&
            HardwareAllocPacket( pHardware, priv->skb_waiting->len ) ) {
        struct sk_buff* skb = priv->skb_waiting;

        rc = send_packet( priv->skb_waiting, dev );
        dev_kfree_skb_irq( skb );
        netif_wake_queue( dev );
    }
    return( rc );
}  /* send_next_packet */
#endif


/*
    transmit_done

    Description:
        This routine is called when the transmit interrupt is triggered,
        indicating either a packet is sent successfully or there are transmit
        errors.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

void
ks8842i_transmit_done (
    struct net_device* dev )
{
    struct dev_priv* priv = (struct dev_priv*)netdev_priv( dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;

#ifdef TWO_NETWORK_INTERFACE
    int              port = MAIN_PORT;

    port = HW_TO_DEV_PORT( pHardware->m_bPortTX );
    if ( OTHER_PORT == port ) {
        dev = hw_priv->pDev[ OTHER_PORT ];
        priv = ( struct dev_priv* ) dev->priv;
    }
#endif
    if ( !HardwareTransmitDone( pHardware ) ) {

        /* Update transmit error statistics. */
    }
    else {

#ifdef SIMULATE_TIMEOUT
        if ( sendcount < SEND_STOP_COUNT )
#endif
        if ( netif_queue_stopped( dev ) )

#ifdef SKB_WAITING
            send_next_packet( dev );

#else
            netif_wake_queue( dev );
#endif
    }
}  /* transmit_done */


/*
    transmit_empty

    Description:
        This routine is called when the transmit empty interrupt is triggered,
        indicating no more packets to be sent.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

void
ks8842i_transmit_empty (
    struct net_device* dev )
{

#ifdef TWO_NETWORK_INTERFACE
    struct dev_priv* priv = ( struct dev_priv* ) dev->priv;
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              port = MAIN_PORT;

    for ( port = 0; port < 2; port++ ) {
        if ( OTHER_PORT == port ) {
            dev = hw_priv->pDev[ OTHER_PORT ];
            priv = ( struct dev_priv* ) dev->priv;
        }

        /* This port has sent packets. */
        if ( ( pHardware->m_fPortTX & ( port + 1 )) ) {
#endif
            if ( netif_queue_stopped( dev ) )
                netif_wake_queue( dev );

#ifdef TWO_NETWORK_INTERFACE
        }
#endif

#ifdef SKB_WAITING
        send_next_packet( dev );
#endif

#ifdef TWO_NETWORK_INTERFACE
    }
    pHardware->m_fPortTX = 0;
#endif
}  /* transmit_empty */

/* -------------------------------------------------------------------------- */

/*
    dev_transmit

    Description:
        This function is used by the upper network layer to send out a packet.

    Parameters:
        struct sk_buff* skb
            Pointer to socket buffer.

        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

int
ks8842i_dev_transmit (
    struct sk_buff*    skb,
    struct net_device* dev )
{
    struct dev_priv* priv = (struct dev_priv*)netdev_priv( dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              len;
    int              rc = 0;
    WORD             wInterruptMask;

#ifdef SKB_WAITING
    /* There is a packet waiting to be sent.  Transmit queue should already be
       stopped.
    */
    if ( priv->skb_waiting ) {

#ifdef DBG
        printk( "waiting to send!\n" );
#endif
        priv->stats.tx_aborted_errors++;
        if ( !netif_queue_stopped( dev ) ) {
            DBG_PRINT( "queue not stopped!\n" );
            netif_stop_queue( dev );
        }
        printk("%s:%d: tx, rc=%d\n", __func__,__LINE__,rc);
        return 1;
    }
#endif
    WORD IntStatus;
    HardwareReadInterrupt( pHardware, &IntStatus );

    ULONG dwReadInterrupt;
    HardwareReadRegDWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        &dwReadInterrupt );
    dwReadInterrupt &= 0x0000FFFF;

//    printk("IntEnable = %x IntStatus = %x \n",dwReadInterrupt, IntStatus);


#if ETH_ZLEN > 60
    len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
#else
    /* Hardware will pad the length to 60. */
    len = skb->len;
#endif

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        netif_stop_queue( dev );
        printk("%s:%d: tx, rc=%d\n", __func__,__LINE__,rc);
        return( rc );
    }

#ifdef TWO_NETWORK_INTERFACE
    pHardware->m_bPortTX = DEV_TO_HW_PORT( priv->port );
#endif

#if 0
    /* Blocking interrupts below does not prevent the interrupt handler be
       called.
    */
    disable_irq( dev->irq );
#endif
    /* Save the current interrupt mask and block all interrupts. */
    wInterruptMask = HardwareBlockInterrupt( pHardware );
	if(wInterruptMask != 0xe000)
		printk("in Send Packet, i found wInterrupt =%x\n",wInterruptMask);

#ifdef SKB_WAITING
    priv->skb_waiting = skb;
#endif
    if ( HardwareAllocPacket( pHardware, len ) ) {
        rc = send_packet( skb, dev );
        dev_kfree_skb( skb );
    }
    else {

#ifdef DBG
        printk( "wait to send:%lu\n", jiffies );
#endif

        /* Stop the transmit queue until packet is allocated. */
        netif_stop_queue( dev );
    }
	if(wInterruptMask != 0xe000)
		printk("in Send Packet, i found m_wInterrupt =%x\n",wInterruptMask);

    HardwareSetInterrupt( pHardware, wInterruptMask );
//    HardwareEnableInterrupt( pHardware );

    ReleaseHardware( hw_priv, FALSE );
//	printk("%s:%d: tx, rc=%d\n", __func__,__LINE__,rc);
    return( rc );
}  /* dev_transmit */

/* -------------------------------------------------------------------------- */

/*
    dev_transmit_timeout

    Description:
        This routine is called when the transmit timer expires.  That indicates
        the hardware is not running correctly because transmit interrupts are
        not triggered to free up resources so that the transmit routine can
        continue sending out packets.  The hardware is reset to correct the
        problem.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

void ks8842i_dev_transmit_timeout (
    struct net_device *dev )
{
    static unsigned long last_reset = 0;

    struct dev_priv* priv = (struct dev_priv*)netdev_priv( dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              port;

	printk( "transmit timeout:%lu\n", jiffies);

#ifdef TWO_NETWORK_INTERFACE

    /* Only reset the hardware if time between calls is long. */
    if ( jiffies - last_reset <= dev->watchdog_timeo )
        hw_priv = NULL;
#endif
    last_reset = jiffies;
    if ( hw_priv ) {
        AcquireHardware( hw_priv, FALSE, TRUE );
        HardwareDisableInterrupt( pHardware );
        HardwareDisable( pHardware );

        HardwareReset( pHardware );
        HardwareSetup( pHardware );
            
#ifdef TWO_NETWORK_INTERFACE
        PortSet_STP_State( pHardware, MAIN_PORT, STP_STATE_BLOCKED );
        PortSet_STP_State( pHardware, OTHER_PORT, STP_STATE_BLOCKED );
#endif

        /* Reset may wipe out these registers. */
        if ( pHardware->m_bMacOverrideAddr )
            HardwareSetAddress( pHardware );
        if ( pHardware->m_bMulticastListSize )
            HardwareSetGroupAddress( pHardware );
    }

#ifdef TWO_NETWORK_INTERFACE
    for ( port = MAIN_PORT; port <= OTHER_PORT; port++ ) {
        struct net_device* net_if = hw_priv->pDev[ port ];

        /* The second net device is not created. */
        if ( !net_if )
            continue;

        priv = ( struct dev_priv* ) net_if->priv;

#else
    for ( port = MAIN_PORT; port < OTHER_PORT; port++ ) {
#endif

        /* This port device is opened for use. */
        if ( priv->opened ) {
        
#ifdef TWO_NETWORK_INTERFACE
            PortSet_STP_State( pHardware, port, STP_STATE_FORWARDING );
#endif

#ifdef SKB_WAITING
            if ( priv->skb_waiting ) {
                priv->stats.tx_aborted_errors++;
                dev_kfree_skb( priv->skb_waiting );
                priv->skb_waiting = NULL;
            }
#endif
        }
    }

    if ( hw_priv ) {
        HardwareEnable( pHardware );
        HardwareEnableInterrupt( pHardware );

        ReleaseHardware( hw_priv, FALSE );
    }

    dev->trans_start = jiffies;
    netif_wake_queue( dev );

#ifdef SIMULATE_TIMEOUT
    sendcount = 0;
#endif
}  /* dev_transmit_timeout */
