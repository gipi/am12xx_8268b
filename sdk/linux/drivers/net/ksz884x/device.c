/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device.c - Linux network device driver.

    Author      Date        Description
    THa         01/05/04    Created file.
    THa         10/14/04    Updated with latest specs.
    THa         05/10/05    Updated for two port devices.
    THa         06/27/05    Updated for version 0.1.5.
    THa         06/29/05    Updated for Linux 2.6.
    THa         07/07/05    Changed /proc entries.
    THa         10/18/05    Provide proc entries to change duplex and speed.
    THa         01/13/06    Test SH generic bus transfer rate.
    THa         03/14/06    Release 1.0.0.
    THa         04/03/06    Use TWO_NETWORK_INTERFACE instead of TWO_PORTS.
    THa         04/06/06    Implement MII IO calls.
    THa         05/31/06    Add ethtool support.
    THa         06/05/06    Make sure interrupts are enabled before processing
                            them.
   ---------------------------------------------------------------------------
*/


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/mach-am7x/actions_regs.h>
#include <asm/mach-am7x/actions_io.h>
#include <asm/mach-am7x/am7x_platform_device.h>

#include "target.h"
#include "hardware.h"
#include "device.h"

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/phy.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
#include <linux/ethtool.h>
#endif
#include <asm/irq.h>
#if 1
#include "ks_ioctl.h"

#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);

int hwread
(
   unsigned char   BankNum,
   unsigned long   RegAddr,
   unsigned char   Width
);
#endif


#if 0
#define STP_SUPPORT
#endif


#ifdef SH_BUS
#ifndef SH_32BIT_ACCESS_ONLY
#ifdef DBG
#if 1
#define SH_BUS_TEST
#endif
#endif
#endif
#endif


#ifdef STP_SUPPORT
#ifdef KBUILD_BASENAME
#include <../net/bridge/br_private.h>

#else
/* From Linux 2.4 */

/* net/bridge/br_private_timer.h */
struct br_timer
{
        int running;
        unsigned long expires;
};

/* net/bridge/br_private.h */
typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

struct bridge_id
{
        unsigned char   prio[2];
        unsigned char   addr[6];
};

struct mac_addr
{
        unsigned char   addr[6];
        unsigned char   pad[2];
};

struct net_bridge;

struct net_bridge_port
{
        struct net_bridge_port  *next;
        struct net_bridge       *br;
        struct net_device       *dev;
        int                     port_no;

        /* STP */
        port_id                 port_id;
        int                     state;
        int                     path_cost;
        bridge_id               designated_root;
        int                     designated_cost;
        bridge_id               designated_bridge;
        port_id                 designated_port;
        unsigned                topology_change_ack:1;
        unsigned                config_pending:1;
        int                     priority;

        struct br_timer         forward_delay_timer;
        struct br_timer         hold_timer;
        struct br_timer         message_age_timer;
};
#endif
#endif


#ifdef DBG
#define NET_MSG_ENABLE  ( NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK )
#else
#define NET_MSG_ENABLE  0
#endif

#define DRV_NAME     AM7X_DEV_NAME_NET_KSZ884X

#ifdef SH_BUS
#ifdef SH_32BIT_ACCESS_ONLY
#define BUS_NAME     "SH 32-bit only"
#elif defined( SH_32BIT_ALIGNED )
#define BUS_NAME     "SH 32-bit"
#else
#define BUS_NAME     "SH 16-bit"
#endif
#else
#define BUS_NAME     "ISA"
#endif
#define DRV_VERSION  "1.0.3"
#define DRV_RELDATE  "Jun 7, 2006"

static char version[] __devinitdata =
    "Micrel " DRV_NAME " " BUS_NAME " " DRV_VERSION " (" DRV_RELDATE ")";

PHARDWARE phw;

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
static int led_count = 0;
#endif


#ifdef DBG

/* 2 lines buffer. */
#define DEBUG_MSG_BUF   ( 80 * 2 )

#define PRINT_MSG_SIZE  ( 80 * 20 )
#define PRINT_INT_SIZE  ( 80 * 10 )

#define DEBUG_MSG_SIZE  ( PRINT_MSG_SIZE + PRINT_INT_SIZE + \
    DEBUG_MSG_BUF * 2 )


static char* strDebug = NULL;
static char* strIntBuf = NULL;
static char* strMsg = NULL;
static char* strIntMsg = NULL;
static int   cnDebug = 0;
static int   cnIntBuf = 0;
static int   fLastDebugLine = 1;
static int   fLastIntBufLine = 1;

static unsigned long lockDebug = 0;
#endif


#define DBG_CH  '-'


void DbgMsg (
    char *fmt, ... )
{
#ifdef DBG
    va_list args;
    char**  pstrMsg;
    int*    pcnDebug;
    int     nLeft;
    int     in_intr = in_interrupt();

    pcnDebug = &cnDebug;
    pstrMsg = &strMsg;
    nLeft = PRINT_MSG_SIZE - cnDebug - 1;

    /* Called within interrupt routines. */
    if ( in_intr ) {

        /* If not able to get lock then put in the interrupt message buffer.
        */
        if ( test_bit( 1, &lockDebug ) ) {
            pcnDebug = &cnIntBuf;
            pstrMsg = &strIntMsg;
            nLeft = PRINT_INT_SIZE - cnIntBuf - 1;
            in_intr = 0;
        }
    }
    else {
        set_bit( 1, &lockDebug );
    }
    va_start( args, fmt );
    nLeft = vsnprintf( *pstrMsg, nLeft, fmt, args );
    va_end( args );
    if ( nLeft ) {
        *pcnDebug += nLeft;
        *pstrMsg += nLeft;
    }
    if ( !in_intr ) {
        clear_bit( 1, &lockDebug );
    }
#endif
}  /* DbgMsg */

/* -------------------------------------------------------------------------- */

/*
    StartTimer

    Description:
        This routine starts the kernel timer after the specified time tick.

    Parameters:
        PTTimerInfo pInfo
            Pointer to kernel timer information.

        int nTime
            The time tick.

    Return (None):
*/

static void StartTimer (
    PTTimerInfo pInfo,
    int         nTime )
{
    pInfo->cnCount = 0;
    pInfo->pTimer->expires = jiffies + nTime;
    add_timer( pInfo->pTimer );

    /* infinity */
    pInfo->nCount = -1;
}  /* StartTimer */


/*
    StopTimer

    Description:
        This routine stops the kernel timer.

    Parameters:
        PTTimerInfo pInfo
            Pointer to timer information.

    Return (None):
*/

static void StopTimer (
    PTTimerInfo pInfo )
{
    pInfo->nCount = 0;
    del_timer_sync( pInfo->pTimer );
}  /* StopTimer */

/* -------------------------------------------------------------------------- */

/*
    InitHardware

    Description:
        This routine initializes system variables required to acquire the
        hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

    Return (None):
*/

static void InitHardware (
    struct dev_info* priv )
{
    ASSERT( priv->hw.m_ulVIoAddr );

    spin_lock_init( &priv->lockHardware );
    init_waitqueue_head( &priv->wqhHardware );
}  /* InitHardware */


/*
    AcquireHardware

    Description:
        This function is used to acquire the hardware so that only one process
        has access to the hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

        int in_intr
            Indicate calling from interrupt routines.

        int wait
            Indicate whether to wait or not.  User functions should wait, while
            periodic timer routine should not.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

int AcquireHardware (
    struct dev_info* priv,
    int              in_intr,
    int              wait )
{
    int rc;
    unsigned long flags = 0;

    ASSERT( priv->hw.m_ulVIoAddr );
    do {

        /* Acquire access to the hardware acquire count variable. */
        if ( in_intr )
            spin_lock( &priv->lockHardware );
        else
            //spin_lock_irq( &priv->lockHardware );
            spin_lock_irqsave( &priv->lockHardware, flags );
        if ( 0 == priv->hw.m_bAcquire ) {

            /* Increase hardware acquire count. */
            priv->hw.m_bAcquire++;

            /* Release access to the hardware acquire count variable. */
            if ( in_intr )
                spin_unlock( &priv->lockHardware );
            else
                //spin_unlock_irq( &priv->lockHardware );
                spin_unlock_irqrestore( &priv->lockHardware, flags );

            /* Hardware acquired. */
            return 0;
        }

        /* Release access to the hardware acquire count variable. */
        if ( in_intr )
            spin_unlock( &priv->lockHardware );
        else
            //spin_unlock_irq( &priv->lockHardware );
            spin_unlock_irqrestore( &priv->lockHardware, flags );

        /* Willing to wait. */
        if ( wait ) {
            if ( ( rc = wait_event_interruptible( priv->wqhHardware,
                    !priv->hw.m_bAcquire )) ) {

                /* System failure. */
                return( rc );
            }
        }
    } while ( wait );

    /* Hardware not acquired. */
    return -EBUSY;
}  /* AcquireHardware */


/*
    ReleaseHardware

    Description:
        This routine is used to release the hardware so that other process can
        have access to the hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

        int in_intr
            Indicate calling from interrupt routines.

    Return (None):
*/

void ReleaseHardware (
    struct dev_info* priv,
    int              in_intr )
{
    unsigned long flags = 0;
    ASSERT( priv->hw.m_ulVIoAddr );

    /* Acquire access to the hardware acquire count variable. */
    if ( in_intr )
        spin_lock( &priv->lockHardware );
    else
        //spin_lock_irq( &priv->lockHardware );
        spin_lock_irqsave( &priv->lockHardware, flags );

    /* Decrease hardware acquire count. */
    priv->hw.m_bAcquire--;

    /* Wake up other processes waiting in the queue. */
    if ( 0 == priv->hw.m_bAcquire )
        wake_up_interruptible( &priv->wqhHardware );

    /* Release access to the hardware acquire count variable. */
    if ( in_intr )
        spin_unlock( &priv->lockHardware );
    else
        //spin_unlock_irq( &priv->lockHardware );
        spin_unlock_irqrestore( &priv->lockHardware, flags );
}  /* ReleaseHardware */

/* -------------------------------------------------------------------------- */

#if 0
/*
    HardwareDisableInterruptSync

    Description:
        This procedure makes sure the interrupt routine is not entered after
        the hardware interrupts are disabled.

    Parameters:
        void* ptr
            Pointer to hardware information structure.

    Return (None):
*/

void HardwareDisableInterruptSync (
    void* ptr )
{
    PHARDWARE          pHardware = ( PHARDWARE ) ptr;
    struct net_device* dev = ( struct net_device* ) pHardware->m_pDevice;

    disable_irq( dev->irq );
    HardwareDisableInterrupt( pHardware );
    enable_irq( dev->irq );
}  /* HardwareDisableInterruptSync */
#endif

/* -------------------------------------------------------------------------- */

#define proc_dir_name  "driver/ksz8842_isa"


typedef enum {

    /* Read-only entries. */
    PROC_INFO,

    /* Read-write entries. */
    PROC_SET_DUPLEX,
    PROC_SET_SPEED,

    PROC_SET_BROADCAST_STORM,
    PROC_SET_DIFFSERV,
    PROC_SET_802_1P,

    /* Port specific read-only entries. */
    PROC_GET_CABLE_STATUS,
    PROC_GET_LINK_STATUS,

    /* Port specific read-write entries. */
    PROC_ENABLE_BROADCAST_STORM,
    PROC_ENABLE_DIFFSERV,
    PROC_ENABLE_802_1P,
    PROC_ENABLE_PRIORITY_RATE,

    PROC_SET_PORT_BASED,

    PROC_SET_RX_P0_RATE,
    PROC_SET_RX_P1_RATE,
    PROC_SET_RX_P2_RATE,
    PROC_SET_RX_P3_RATE,
    PROC_SET_TX_P0_RATE,
    PROC_SET_TX_P1_RATE,
    PROC_SET_TX_P2_RATE,
    PROC_SET_TX_P3_RATE,

    PROC_SET_CAPABILITIES,

    PROC_LAST
} TProcNum;


typedef struct {
    TProcNum proc_num;
    char*    proc_name;
} TProcInfo, *PTProcInfo;


static TProcInfo ProcInfo[ PROC_LAST ] = {

    /* Read-only entries. */
    { PROC_INFO,                    "info",                 },

    /* Read-write entries. */
    { PROC_SET_DUPLEX,              "duplex",               },
    { PROC_SET_SPEED,               "speed",                },

    { PROC_SET_BROADCAST_STORM,     "broadcast_percent",    },
    { PROC_SET_DIFFSERV,            "v_diffserv",           },
    { PROC_SET_802_1P,              "v_802_1p",             },

    /* Port specific read-only entries. */
    { PROC_GET_CABLE_STATUS,        "cable_status",         },
    { PROC_GET_LINK_STATUS,         "link_status",          },

    /* Port specific read-write entries. */
    { PROC_ENABLE_BROADCAST_STORM,  "broadcast_storm",      },
    { PROC_ENABLE_DIFFSERV,         "diffserv",             },
    { PROC_ENABLE_802_1P,           "p_802_1p",             },
    { PROC_ENABLE_PRIORITY_RATE,    "priority_rate",        },

    { PROC_SET_PORT_BASED,          "port_based",           },

    { PROC_SET_RX_P0_RATE,          "rx_p0_rate",           },
    { PROC_SET_RX_P1_RATE,          "rx_p1_rate",           },
    { PROC_SET_RX_P2_RATE,          "rx_p2_rate",           },
    { PROC_SET_RX_P3_RATE,          "rx_p3_rate",           },
    { PROC_SET_TX_P0_RATE,          "tx_p0_rate",           },
    { PROC_SET_TX_P1_RATE,          "tx_p1_rate",           },
    { PROC_SET_TX_P2_RATE,          "tx_p2_rate",           },
    { PROC_SET_TX_P3_RATE,          "tx_p3_rate",           },

    { PROC_SET_CAPABILITIES,        "set_capabilities",     },
};


typedef struct {
    struct dev_info* priv;
    PTProcInfo       info;
    int              port;
} TProcPortInfo, *PTProcPortInfo;


TProcPortInfo ProcPortInfo[ 3 ][ PROC_LAST ];


static struct proc_dir_entry* proc_info;
static struct proc_dir_entry* proc_set;


/*
    read_proc

    Description:
        This function process the read operation of /proc files.

    Parameters:

    Return (int):
        The length of buffer data returned.
*/

static
int read_proc (
    char*  buf,
    char** start,
    off_t  offset,
    int    count,
    int*   eof,
    void*  data )
{
    int              i;
    int              len = 0;
    int              processed = TRUE;
    int              rc;
    PTProcPortInfo   proc = ( PTProcPortInfo ) data;
    PTProcInfo       info = proc->info;
    struct dev_info* priv = proc->priv;
    PHARDWARE        pHardware = &priv->hw;
    DWORD            dwData[ 5 * 2 ];

    /* Why want more? */
    if ( offset ) {
        *eof = 1;
        return( 0 );
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( down_interruptible( &priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }

#if 0
    len += sprintf( buf + len, "%s:%d %d\n", info->proc_name, info->proc_num,
        proc->port );
#endif
    switch ( info->proc_num ) {
        case PROC_SET_DUPLEX:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_bDuplex );
            if ( MediaStateConnected == pHardware->m_ulHardwareState ) {
                if ( 1 == pHardware->m_ulDuplex )
                    len += sprintf( buf + len, "half-duplex\n" );
                else if ( 2 == pHardware->m_ulDuplex )
                    len += sprintf( buf + len, "full-duplex\n" );
            }
            else
                len += sprintf( buf + len, "unlinked\n" );
            break;
        case PROC_SET_SPEED:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_bSpeed );
            if ( MediaStateConnected == pHardware->m_ulHardwareState ) {
                len += sprintf( buf + len, "%lu\n",
                    pHardware->m_ulTransmitRate / 10000 );
            }
            else
                len += sprintf( buf + len, "unlinked\n" );
            break;
        case PROC_SET_BROADCAST_STORM:
            len += sprintf( buf + len, "%u%%\n",
                pHardware->m_bBroadcastPercent );
            break;
        case PROC_SET_DIFFSERV:
            for ( i = 0; i < 64; i++ ) {
                len += sprintf( buf + len, "%2u=0x%02X ", i, ( int )
                    pHardware->m_wDiffServ[ i ]);
                if ( ( i % 8 ) == 7 )
                    len += sprintf( buf + len, "\n" );
            }
            break;
        case PROC_SET_802_1P:
            for ( i = 0; i < 8; i++ )
                len += sprintf( buf + len, "%u=%u ", i,
                    pHardware->m_b802_1P_Priority[ i ]);
            len += sprintf( buf + len, "\n" );
            break;
        case PROC_SET_PORT_BASED:
            len += sprintf( buf + len, "%u\n",
                pHardware->m_Port[ proc->port ].bPortPriority );
            break;
        case PROC_SET_RX_P0_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwRxRate[ 0 ]);
            break;
        case PROC_SET_RX_P1_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwRxRate[ 1 ]);
            break;
        case PROC_SET_RX_P2_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwRxRate[ 2 ]);
            break;
        case PROC_SET_RX_P3_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwRxRate[ 3 ]);
            break;
        case PROC_SET_TX_P0_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwTxRate[ 0 ]);
            break;
        case PROC_SET_TX_P1_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwTxRate[ 1 ]);
            break;
        case PROC_SET_TX_P2_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwTxRate[ 2 ]);
            break;
        case PROC_SET_TX_P3_RATE:
            len += sprintf( buf + len, "%u\n", ( int )
                pHardware->m_Port[ proc->port ].dwTxRate[ 3 ]);
            break;
        default:
            processed = FALSE;
    }

    /* Require hardware to be acquired first. */
    if ( !processed ) {
        if ( ( rc = AcquireHardware( priv, FALSE, TRUE )) ) {
            up( &priv->proc_sem );
            return( rc );
        }
        switch ( info->proc_num ) {
            case PROC_ENABLE_BROADCAST_STORM:
                len += sprintf( buf + len, "%d\n",
                    PortGetBroadcastStorm( pHardware, proc->port ));
                break;
            case PROC_ENABLE_DIFFSERV:
                len += sprintf( buf + len, "%d\n",
                    PortGetDiffServ( pHardware, proc->port ));
                break;
            case PROC_ENABLE_802_1P:
                len += sprintf( buf + len, "%d\n",
                    PortGet802_1P( pHardware, proc->port ));
                break;
            case PROC_ENABLE_PRIORITY_RATE:
                len += sprintf( buf + len, "%d\n",
                    PortGetPriority( pHardware, proc->port ));
                break;
            case PROC_GET_CABLE_STATUS:
                HardwareGetCableStatus( pHardware, proc->port, dwData );
                len += sprintf( buf + len,
                    "^[%lu:%lu, %lu:%lu, %lu:%lu, %lu:%lu, %lu:%lu]$\n",
                    dwData[ 0 ], dwData[ 1 ],
                    dwData[ 2 ], dwData[ 3 ],
                    dwData[ 4 ], dwData[ 5 ],
                    dwData[ 6 ], dwData[ 7 ],
                    dwData[ 8 ], dwData[ 9 ]);
                break;
            case PROC_GET_LINK_STATUS:
                HardwareGetLinkStatus( pHardware, proc->port, dwData );
                len += sprintf( buf + len,
                    "^[%08lX, %lX, %08lX, %08lX, %08lX, %08lX]$\n",
                    dwData[ 0 ], dwData[ 1 ], dwData[ 2 ],
                    dwData[ 3 ], dwData[ 4 ], dwData[ 5 ]);
                break;
            default:
                break;
        }
        ReleaseHardware( priv, FALSE );
    }
    up( &priv->proc_sem );
    *eof = 1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif

    return len;
}  /* read_proc */


/*
    write_proc

    Description:
        This function process the write operation of /proc files.

    Parameters:

    Return (int):
        The length of buffer data accepted.
*/

static
int write_proc (
    struct file*  file,
    const char*   buffer,
    unsigned long count,
    void*         data )
{
    int              len;
    int              num;
    int              rc;
    char             str[ 22 ];
    PTProcPortInfo   proc = ( PTProcPortInfo ) data;
    PTProcInfo       info = proc->info;
    struct dev_info* priv = proc->priv;
    PHARDWARE        pHardware = &priv->hw;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( count > 20 )
        len = 20;
    else
        len = count;
    if ( copy_from_user( str, buffer, len ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -EFAULT;
    }
    if ( down_interruptible( &priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }
    if ( ( rc = AcquireHardware( priv, FALSE, TRUE )) ) {
        up( &priv->proc_sem );
        return( rc );
    }
    if ( '0' == str[ 0 ]  &&  'x' == str[ 1 ] ) {
        sscanf( &str[ 2 ], "%x", ( unsigned int* ) &num );
    }
    else if ( '0' == str[ 0 ]  &&  'b' == str[ 1 ] ) {
        int i = 2;

        num = 0;
        while ( str[ i ] ) {
            num <<= 1;
            num |= str[ i ] - '0';
            i++;
        }
    }
    else if ( '0' == str[ 0 ]  &&  'd' == str[ 1 ] ) {
        sscanf( &str[ 2 ], "%u", &num );
    }
    else
        sscanf( str, "%d", &num );
    switch ( info->proc_num ) {
        case PROC_SET_DUPLEX:
            if ( num <= 2 )
                pHardware->m_bDuplex = ( BYTE ) num;
            break;
        case PROC_SET_SPEED:
            if ( 0 == num  ||  10 == num  ||  100 == num )
                pHardware->m_bSpeed = ( BYTE ) num;
            break;
        case PROC_SET_BROADCAST_STORM:
            HardwareConfigBroadcastStorm( pHardware, num );
            break;
        case PROC_ENABLE_BROADCAST_STORM:
            if ( !num )
                SwitchDisableBroadcastStorm( pHardware, proc->port );
            else
                SwitchEnableBroadcastStorm( pHardware, proc->port );
            break;
        case PROC_ENABLE_DIFFSERV:
            if ( !num )
                SwitchDisableDiffServ( pHardware, proc->port );
            else
                SwitchEnableDiffServ( pHardware, proc->port );
            break;
        case PROC_ENABLE_802_1P:
            if ( !num )
                SwitchDisable802_1P( pHardware, proc->port );
            else
                SwitchEnable802_1P( pHardware, proc->port );
            break;
        case PROC_ENABLE_PRIORITY_RATE:
            if ( !num )
                SwitchDisableMultiQueue( pHardware, proc->port );
            else
                SwitchEnableMultiQueue( pHardware, proc->port );
            break;
#if 0
        case PROC_SET_DIFFSERV:
            pHardware->m_wDiffServ[ 0 ] = ( ULONG ) num;
            break;
        case PROC_SET_802_1P:
            HardwareConfig802_1P_Priority( pHardware, ( UCHAR ) num );
            break;
#endif
        case PROC_SET_PORT_BASED:
            SwitchConfigPortBased( pHardware, proc->port, num );
            break;
        case PROC_SET_RX_P0_RATE:
            HardwareConfigRxPriorityRate( pHardware, proc->port, 0, num );
            break;
        case PROC_SET_RX_P1_RATE:
            HardwareConfigRxPriorityRate( pHardware, proc->port, 1, num );
            break;
        case PROC_SET_RX_P2_RATE:
            HardwareConfigRxPriorityRate( pHardware, proc->port, 2, num );
            break;
        case PROC_SET_RX_P3_RATE:
            HardwareConfigRxPriorityRate( pHardware, proc->port, 3, num );
            break;
        case PROC_SET_TX_P0_RATE:
            HardwareConfigTxPriorityRate( pHardware, proc->port, 0, num );
            break;
        case PROC_SET_TX_P1_RATE:
            HardwareConfigTxPriorityRate( pHardware, proc->port, 1, num );
            break;
        case PROC_SET_TX_P2_RATE:
            HardwareConfigTxPriorityRate( pHardware, proc->port, 2, num );
            break;
        case PROC_SET_TX_P3_RATE:
            HardwareConfigTxPriorityRate( pHardware, proc->port, 3, num );
            break;
        case PROC_SET_CAPABILITIES:
            HardwareSetCapabilities( pHardware, proc->port, num );
            break;
        default:
            printk( KERN_ALERT "write_proc:%d\n", info->proc_num );
    }
    ReleaseHardware( priv, FALSE );
    up( &priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif

    return len;
}  /* write_proc */

/* -------------------------------------------------------------------------- */

/*
    netdev_close

    Description:
        This function process the close operation of network device.  This is
        caused by the user command "ifconfig ethX down."

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int netdev_close (
    struct net_device* dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              rc;

#ifdef DBG
    int i;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif
//printk("net close\n");
    priv->opened--;

    if ( !( priv->opened ) ) {
        if ( priv->skb_waiting ) {
            priv->stats.tx_aborted_errors++;
            dev_kfree_skb( priv->skb_waiting );
            priv->skb_waiting = NULL;
        }
        netif_stop_queue( dev );

        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
            return( rc );
        }



            if ( hw_priv->MonitorTimerInfo.nCount ) {
                StopTimer( &hw_priv->MonitorTimerInfo );
            }

            HardwareDisableInterrupt( pHardware );
            HardwareDisable( pHardware );

            free_irq( dev->irq, dev );


        ReleaseHardware( hw_priv, FALSE );
    }

#ifdef DBG
    DBG_PRINT( "counters:\n" );
    for ( i = OID_COUNTER_FIRST; i < OID_COUNTER_LAST; i++ )
    {
        DBG_PRINT( "%u = %u, %u\n", i, ( int ) pHardware->m_cnCounter[ 0 ][ i ],
            ( int ) pHardware->m_cnCounter[ 1 ][ i ]);
    }
    DBG_PRINT( "wait delays:\n" );
    for ( i = WAIT_DELAY_FIRST; i < WAIT_DELAY_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, pHardware->m_nWaitDelay[ i ]);
    }
    DBG_PRINT( "bad:\n" );
    for ( i = COUNT_BAD_FIRST; i < COUNT_BAD_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, ( int ) pHardware->m_nBad[ i ]);
    }
    DBG_PRINT( "good:\n" );
    for ( i = COUNT_GOOD_FIRST; i < COUNT_GOOD_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, ( int ) pHardware->m_nGood[ i ]);
    }
#endif

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
    mach_led( 0x80, 0 );
#endif
    return 0;
}  /* netdev_close */


/*
    netdev_open

    Description:
        This function process the open operation of network device.  This is
        caused by the user command "ifconfig ethX up."

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int netdev_open (
    struct net_device* dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              port = MAIN_PORT;
    int              rc = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif
    if ( !( priv->opened ) ) {

        /* Reset device statistics. */
        memset( &priv->stats, 0, sizeof( struct net_device_stats ));

        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
            MOD_DEC_USE_COUNT;
#endif
            return( rc );
        }


        {
            struct net_device* hw_dev = dev;

#if 1 /* use poll controller instead of interrupt!! */
            if ( ( rc = request_irq( hw_dev->irq, DEV_INTERRUPT_PTR, 0,
                    hw_dev->name, hw_dev )) ) {
                ReleaseHardware( hw_priv, FALSE );
                return( rc );
            }
#endif

            pHardware->m_bPromiscuous = FALSE;
            pHardware->m_bAllMulticast = FALSE;
            pHardware->m_bMulticastListSize = 0;
            pHardware->m_ulHardwareState = MediaStateDisconnected;

            /* Initialize to invalid value so that link detection is done. */
            pHardware->m_PortInfo[ MAIN_PORT ].bLinkPartner = 0xFF;
            pHardware->m_PortInfo[ MAIN_PORT ].ulHardwareState =
                MediaStateDisconnected;

            hw_priv->Counter[ MAIN_PORT ].time = jiffies + HZ * 4;

            hw_priv->Counter[ HOST_PORT ].time =
                hw_priv->Counter[ MAIN_PORT ].time + HZ * 2;

            HardwareReset( pHardware );
            HardwareSetup( pHardware );
            HardwareSwitchSetup( pHardware );

            if ( pHardware->m_bMacOverrideAddr )
                HardwareSetAddress( pHardware );

            PortInitCounters( pHardware, HOST_PORT );

            HardwareSetupInterrupt( pHardware );
            HardwareEnable( pHardware );
            HardwareEnableInterrupt( pHardware );

            StartTimer( &hw_priv->MonitorTimerInfo,
                hw_priv->MonitorTimerInfo.nTime );
        }
        memset(( void* ) pHardware->m_cnCounter[ port ], 0,
            ( sizeof( ULONGLONG ) * OID_COUNTER_LAST ) );
        PortInitCounters( pHardware, port );


        priv->media_state = pHardware->m_ulHardwareState;

        ReleaseHardware( hw_priv, FALSE );

        if ( MediaStateConnected == priv->media_state )
            netif_carrier_on( dev );
        else
            netif_carrier_off( dev );
        if ( netif_msg_link( priv ) ) {
            printk(KERN_INFO "%s link %s\n", dev->name,
                ( MediaStateConnected == priv->media_state ? "on" : "off" ));
        }
		HardwareInitialize(hw_priv);
        netif_start_queue( dev );
    }
    priv->opened++;

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
    mach_led( 0x80, 1 );
#endif
    return 0;
}  /* netdev_open */


/*
    dev_query_statistics

    Description:
        This function returns the statistics of the network device.  The device
        needs not be opened.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (struct net_device_stat*):
        Network device statistics.
*/

static struct net_device_stats* dev_query_statistics (
    struct net_device* dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    PHARDWARE        pHardware = &priv->pDevInfo->hw;
    PPORT_CONFIG     pPort;
    int              port = MAIN_PORT;

    pPort = &pHardware->m_Port[ port ];

#if defined( DEF_KS8841 )  ||  defined( TWO_NETWORK_INTERFACE )
#if 0
    priv->stats.rx_bytes = ( unsigned long )( pPort->cnCounter[
        MIB_COUNTER_RX_LO_PRIORITY ] + pPort->cnCounter[
        MIB_COUNTER_RX_HI_PRIORITY ]);
    priv->stats.rx_packets = ( unsigned long )( pPort->cnCounter[
        MIB_COUNTER_RX_BROADCAST ] + pPort->cnCounter[
        MIB_COUNTER_RX_MULTICAST ] + pPort->cnCounter[
        MIB_COUNTER_RX_UNICAST ]);
    priv->stats.tx_bytes = ( unsigned long )( pPort->cnCounter[
        MIB_COUNTER_TX_LO_PRIORITY ] + pPort->cnCounter[
        MIB_COUNTER_TX_HI_PRIORITY ]);
    priv->stats.tx_packets = ( unsigned long )( pPort->cnCounter[
        MIB_COUNTER_TX_BROADCAST ] + pPort->cnCounter[
        MIB_COUNTER_TX_MULTICAST ] + pPort->cnCounter[
        MIB_COUNTER_TX_UNICAST ]);
#endif
#endif

    priv->stats.rx_errors = pHardware->m_cnCounter[ port ]
        [ OID_COUNTER_RCV_ERROR ];
    priv->stats.tx_errors = pHardware->m_cnCounter[ port ]
        [ OID_COUNTER_XMIT_ERROR ];
#if 0
    priv->stats.tx_carrier_errors =
        pHardware->m_cnCounter[ port ][ OID_COUNTER_XMIT_LOST_CARRIER ];
#endif

    priv->stats.multicast = ( unsigned long ) pPort->cnCounter[
        MIB_COUNTER_RX_MULTICAST ];
        
    priv->stats.collisions = ( unsigned long ) pPort->cnCounter[
        MIB_COUNTER_TX_TOTAL_COLLISION ];

    priv->stats.rx_length_errors = ( unsigned long )( pPort->cnCounter[
        MIB_COUNTER_RX_UNDERSIZE ] + pPort->cnCounter[
        MIB_COUNTER_RX_FRAGMENT ] + pPort->cnCounter[
        MIB_COUNTER_RX_OVERSIZE ] + pPort->cnCounter[
        MIB_COUNTER_RX_JABBER ]);
    priv->stats.rx_crc_errors = ( unsigned long ) pPort->cnCounter[
        MIB_COUNTER_RX_CRC_ERR ];
    priv->stats.rx_frame_errors = ( unsigned long ) pPort->cnCounter[
        MIB_COUNTER_RX_ALIGNMENT_ERR ];
        
    priv->stats.tx_window_errors = ( unsigned long ) pPort->cnCounter[
        MIB_COUNTER_TX_LATE_COLLISION ];

    return( &priv->stats );
}  /* dev_query_statistics */


/*
    device_set_mac_address

    Description:
        This function is used to set the MAC address of the network device.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        void* addr
            Buffer of MAC address.

    Return (int):
        Zero to indicate success.
*/

static int device_set_mac_address (
    struct net_device* dev,
    void*              addr )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    struct sockaddr* mac = addr;
    int              rc;
    BYTE             InterruptMask;

#ifdef DBG
    printk( "set mac address\n" );
#endif
//ACT_DEBUG_
    printk( "set mac address\n" );

    memcpy( dev->dev_addr, mac->sa_data, MAX_ADDR_LEN );

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return( rc );
    }

    pHardware->m_bMacOverrideAddr = TRUE;
    memcpy( pHardware->m_bOverrideAddress, mac->sa_data, MAC_ADDRESS_LENGTH );

    InterruptMask = HardwareBlockInterrupt( pHardware );
    HardwareSetAddress( pHardware );
    HardwareSetInterrupt( pHardware, InterruptMask );
    ReleaseHardware( hw_priv, FALSE );
    return 0;
}  /* device_set_mac_address */


/*
    dev_set_multicast_list

    Description:
        This routine is used to set multicast addresses or put the network
        device into promiscuous mode.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        void* addr
            Buffer of MAC address.

    Return (None):
*/

static void dev_set_multicast_list (
    struct net_device* dev )
{
    struct dev_priv*    priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info*    hw_priv = priv->pDevInfo;
    PHARDWARE           pHardware = &hw_priv->hw;
    struct dev_mc_list* mc_ptr;
    int                 rc;


	if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return;
    }

    /* Turn on/off promiscuous mode. */
    HardwareSetPromiscuous( pHardware,
        ( BYTE )(( dev->flags & IFF_PROMISC ) == IFF_PROMISC ));

    /* Turn on/off all multicast mode. */
    HardwareSetMulticast( pHardware,
        ( BYTE )(( dev->flags & IFF_ALLMULTI ) == IFF_ALLMULTI ));

    if ( ( dev->flags & IFF_MULTICAST )  &&  dev->mc_count ) {
        if ( dev->mc_count <= MAX_MULTICAST_LIST ) {
            int i = 0;

            for ( mc_ptr = dev->mc_list; mc_ptr; mc_ptr = mc_ptr->next ) {
                if ( !( *mc_ptr->dmi_addr & 1 ) )
                    continue;
                if ( i >= MAX_MULTICAST_LIST )
                    break;
                memcpy( pHardware->m_bMulticastList[ i++ ], mc_ptr->dmi_addr,
                    MAC_ADDRESS_LENGTH );
            }
            pHardware->m_bMulticastListSize = ( BYTE ) i;
            HardwareSetGroupAddress( pHardware );
        }

        /* List too big to support so turn on all multicast mode. */
        else {
            pHardware->m_bMulticastListSize = 255;
            HardwareSetMulticast( pHardware, TRUE );
        }
    }
    else {
        pHardware->m_bMulticastListSize = 0;
        HardwareClearMulticast( pHardware );
    }

    ReleaseHardware( hw_priv, FALSE );

}  /* dev_set_multicast_list */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
/*
    mdio_read

    Description:
        This function returns the PHY register value.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        int phy_id
            The PHY id.

        int reg_num
            The register number.

    Return (int):
        The register value.
*/

static int mdio_read (
    struct net_device *dev,
    int               phy_id,
    int               reg_num )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              port = MAIN_PORT;
    u16              val_out;

    if ( phy_id <= OTHER_PORT )
        port = phy_id;
    HardwareReadPhy( pHardware, port, reg_num << 1, &val_out );

    return val_out;
}  /* mdio_read */


/*
    mdio_write

    Description:
        This procedure sets the PHY register value.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        int phy_id
            The PHY id.

        int reg_num
            The register number.

        int val
            The register value.

    Return (None):
*/

static void mdio_write (
    struct net_device *dev,
    int               phy_id,
    int               reg_num,
    int               val )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              port = MAIN_PORT;

    if ( phy_id <= OTHER_PORT )
        port = phy_id;
    HardwareWritePhy( pHardware, port, reg_num << 1, val );
}  /* mdio_write */


#include "device_ethtool.c"
#endif


/*
    netdev_ioctl

    Description:
        This function is used to process I/O control calls.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ifreq* ifr
            Pointer to interface request structure.

        int cmd
            I/O control code.

    Return (int):
        Zero to indicate success.
*/

#define SIOCDEVDEBUG  ( SIOCDEVPRIVATE + 10 )

static int netdev_ioctl (
    struct net_device* dev,
    struct ifreq*      ifr,
    int                cmd )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTRequest        req = ( PTRequest ) ifr->ifr_data;
    int              nResult;
    int              rc;
    int              result = 0;
    int              port = MAIN_PORT;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    struct mii_ioctl_data *data = ( struct mii_ioctl_data* ) &ifr->ifr_data;

#else
    struct mii_ioctl_data *data = if_mii( ifr );
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( down_interruptible( &hw_priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }
    // assume success
    nResult = DEV_IOC_OK;
    switch ( cmd ) {
    	case SIOCGMIIPHY:               /* Get address of MII PHY in use. */
        case SIOCDEVPRIVATE:            /* for binary compat, remove in 2.5 */

            data->phy_id = 0;
            /* Fallthrough... */

	case SIOCGMIIREG:               /* Read MII PHY register. */
        case SIOCDEVPRIVATE+1:          /* for binary compat, remove in 2.5 */

            if ( data->phy_id != 0  ||
                    data->reg_num >= 6 )
	     {
                result = -EIO;
	     } else {

                HardwareReadPhy( pHardware, port, data->reg_num << 1,
                    &data->val_out );
            }
            break;

        case SIOCSMIIREG:               /* Write MII PHY register. */
        case SIOCDEVPRIVATE+2:          /* for binary compat, remove in 2.5 */
            if ( !capable( CAP_NET_ADMIN ) )
	    {
                result = -EPERM;
	    }
            else if ( data->phy_id != 0  ||
                    data->reg_num >= 6 )
		   {
                     result = -EIO;
		   }  else {

                HardwareWritePhy( pHardware, port, data->reg_num << 1,
                    data->val_in );
            }
            break;

        case SIOCDEVDEBUG:
            if ( req ) {
                switch ( req->nCmd ) {
                    case DEV_CMD_INIT:
                        req->param.bData[ 0 ] = 'M';
                        req->param.bData[ 1 ] = 'i';
                        req->param.bData[ 2 ] = 'c';
                        req->param.bData[ 3 ] = 'r';
                        req->nSize = 8 + 4;
                        break;
                    case DEV_CMD_GET:
                        switch ( req->nSubCmd ) {
                            case DEV_READ_REG:
                                hwread( req->param.nData[ 0 ],
                                    req->param.nData[ 1 ],
                                    req->param.nData[ 2 ]);
                                break;
                            case DEV_LINK_STATUS:
                                HardwareGetLinkStatus( pHardware, 0,
                                    &req->param.LinkStatus );
                                break;
                        }
                        break;
                    case DEV_CMD_PUT:
                        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
                            result = -EAGAIN;
                            break;
                        }
                        switch ( req->nSubCmd ) {
                            case DEV_CAPABILITIES:
                                HardwareSetCapabilities( pHardware, 0,
                                    req->param.nData[ 0 ]);
                                break;
                            case DEV_CABLE_STATUS:
                                HardwareGetCableStatus( pHardware, 0,
                                    req->param.CableStatus );
                                break;
                        }
                        ReleaseHardware( hw_priv, FALSE );
                        break;
                    default:
                        nResult = DEV_IOC_INVALID_CMD;
                        break;
                }
            }
            if ( req ) {
                req->nResult = nResult;
            }
            else if ( !result )
                result = -EFAULT;
            break;
        default:
            result = -EOPNOTSUPP;
    }

    up( &hw_priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif
    return( result );
}  /* netdev_ioctl */


/*
    dev_monitor

    Description:
        This routine is run in a kernel timer to monitor the network device.

    Parameters:
        unsigned long ptr
            Pointer value to supplied data.

    Return (None):
*/

static void dev_monitor (
    unsigned long ptr )
{
    struct net_device* dev = ( struct net_device* ) ptr;
    struct dev_priv*   priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;
    PTTimerInfo        pInfo = &hw_priv->MonitorTimerInfo;
    int                port;
    int                rc;

    if ( !( rc = AcquireHardware( hw_priv, TRUE, FALSE )) ) {

/* PHY change interrupt is working for KS8841. */
        SwitchGetLinkStatus( pHardware );
#if 0 /* Polling mode */
        ks8842i_dev_interrupt(dev->irq, dev);
#endif

        for ( port = 0; port < TOTAL_PORT_NUM; port++ ) {

#ifdef DEF_KS8841
            /* KS8841 does not have second port. */
            if ( OTHER_PORT == port )
                continue;
#endif

            /* Reading MIB counters or requested to read. */
            if ( pHardware->m_Port[ port ].bCurrentCounter  ||
                    hw_priv->Counter[ port ].fRead ) {

                /* Need to process receive interrupt. */
                if ( PortReadCounters( pHardware, port ) )
                    goto dev_monitor_release;
                hw_priv->Counter[ port ].fRead = 0;

                /* Finish reading counters. */
                if ( 0 == pHardware->m_Port[ port ].bCurrentCounter ) {
                    wake_up_interruptible(
                        &hw_priv->Counter[ port ].wqhCounter );
                }
            }
            else if ( jiffies >= hw_priv->Counter[ port ].time ) {
                hw_priv->Counter[ port ].fRead = 1;
                if ( port == MAIN_PORT )
                    hw_priv->Counter[ MAIN_PORT ].time = jiffies + HZ * 6;
                else
                    hw_priv->Counter[ port ].time =

                        hw_priv->Counter[ MAIN_PORT ].time + HZ * 2;
            }
        }

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
        mach_led( led_count, 0 );
        led_count++;
        led_count &= 0xF;
        mach_led( led_count, 1 );
#endif

#ifdef STP_SUPPORT
#ifdef DBG
        if ( dev->br_port  &&
                dev->br_port->state != pHardware->m_PortInfo[ 0 ].nSTP_State ) {
printk( "STP state: %d\n", dev->br_port->state );
pHardware->m_PortInfo[ 0 ].nSTP_State = dev->br_port->state;
        }
#endif
#endif
        if ( priv->opened  &&
                priv->media_state != pHardware->m_ulHardwareState ) {
            if ( MediaStateConnected == pHardware->m_ulHardwareState )
                netif_carrier_on( dev );
            else
                netif_carrier_off( dev );
            priv->media_state = pHardware->m_ulHardwareState;
            if ( netif_msg_link( priv ) ) {
                printk(KERN_INFO "%s link %s\n", dev->name,
                    ( MediaStateConnected == priv->media_state ?
                    "on" : "off" ));
            }
        }

dev_monitor_release:
        ReleaseHardware( hw_priv, TRUE );
    }

#ifdef DBG
    if ( strMsg != strDebug ) {
        if ( fLastDebugLine )
            printk( "%c", DBG_CH );
        fLastDebugLine = 0;
        if ( '\n' == strDebug[ cnDebug - 2 ]  &&
                DBG_CH == strDebug[ cnDebug - 1 ] ) {
            strDebug[ cnDebug - 1 ] = '\0';
            fLastDebugLine = 1;
        }
        strMsg = strDebug;
        cnDebug = 0;
        printk( strDebug );
    }
    if ( strIntMsg != strIntBuf ) {
        printk( "---\n" );
        if ( fLastIntBufLine )
            printk( "%c", DBG_CH );
        fLastIntBufLine = 0;
        if ( '\n' == strIntBuf[ cnIntBuf - 2 ]  &&
                DBG_CH == strIntBuf[ cnIntBuf - 1 ] ) {
            strIntBuf[ cnIntBuf - 1 ] = '\0';
            fLastIntBufLine = 1;
        }
        strIntMsg = strIntBuf;
        cnIntBuf = 0;
        printk( strIntBuf );
    }
#endif

    ++pInfo->cnCount;
    if ( pInfo->nCount > 0 ) {
        if ( pInfo->cnCount < pInfo->nCount ) {
            pInfo->pTimer->expires = jiffies + pInfo->nTime;
            add_timer( pInfo->pTimer );
        }
        else
            pInfo->nCount = 0;
    }
    else if ( pInfo->nCount < 0 ) {
        pInfo->pTimer->expires = jiffies + pInfo->nTime;
        add_timer( pInfo->pTimer );
    }

}  /* dev_monitor */


#ifdef SH_BUS_TEST
void test_memory_copy (
    void* dest,
    void* src,
    int   len,
    char* message )
{
    unsigned long start;
    unsigned long stop;
    int           j;

    start = jiffies;
    for ( j = 0; j < 10000; j++ ) {
        memcpy( dest, src, len );
    }
    stop = jiffies;
    printk( "%s: ", message );
    printk( "%lu %lu Mbps\n", stop - start, 10000 * len * 8 /
        (( stop - start ) * ( 1000 / HZ )) / 1000 );
}  /* test_memory_copy */


void test_data_read (
    PHARDWARE pHardware,
    void*     dest,
    char*     message )
{
    unsigned long start;
    unsigned long stop;
    int           j;
    PULONG        pdwData;

    pdwData = ( PULONG ) dest;
    start = jiffies;
    for ( j = 0; j < 500; j++ ) {

        /* Setup read address pointer. */
        HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
            REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );

        HW_READ_BUFFER_DWORD( pHardware, REG_DATA_OFFSET, pdwData, 1024 );
    }
    stop = jiffies;
    printk( "%s: ", message );
    printk( "%lu %lu Mbps\n", stop - start, 500 * 1024 * 8 /
        (( stop - start ) * ( 1000 / HZ )) / 1000 );
}  /* test_data_read */


void test_data_write (
    PHARDWARE pHardware,
    void*     dest,
    void*     src,
    char*     message )
{
    unsigned long start;
    unsigned long stop;
    int           j;
    PULONG        pdwData;

    pdwData = ( PULONG ) dest;
    start = jiffies;
    for ( j = 0; j < 500; j++ ) {
        memcpy( dest, src, 1024 );

        /* Setup read address pointer. */
        HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
            REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );

        HW_WRITE_BUFFER_DWORD( pHardware, REG_DATA_OFFSET, pdwData, 1024 );
    }
    stop = jiffies;
    printk( "%s: ", message );
    printk( "%lu %lu Mbps\n", stop - start, 500 * 1024 * 8 /
        (( stop - start ) * ( 1000 / HZ )) / 1000 );
}  /* test_data_write */


void test_data_read_word (
    PHARDWARE pHardware,
    void*     dest,
    char*     message )
{
    unsigned long start;
    unsigned long stop;
    int           j;
    PULONG        pdwData;

    pdwData = ( PULONG ) dest;
    start = jiffies;
    for ( j = 0; j < 500; j++ ) {

        /* Setup read address pointer. */
        HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
            REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );

        HW_READ_BUFFER_WORD( pHardware, REG_DATA_OFFSET, pdwData, 1024 );
    }
    stop = jiffies;
    printk( "%s: ", message );
    printk( "%lu %lu Mbps\n", stop - start, 500 * 1024 * 8 /
        (( stop - start ) * ( 1000 / HZ )) / 1000 );
}  /* test_data_read_word */


void test_data_write_word (
    PHARDWARE pHardware,
    void*     dest,
    void*     src,
    char*     message )
{
    unsigned long start;
    unsigned long stop;
    int           j;
    PULONG        pdwData;

    pdwData = ( PULONG ) dest;
    start = jiffies;
    for ( j = 0; j < 500; j++ ) {
        memcpy( dest, src, 1024 );

        /* Setup read address pointer. */
        HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
            REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );

        HW_WRITE_BUFFER_WORD( pHardware, REG_DATA_OFFSET, pdwData, 1024 );
    }
    stop = jiffies;
    printk( "%s: ", message );
    printk( "%lu %lu Mbps\n", stop - start, 500 * 1024 * 8 /
        (( stop - start ) * ( 1000 / HZ )) / 1000 );
}  /* test_data_write_word */


void test_data_bus (
    PHARDWARE pHardware )
{
    DWORD*  dwData;
    PUCHAR  pbData;
    PUSHORT pwData;

    dwData = kmalloc( 2048, GFP_KERNEL | GFP_DMA );
    pbData = ( PUCHAR ) dwData;
    pwData = ( PUSHORT ) dwData;

    test_memory_copy( pwData, pbData + 1024, 1024, "dword-aligned copy" );
    test_memory_copy( pwData, pbData + 1022, 1024, " word-aligned copy" );
    test_memory_copy( pwData, pbData + 1021, 1024, " byte-aligned copy" );

    test_data_read( pHardware, pbData + 1024,
        "dword-aligned read buffer dword" );
    test_data_write( pHardware, pwData, pbData + 1024,
        "dword-aligned write buffer dword with copy" );

    test_data_read( pHardware, pbData + 1022,
        " word-aligned read buffer dword" );
    test_data_write( pHardware, pwData, pbData + 1022,
        " word-aligned write buffer dword with copy" );

    test_data_read( pHardware, pbData + 1021,
        " byte-aligned read buffer dword" );
    test_data_write( pHardware, pwData, pbData + 1021,
        " byte-aligned write buffer dword with copy" );

    test_data_read_word( pHardware, pbData + 1022,
        " word-aligned read buffer word" );
    test_data_write_word( pHardware, pwData, pbData + 1022,
        " word-aligned write buffer word with copy" );

    test_data_read_word( pHardware, pbData + 1021,
        " byte-aligned read buffer word" );
    test_data_write_word( pHardware, pwData, pbData + 1021,
        " byte-aligned write buffer word with copy" );

    kfree( dwData );
}  /* test_data_bus */
#endif

static struct net_device* last_dev = NULL;
static struct hw_fn* ks8842_fn = NULL;
static int device_present = 0;


static void init_proc (
    struct dev_info* priv )
{
    int  i;
    int  port;

    int  port_count = 1;
    char proc_name[ 40 ];

    sprintf( proc_name, "%s-%d", proc_dir_name, priv->id );
    sema_init( &priv->proc_sem, 1 );
    priv->proc_main = proc_mkdir( proc_name, NULL );
    if ( priv->proc_main ) {
//        priv->proc_main->owner = THIS_MODULE;
        memset( ProcPortInfo, 0, sizeof( TProcPortInfo ) * 3 * PROC_LAST );
        port = 2;

        /* Read-only entries. */
        for ( i = 0; i < PROC_SET_DUPLEX; i++ ) {
            ProcPortInfo[ port ][ i ].priv = priv;
            ProcPortInfo[ port ][ i ].info = &ProcInfo[ i ];
            ProcPortInfo[ port ][ i ].port = port;
            proc_info = create_proc_read_entry( ProcInfo[ i ].proc_name,
                0444, priv->proc_main, read_proc, &ProcPortInfo[ port ][ i ]);
            if ( proc_info ) {
              //  proc_info->owner = THIS_MODULE;
            }
        }

        /* Can be written to. */
        for ( i = PROC_SET_DUPLEX; i < PROC_GET_CABLE_STATUS;
                i++ ) {
            ProcPortInfo[ port ][ i ].priv = priv;
            ProcPortInfo[ port ][ i ].info = &ProcInfo[ i ];
            ProcPortInfo[ port ][ i ].port = port;
            proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
                priv->proc_main );
            if ( proc_set ) {
                proc_set->data = &ProcPortInfo[ port ][ i ];
                proc_set->read_proc = read_proc;
                proc_set->write_proc = write_proc;
            //    proc_set->owner = THIS_MODULE;
            }
        }
        for ( i = PROC_SET_RX_P0_RATE; i <= PROC_SET_TX_P3_RATE; i++ ) {
            ProcPortInfo[ port ][ i ].priv = priv;
            ProcPortInfo[ port ][ i ].info = &ProcInfo[ i ];
            ProcPortInfo[ port ][ i ].port = port;
            proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
                priv->proc_main );
            if ( proc_set ) {
                proc_set->data = &ProcPortInfo[ port ][ i ];
                proc_set->read_proc = read_proc;
                proc_set->write_proc = write_proc;
              //  proc_set->owner = THIS_MODULE;
            }
        }
        for ( port = 0; port < port_count; port++ ) {
            sprintf( proc_name, "%d", port );
            priv->proc_port[ port ] = proc_mkdir( proc_name, priv->proc_main );
        //    priv->proc_port[ port ]->owner = THIS_MODULE;
            for ( i = PROC_GET_CABLE_STATUS; i < PROC_ENABLE_BROADCAST_STORM;
                    i++ ) {
                ProcPortInfo[ port ][ i ].priv = priv;
                ProcPortInfo[ port ][ i ].info = &ProcInfo[ i ];
                ProcPortInfo[ port ][ i ].port = port;
                proc_info = create_proc_read_entry( ProcInfo[ i ].proc_name,
                    0444, priv->proc_port[ port ], read_proc,
                    &ProcPortInfo[ port ][ i ]);
                if ( proc_info ) {
             //       proc_info->owner = THIS_MODULE;
                }
            }
            for ( i = PROC_ENABLE_BROADCAST_STORM; i < PROC_LAST; i++ ) {
                ProcPortInfo[ port ][ i ].priv = priv;
                ProcPortInfo[ port ][ i ].info = &ProcInfo[ i ];
                ProcPortInfo[ port ][ i ].port = port;
                proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
                    priv->proc_port[ port ]);
                if ( proc_set ) {
                    proc_set->data = &ProcPortInfo[ port ][ i ];
                    proc_set->read_proc = read_proc;
                    proc_set->write_proc = write_proc;
               //     proc_set->owner = THIS_MODULE;
                }
            }
        }
    }
}  /* init_proc */


#if !LINUX_2_6_27_29
static const struct ethtool_ops ks8842i_ethtool_ops = {
	.get_settings		= netdev_get_settings,
	.set_settings		= netdev_set_settings,
	.get_drvinfo		= netdev_get_drvinfo,
	.get_link		= netdev_get_link,
};

static const struct net_device_ops ks8842i_netdev_ops = {
	.ndo_open		= netdev_open,
	.ndo_stop		= netdev_close,
	.ndo_get_stats		= dev_query_statistics,
	.ndo_start_xmit		= DEV_TRANSMIT,
	.ndo_do_ioctl		= netdev_ioctl,
	.ndo_set_multicast_list =dev_set_multicast_list,
	.ndo_set_mac_address	= device_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_tx_timeout	=	DEV_TRANSMIT_TIMEOUT,
};
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 *  * Polling receive - used by netconsole and other diagnostic tools
 *   * to allow network i/o with interrupts disabled.
 *    */
static void netdev_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
printk("polling\n");
	ks8842i_dev_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

static unsigned int ifport= 0;

/*
    netdev_init

    Description:
        This function initializes the network device.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int __init ks8841_probe(
    struct platform_device *pdev )
{
	struct net_device *dev;
	struct dev_priv* priv;
	struct dev_info* hw_priv = NULL;
	int              rc = -1;
	int err;
	struct resource  *res;

	dev = alloc_etherdev( sizeof( struct dev_priv ));
	if (!dev) {
		err = -ENOMEM;
		return err;
	}

	platform_set_drvdata(pdev, dev);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
		return -EINVAL;

        /* Copy the parameters from insmod into the device structure. */
       dev->base_addr = res->start;
	
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!res)
		return -EINVAL;

       dev->irq       = res->start;
       dev->if_port   = ifport;
#if !LINUX_2_6_27_29
	dev->netdev_ops = &ks8842i_netdev_ops;
#endif
	
    /* This is a real device. */
    if ( !last_dev ) {

        printk(KERN_INFO "%s\n", version );
        ks8842_fn = kmalloc( sizeof( struct hw_fn ), GFP_KERNEL );
        if ( ks8842_fn == NULL ) {
			printk("$$$$2Device is not found.\n");
            return -ENOMEM;
        }
        HardwareSetupFunc( ks8842_fn );
    }

    /* Allocate private data. */
    priv = ( struct dev_priv* ) netdev_priv(dev);
    if ( priv == NULL ) {
        if ( !last_dev ) {
	    release_region( dev->base_addr, BASE_IO_RANGE );
        }
        return -ENOMEM;
    }
    memset( priv, 0, sizeof( struct dev_priv ));
    spin_lock_init( &priv->lock );

    if ( !last_dev ) {
        priv->pDevInfo = &priv->dev_info;
        hw_priv = priv->pDevInfo;

        hw_priv->hw.m_ulVIoAddr = dev->base_addr;
        hw_priv->hw.m_pDevice = dev;

        InitHardware( hw_priv );

        hw_priv->hw.m_hwfn = ks8842_fn;

        hw_priv->MonitorTimerInfo.pTimer = &hw_priv->timerMonitor;
        hw_priv->MonitorTimerInfo.nCount = 0;

        /* 500 ms timeout */
        hw_priv->MonitorTimerInfo.nTime = 500 * HZ / 1000;
        init_timer( &hw_priv->timerMonitor );
        hw_priv->timerMonitor.function = dev_monitor;
        hw_priv->timerMonitor.data = ( unsigned long ) dev;

        init_waitqueue_head( &hw_priv->Counter[ MAIN_PORT ].wqhCounter );
        init_waitqueue_head( &hw_priv->Counter[ OTHER_PORT ].wqhCounter );
        init_waitqueue_head( &hw_priv->Counter[ HOST_PORT ].wqhCounter );
    }

    /* Fill in the fields of the device structure with default values. */
    ether_setup( dev );

    dev->open               = netdev_open;
    dev->stop               = netdev_close;
    dev->hard_start_xmit    = ks8842i_dev_transmit;
    dev->tx_timeout         = ks8842i_dev_transmit_timeout;
#ifdef DEVELOP
    /* 500 ms timeout */
    dev->watchdog_timeo     = HZ / 2;

#else
    dev->watchdog_timeo     = HZ / 20;
#endif
    dev->get_stats          = dev_query_statistics;
    dev->set_mac_address    = device_set_mac_address;
    dev->set_multicast_list = dev_set_multicast_list;
    dev->do_ioctl           = netdev_ioctl;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
    dev->ethtool_ops        = &netdev_ethtool_ops;
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
    dev->poll_controller    = netdev_poll_controller;
#endif

//    SET_MODULE_OWNER( dev );

    /* This is a real device. */
    if ( !last_dev ) {

#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
        mach_led( 0xFF, 0 );
#endif
        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
            return( rc );
        }
        HardwareReadAddress( &hw_priv->hw );
        ReleaseHardware( hw_priv, FALSE );
        memcpy( dev->dev_addr, hw_priv->hw.m_bPermanentAddress, MAX_ADDR_LEN );


        hw_priv->hw.m_bPort = MAIN_PORT;

        hw_priv->id = device_present;
        init_proc( hw_priv );

        last_dev = dev;

        phw = &hw_priv->hw;
    }

    ++device_present;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
    priv->mii_if.phy_id_mask = 0x1;
    priv->mii_if.reg_num_mask = 0x7;
    priv->mii_if.dev = dev;
    priv->mii_if.mdio_read = mdio_read;
    priv->mii_if.mdio_write = mdio_write;
    priv->mii_if.phy_id = 0;
#endif
    priv->msg_enable = NET_MSG_ENABLE;

#ifdef SH_BUS
    /* m_bLookahead needs to be dword-aligned. */
    ASSERT( !(( int )( hw_priv->hw.m_bLookahead ) & 3 ));
#endif

	/*
	 * Lacking any better mechanism to allocate a MAC address we use a
	 * random one ...
	 */
//	random_ether_addr(dev->dev_addr);

	err = register_netdev(dev);

	if (err) {
		printk(KERN_ERR "ks8841: failed to register netdev.\n");
	}else
		printk( "%s: I/O = %x, IRQ = %d\n", dev->name, (u32)dev->base_addr, dev->irq );
    return err;
}  /* netdev_init */

static int __devexit ks8841_device_remove(struct platform_device *device)
{
	struct net_device *dev = platform_get_drvdata(device);

	unregister_netdev(dev);
	release_region(dev->base_addr, BASE_IO_RANGE);
	free_netdev(dev);
	platform_set_drvdata(device, NULL);

	return 0;
}

static struct platform_driver ks8841_driver = {
	.probe		= ks8841_probe,
	.remove		= __devexit_p(ks8841_device_remove),
	.driver		= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init ks8841_init(void)
{
	int err;

	err = platform_driver_register(&ks8841_driver);
	if (err)
		printk(KERN_ERR "ks8841 driver registration failed\n");

	return err;;
}

static void __exit ks8841_exit(void)
{
	platform_driver_unregister(&ks8841_driver);
}

module_init(ks8841_init);
module_exit(ks8841_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KSZ8841 Ethernet driver");
MODULE_AUTHOR("JAMES <james@126.com>,");

//#endif


static unsigned int io    = 0xbe000000;
static unsigned int irq   = IRQ_EXT;
module_param(io, uint, 0);
module_param(irq, uint, 0);
module_param(ifport, uint, 0);
MODULE_PARM_DESC( io, "Device I/O base address" );
MODULE_PARM_DESC( irq, "Devie IRQ number" );
MODULE_PARM_DESC( ifport, "Device interface port (0-default, 1-TP, 2-AUI)" );


