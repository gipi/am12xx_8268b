/* ---------------------------------------------------------------------------
          Copyright (c) 2003-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    hardware.h - Target independent hardware header

    Author  Date      Version  Description
    THa     06/07/06           Hardware interrupts may not be disabled
                               immediately after the register is written to.
    THa     04/06/06           Implement AT93C46 EEPROM access functions.
    THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                               some hardware platforms.
    THa     01/25/06           Add HardwareWriteIntMask and
                               HardwareWriteIntStat functions for 32-bit I/O
                               access only.
    THa     01/13/06           Transfer dword-aligned data for performance.
    THa     10/06/05           Changed descriptor structure.
    THa     08/15/05           Added PCI configuration I/O.
    PCD     03/30/05  0.1.0    First release.
                               (1). CLI read\write device registers works.
                               (2). Driver Initialization device works .
                               (3). Driver Interrupt Server Routine works.
                               (4). Driver transmit packets to device port works.
                               (5). Driver receive packsts from device works.
    THa     02/23/04           Use inline functions to improve performance.
    THa     10/05/04           Updated for PCI version.
    THa     10/14/04           Updated with latest specs.
    THa     12/10/03  0.0.1    Created file.
   ---------------------------------------------------------------------------
*/


#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "target.h"

#ifdef DBG
#ifndef UNDER_CE
#define DEBUG_COUNTER
#define DEBUG_TIMEOUT
#endif

#if 0
#define DEBUG_INTERRUPT
#endif
#if 0
#define DEBUG_MEM
#endif
#if 0
#define DEBUG_RX
#endif
#if 0
#define DEBUG_RX_DATA
#endif
#if 0
#define DEBUG_TX
#endif
#if 0
#define DEBUG_RX_DESC
#endif
#if 0
#define DEBUG_RX_DESC_CHECK
#endif
#if 0
#define DEBUG_TX_DESC
#endif
#if 0
#define DEBUG_TX_DESC_CHECK
#endif
#endif

#if 0
#define INLINE
#endif


/* define it if software support STP protocol */
#undef SOFTWARE_STP_SUPPORT


/* -------------------------------------------------------------------------- */

#define MAC_ADDRESS_LENGTH  6

/* -------------------------------------------------------------------------- */

#ifdef NDIS_MINIPORT_DRIVER
#if defined( NDIS50_MINIPORT )  ||  defined( NDIS51_MINIPORT )
#include <pshpack1.h>

#else
#include <packon.h>
#endif
#endif

#include "ks_def.h"

/* define max switch port */
#ifdef DEF_KS8842
#define MAX_SWITCH_PORT   2
#else
#define MAX_SWITCH_PORT   1
#endif

#define MAX_ETHERNET_BODY_SIZE  1500
#define ETHERNET_HEADER_SIZE    14

#define MAXIMUM_ETHERNET_PACKET_SIZE  \
    ( MAX_ETHERNET_BODY_SIZE + ETHERNET_HEADER_SIZE )

#define MAX_BUF_SIZE            2048

#define TX_BUF_SIZE             2040
#define RX_BUF_SIZE             1600

#define NDIS_MAX_LOOKAHEAD      ( RX_BUF_SIZE - ETHERNET_HEADER_SIZE )

#define MAX_MULTICAST_LIST      32

#define HW_MULTICAST_SIZE       8


#define MAC_ADDR_ORDER( i )  ( MAC_ADDRESS_LENGTH - 1 - ( i ))


#define MAIN_PORT   0
#define OTHER_PORT  1
#define HOST_PORT   2

#define PORT_1      1
#define PORT_2      2

#define DEV_TO_HW_PORT( port )  ( port + 1 )
#define HW_TO_DEV_PORT( port )  ( port - 1 )

/* Driver set Switch broadcast storm protection at 10% rate */
#define BROADCAST_STORM_PROTECTION_RATE    10   


typedef enum
{
    MediaStateConnected,
    MediaStateDisconnected
} MEDIA_STATE;


typedef enum
{
    OID_COUNTER_UNKOWN,

    OID_COUNTER_FIRST,
    OID_COUNTER_DIRECTED_BYTES_XMIT = OID_COUNTER_FIRST, /* total bytes transmitted  */
    OID_COUNTER_DIRECTED_FRAMES_XMIT,    /* total packets transmitted */

    OID_COUNTER_BROADCAST_BYTES_XMIT,
    OID_COUNTER_BROADCAST_FRAME_XMIT,

    OID_COUNTER_DIRECTED_BYTES_RCV,      /* total bytes received   */
    OID_COUNTER_DIRECTED_FRAMES_RCV,     /* total packets received */
    OID_COUNTER_BROADCAST_BYTES_RCV,
    OID_COUNTER_BROADCAST_FRAMES_RCV,    /* total broadcast packets received (RXSR: RXBF)                */
    OID_COUNTER_MULTICAST_FRAMES_RCV,    /* total multicast packets received (RXSR: RXMF) or (RDSE0: MF) */
    OID_COUNTER_UNICAST_FRAMES_RCV,      /* total unicast packets received   (RXSR: RXUF)                */

    OID_COUNTER_XMIT_ERROR,              /* total transmit errors */
    OID_COUNTER_XMIT_LATE_COLLISION,     /* transmit Late Collision (TXSR: TXLC) */
    OID_COUNTER_XMIT_MORE_COLLISIONS,    /* transmit Maximum Collision (TXSR: TXMC) */
    OID_COUNTER_XMIT_UNDERRUN,           /* transmit Underrun (TXSR: TXUR) */
    OID_COUNTER_XMIT_ALLOC_FAIL,         /* transmit fail because no enought memory in the Tx Packet Memory */
    OID_COUNTER_XMIT_DROPPED,            /* transmit packet drop because no buffer in the host memory */
    OID_COUNTER_XMIT_INT_UNDERRUN,       /* transmit underrun from interrupt status (ISR: TXUIS) */
    OID_COUNTER_XMIT_INT_STOP,           /* transmit DMA MAC process stop from interrupt status (ISR: TXPSIE) */
    OID_COUNTER_XMIT_INT,                /* transmit Tx interrupt status (ISR: TXIE) */

    OID_COUNTER_RCV_ERROR,               /* total receive errors */
    OID_COUNTER_RCV_ERROR_CRC,           /* receive packet with CRC error (RXSR: RXCE) or (RDSE0: CE) */
    OID_COUNTER_RCV_ERROR_MII,           /* receive MII error (RXSR: RXMR) or (RDSE0: RE) */
    OID_COUNTER_RCV_ERROR_TOOLONG,       /* receive frame too long error (RXSR: RXTL) or (RDSE0: TL)  */
    OID_COUNTER_RCV_ERROR_RUNT,          /* receive Runt frame error (RXSR: RXRF) or (RDSE0: RF)  */
    OID_COUNTER_RCV_INVALID_FRAME,       /* receive invalid frame (RXSR: RXFV) */
    OID_COUNTER_RCV_ERROR_IP,            /* receive frame with IP checksum error  (RDSE0: IPE) */
    OID_COUNTER_RCV_ERROR_TCP,           /* receive frame with TCP checksum error (RDSE0: TCPE) */
    OID_COUNTER_RCV_ERROR_UDP,           /* receive frame with UDP checksum error (RDSE0: UDPE) */
    OID_COUNTER_RCV_NO_BUFFER,           /* receive failed on memory allocation for the incoming frames from interrupt status (ISR: RXOIS). */
    OID_COUNTER_RCV_DROPPED,             /* receive packet drop because no buffer in the host memory */
    OID_COUNTER_RCV_INT_ERROR,           /* receive error from interrupt status (ISR: RXEFIE) */
    OID_COUNTER_RCV_INT_STOP,            /* receive DMA MAC process stop from interrupt status (ISR: RXPSIE) */
    OID_COUNTER_RCV_INT,                 /* receive Rx interrupt status (ISR: RXIE) */

    OID_COUNTER_XMIT_OK,
    OID_COUNTER_RCV_OK,

    OID_COUNTER_RCV_ERROR_LEN,

    OID_COUNTER_LAST
} EOidCounter;


enum
{
    COUNT_BAD_FIRST,
    COUNT_BAD_ALLOC = COUNT_BAD_FIRST,
    COUNT_BAD_CMD,
    COUNT_BAD_CMD_BUSY,
    COUNT_BAD_CMD_INITIALIZE,
    COUNT_BAD_CMD_MEM_ALLOC,
    COUNT_BAD_CMD_RESET,
    COUNT_BAD_CMD_WRONG_CHIP,
    COUNT_BAD_COPY_DOWN,
    COUNT_BAD_RCV_FRAME,
    COUNT_BAD_RCV_PACKET,
    COUNT_BAD_SEND,
    COUNT_BAD_SEND_DIFF,
    COUNT_BAD_SEND_PACKET,
    COUNT_BAD_SEND_ZERO,
    COUNT_BAD_XFER_ZERO,
    COUNT_BAD_LAST
};


enum
{
    COUNT_GOOD_FIRST,
    COUNT_GOOD_CMD_RESET = COUNT_GOOD_FIRST,
    COUNT_GOOD_CMD_RESET_MMU,
    COUNT_GOOD_COPY_DOWN_ODD,
    COUNT_GOOD_INT,
    COUNT_GOOD_INT_LOOP,
    COUNT_GOOD_INT_ALLOC,
    COUNT_GOOD_INT_RX,
    COUNT_GOOD_INT_RX_EARLY,
    COUNT_GOOD_INT_RX_OVERRUN,
    COUNT_GOOD_INT_TX,
    COUNT_GOOD_INT_TX_EMPTY,
    COUNT_GOOD_NEXT_PACKET,
    COUNT_GOOD_NO_NEXT_PACKET,
    COUNT_GOOD_RCV_COMPLETE,
    COUNT_GOOD_RCV_DISCARD,
    COUNT_GOOD_RCV_NOT_DISCARD,
    COUNT_GOOD_SEND_PACKET,
    COUNT_GOOD_SEND_QUEUE,
    COUNT_GOOD_SEND_ZERO,
    COUNT_GOOD_XFER_ZERO,
    COUNT_GOOD_LAST
};


enum
{
    WAIT_DELAY_FIRST,
    WAIT_DELAY_PHY_RESET = WAIT_DELAY_FIRST,
    WAIT_DELAY_AUTO_NEG,
    WAIT_DELAY_MEM_ALLOC,
    WAIT_DELAY_CMD_BUSY,
    WAIT_DELAY_LAST
};


#ifdef KS_PCI_BUS

#if 0
#define CHECK_OVERRUN
#endif
#ifdef DBG
#if 1
#define DEBUG_OVERRUN
#endif
#endif


#define DESC_ALIGNMENT              16
#define BUFFER_ALIGNMENT            8


#define NUM_OF_RX_DESC  128
#define NUM_OF_TX_DESC  32

#define DESC_RX_FRAME_LEN        0x000007FF
#define DESC_RX_FRAME_TYPE       0x00008000
#define DESC_RX_ERROR_CRC        0x00010000
#define DESC_RX_ERROR_RUNT       0x00020000
#define DESC_RX_ERROR_TOOL_LONG  0x00040000
#define DESC_RX_ERROR_PHY        0x00080000
#define DESC_RX_PORT_MASK        0x00300000
#define DESC_RX_MULTICAST        0x01000000
#define DESC_RX_ERROR            0x02000000
#define DESC_RX_ERROR_CSUM_UDP   0x04000000
#define DESC_RX_ERROR_CSUM_TCP   0x08000000
#define DESC_RX_ERROR_CSUM_IP    0x10000000
#define DESC_RX_LAST             0x20000000
#define DESC_RX_FIRST            0x40000000

#define DESC_HW_OWNED            0x80000000

#define DESC_BUF_SIZE            0x000007FF
#define DESC_TX_PORT_MASK        0x00300000
#define DESC_END_OF_RING         0x02000000
#define DESC_TX_CSUM_GEN_UDP     0x04000000
#define DESC_TX_CSUM_GEN_TCP     0x08000000
#define DESC_TX_CSUM_GEN_IP      0x10000000
#define DESC_TX_LAST             0x20000000
#define DESC_TX_FIRST            0x40000000
#define DESC_TX_INTERRUPT        0x80000000

#define DESC_RX_MASK  ( DESC_BUF_SIZE )

#define DESC_TX_MASK  ( DESC_TX_INTERRUPT | DESC_TX_FIRST | DESC_TX_LAST | \
    DESC_BUF_SIZE )


typedef struct
{
    ULONG wFrameLen     : 11;
    ULONG ulReserved1   : 4;
    ULONG fFrameType    : 1;
    ULONG fErrCRC       : 1;
    ULONG fErrRunt      : 1;
    ULONG fErrTooLong   : 1;
    ULONG fErrPHY       : 1;
    ULONG ulSourePort   : 4;
    ULONG fMulticast    : 1;
    ULONG fError        : 1;
    ULONG fCsumErrUDP   : 1;
    ULONG fCsumErrTCP   : 1;
    ULONG fCsumErrIP    : 1;
    ULONG fLastDesc     : 1;
    ULONG fFirstDesc    : 1;
    ULONG fHWOwned      : 1;
} TDescRxStat;

typedef struct
{
    ULONG ulReserved1   : 31;
    ULONG fHWOwned      : 1;
} TDescTxStat;

typedef struct
{
    ULONG wBufSize      : 11;
    ULONG ulReserved3   : 14;
    ULONG fEndOfRing    : 1;
    ULONG ulReserved4   : 6;
} TDescRxBuf;

typedef struct
{
    ULONG wBufSize      : 11;
    ULONG ulReserved3   : 9;
    ULONG ulDestPort    : 4;
    ULONG ulReserved4   : 1;
    ULONG fEndOfRing    : 1;
    ULONG fCsumGenUDP   : 1;
    ULONG fCsumGenTCP   : 1;
    ULONG fCsumGenIP    : 1;
    ULONG fLastSeg      : 1;
    ULONG fFirstSeg     : 1;
    ULONG fInterrupt    : 1;
} TDescTxBuf;

typedef union
{
    TDescRxStat rx;
    TDescTxStat tx;
    ULONG       ulData;
} TDescStat;

typedef union
{
    TDescRxBuf rx;
    TDescTxBuf tx;
    ULONG      ulData;
} TDescBuf;

typedef struct
{
    TDescStat Control;
    TDescBuf  BufSize;
    ULONG     ulBufAddr;
    ULONG     ulNextPtr;
} THw_Desc, *PTHw_Desc;


typedef struct
{
    TDescStat Control;
    TDescBuf  BufSize;

    /* Current buffers size value in hardware descriptor. */
    ULONG     ulBufSize;
} TSw_Desc, *PTSw_Desc;


typedef struct _Desc
{
    /* Hardware descriptor pointer to uncached physical memory. */
    PTHw_Desc     phw;

    /* Cached memory to hold hardware descriptor values for manipulation. */
    TSw_Desc      sw;

    /* Operating system dependent data structure to hold physical memory buffer
       allocation information.
    */
    PVOID         pReserved;

#ifdef CHECK_OVERRUN
    PTHw_Desc     pCheck;
#endif
} TDesc, *PTDesc;


typedef struct
{
    /* First descriptor in the ring. */
    PTDesc    pRing;

    /* Current descriptor being manipulated. */
    PTDesc    pCurrent;

    /* First hardware descriptor in the ring. */
    PTHw_Desc phwRing;

    /* The physical address of the first descriptor of the ring. */
    ULONG     ulRing;

    int       nSize;

    /* Number of descriptors allocated. */
    int       cnAlloc;

    /* Number of descriptors available for use. */
    int       cnAvail;

    /* Index for last descriptor released to hardware .*/
    int       iLast;

    /* Index for next descriptor available for use. */
    int       iNext;

    /* Mask for index wrapping. */
    int       iMax;
} TDescInfo, *PTDescInfo;
#endif


struct hw_fn;

typedef struct
{
    struct hw_fn*           m_hwfn;

    UCHAR                   m_bPermanentAddress[ MAC_ADDRESS_LENGTH ];

    UCHAR                   m_bOverrideAddress[ MAC_ADDRESS_LENGTH ];

    /* PHY status info. */
    ULONG                   m_ulHardwareState;
    ULONG                   m_ulTransmitRate;
    ULONG                   m_ulDuplex;

    /* hardware resources */
    PUCHAR                  m_pVirtualMemory;
    ULONG                   m_ulVIoAddr;             /* device's base address */
    ULONG                   m_boardBusEndianMode;    /* board bus endian mode board specific */

    UCHAR                   m_bMacOverrideAddr;

    UCHAR                   m_bBroadcastPercent;
    USHORT                  m_w802_1P_Mapping;
    USHORT                  m_wDiffServ[ 64 ];      /* possible values from 6-bit of ToS (bit7 ~ bit2) field */
    USHORT                  m_b802_1P_Priority[8];  /* possible values from 3-bit of 802.1p Tag priority field */
    MAC_TABLE               m_MacTable[ STATIC_MAC_TABLE_ENTRIES ];
    PORT_CONFIG             m_Port[ TOTAL_PORT_NUM ];        /* Device switch MIB counters */
    PORT_INFO               m_PortInfo[ SWITCH_PORT_NUM ];
    VLAN_TABLE              m_VlanTable[ VLAN_TABLE_ENTRIES ];

#ifdef KS_PCI_BUS
    ULONG                   m_dwTransmitConfig;
    ULONG                   m_dwReceiveConfig;
    USHORT                  m_wTransmitThreshold;
    USHORT                  m_wReceiveThreshold;
    ULONG                   m_ulInterruptMask;
    ULONG                   m_ulInterruptSet;
    UCHAR                   m_bReceiveStop;

#else
    USHORT                  m_wTransmitConfig;
    USHORT                  m_wTransmitThreshold;
    USHORT                  m_wReceiveConfig;
    USHORT                  m_wReceiveThreshold;
    USHORT                  m_wInterruptMask;
    UCHAR                   m_bBurstLength;
    UCHAR                   m_bReserved1;
#endif
    UCHAR                   m_bEnabled;
    UCHAR                   m_bPromiscuous;
    UCHAR                   m_bAllMulticast;

    /* List of multicast addresses in use. */

    UCHAR                   m_bMulticastListSize;
    UCHAR                   m_bMulticastList[ MAX_MULTICAST_LIST ]
        [ MAC_ADDRESS_LENGTH ];

    /* member variables used for receiving */

#ifdef KS_PCI_BUS
    PUCHAR                  m_bLookahead;

#else
    UCHAR                   m_bLookahead[ MAX_BUF_SIZE ];
#endif
    int                     m_nPacketLen;

    /* member variables used for sending commands, mostly for debug purpose */
    int                     m_nWaitDelay[ WAIT_DELAY_LAST ];

    /* member variables for statistics */
    ULONGLONG               m_cnCounter[ SWITCH_PORT_NUM ][ OID_COUNTER_LAST ];  /* Driver statistics counter */
    ULONG                   m_nBad[ COUNT_BAD_LAST ];
    ULONG                   m_nGood[ COUNT_GOOD_LAST ];

    UCHAR                   m_bBank;
    UCHAR                   m_bReceiveDiscard;
    UCHAR                   m_bSentPacket;
    UCHAR                   m_bTransmitPacket;

    /* hardware configurations read from the registry */
    UCHAR                   m_bDuplex;           /* 10: 10BT; 100: 100BT */
    UCHAR                   m_bSpeed;            /* 1: Full duplex; 2: half duplex */

    USHORT                  m_wPhyAddr;

    UCHAR                   m_bMulticastBits[ HW_MULTICAST_SIZE ];

    UCHAR                   f_dircetMode; /* 1: Tx by direct mode, 0:Tx by loopkup mode */
    UCHAR                   m_bReserved2[ 3 ];

    UCHAR                   m_bPort;
    UCHAR                   m_bPortAlloc;
    UCHAR                   m_bPortRX;    /* 1:Rx from Port1; 2:Rx from Port2 */
    UCHAR                   m_bPortTX;    /* 1:Tx to Port1; 2:Tx to Port2; 3:Tx to Port1 and Port2; 0:Tx by loopkup mode */
    UCHAR                   m_fPortTX;
    UCHAR                   m_bStarted;

    UCHAR                   m_bAcquire;
    UCHAR                   m_bPortSelect;

    /* member variables used for saving registers during interrupt */
    UCHAR                   m_bSavedBank;
    UCHAR                   m_bSavedPacket;
    USHORT                  m_wSavedPointer;

#ifdef KS_PCI_BUS
    TDescInfo               m_RxDescInfo;
    TDescInfo               m_TxDescInfo;

    void*                   m_pPciCfg;

#ifdef DEBUG_OVERRUN
    ULONG                   m_ulDropped;
    ULONG                   m_ulReceived;
#endif
#endif
    void*                   m_pDevice;

#ifdef KS_ISA_BUS
#ifdef UNDER_CE
    UCHAR                   reg[ 54 ][ 16 ];
#endif
#endif
    /* for debug hardware transmit\receive packets */
    UCHAR                   fDebugDumpTx;    /* Dump transmit packets to Consult port */
    UCHAR                   fDebugDumpRx;    /* Dump received packets to Consult port */
    UCHAR                   fLoopbackStart;  /* loopback the received packets to trasnmit. */
    UCHAR                   m_bReserved3;
} HARDWARE, *PHARDWARE;


struct hw_fn {
    int m_fPCI;

    void ( *fnSwitchDisableMirrorSniffer )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableMirrorSniffer )( PHARDWARE, UCHAR );
    void ( *fnSwitchDisableMirrorReceive )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableMirrorReceive )( PHARDWARE, UCHAR );
    void ( *fnSwitchDisableMirrorTransmit )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableMirrorTransmit )( PHARDWARE, UCHAR );
    void ( *fnSwitchDisableMirrorRxAndTx )( PHARDWARE );
    void ( *fnSwitchEnableMirrorRxAndTx )( PHARDWARE );

    void ( *fnHardwareConfig_TOS_Priority )( PHARDWARE, UCHAR, USHORT );
    void ( *fnSwitchDisableDiffServ )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableDiffServ )( PHARDWARE, UCHAR );

    void ( *fnHardwareConfig802_1P_Priority )( PHARDWARE, UCHAR, USHORT );
    void ( *fnSwitchDisable802_1P )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnable802_1P )( PHARDWARE, UCHAR );
    void ( *fnSwitchDisableDot1pRemapping )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableDot1pRemapping )( PHARDWARE, UCHAR );

    void ( *fnSwitchConfigPortBased )( PHARDWARE, UCHAR, UCHAR );

    void ( *fnSwitchDisableMultiQueue )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableMultiQueue )( PHARDWARE, UCHAR );

    void ( *fnSwitchDisableBroadcastStorm )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnableBroadcastStorm )( PHARDWARE, UCHAR );
    void ( *fnHardwareConfigBroadcastStorm )( PHARDWARE, int );

    void ( *fnSwitchDisablePriorityRate )( PHARDWARE, UCHAR );
    void ( *fnSwitchEnablePriorityRate )( PHARDWARE, UCHAR );

    void ( *fnHardwareConfigRxPriorityRate )( PHARDWARE, UCHAR, UCHAR,
        ULONG );
    void ( *fnHardwareConfigTxPriorityRate )( PHARDWARE, UCHAR, UCHAR,
        ULONG );

    void ( *fnPortSet_STP_State )( PHARDWARE, UCHAR, int );

    void ( *fnPortReadMIBCounter )( PHARDWARE, UCHAR, USHORT, PULONGLONG );
    void ( *fnPortReadMIBPacket )( PHARDWARE, UCHAR, PULONG, PULONGLONG );

    void ( *fnSwitchEnableVlan )( PHARDWARE );
};


#ifdef NDIS_MINIPORT_DRIVER
#if defined( NDIS50_MINIPORT )  ||  defined( NDIS51_MINIPORT )
#include <poppack.h>
#else

#ifdef UNDER_CE
    #pragma warning(disable:4103)
    #pragma pack(1)

#else
#include <packoff.h>
#endif
#endif
#endif


#define BASE_IO_RANGE           0x10

#if 1
#define AUTO_RELEASE
#endif
#if 1
#define AUTO_FAST_AGING
#endif
#if 0
#define EARLY_RECEIVE
#endif
#if 0
#define EARLY_TRANSMIT
#endif


/* Bank select register offset is accessible in all banks to allow bank
   selection.
*/

#define REG_BANK_SEL_OFFSET     0x0E

/* -------------------------------------------------------------------------- */

/*
    KS8841\KS8842 register definitions
*/


#define SW_PHY_AUTO             0         /* autosense */
#define SW_PHY_10BASE_T         1         /* 10Base-T */
#define SW_PHY_10BASE_T_FD      2         /* 10Base-T Full Duplex */
#define SW_PHY_100BASE_TX       3         /* 100Base-TX */
#define SW_PHY_100BASE_TX_FD    4         /* 100Base-TX Full Duplex */

/* Default setting definitions */

#define KS8695_MIN_FBUF         (1536)    /* min data buffer size */
#define BUFFER_1568             1568      /* 0x620 */
#define BUFFER_2044             2044      /* 2K-4 buffer to meet Huge packet support (1916 bytes)
                                             (max buffer length that ks884x allowed */

#define RXCHECKSUM_DEFAULT      TRUE      /* HW Rx IP/TCP/UDP checksum enable */
#define TXCHECKSUM_DEFAULT      TRUE      /* HW Tx IP/TCP/UDP checksum enable */
#define FLOWCONTROL_DEFAULT     TRUE      /* Flow control enable */
#define PBL_DEFAULT             8         /* DMA Tx/Rx burst Size. 0:unlimited, other value for (4 * x) */

#define PHY_POWERDOWN_DEFAULT   TRUE      /* PHY PowerDown Reset enable */
#define PHY_SPEED_DEFAULT       SW_PHY_AUTO /* PHY auto-negotiation enable */

#define PORT_STP_DEFAULT        FALSE     /* Spanning tree disable */
#define PORT_STORM_DEFAULT      TRUE      /* Broadcast storm protection enable */

/* OBCR */
#define BUS_SPEED_125_MHZ       0x0000
#define BUS_SPEED_62_5_MHZ      0x0001
#define BUS_SPEED_41_66_MHZ     0x0002
#define BUS_SPEED_25_MHZ        0x0003

/* EEPCR */
#define EEPROM_CHIP_SELECT      0x0001
#define EEPROM_SERIAL_CLOCK     0x0002
#define EEPROM_DATA_OUT         0x0004
#define EEPROM_DATA_IN          0x0008
#define EEPROM_ACCESS_ENABLE    0x0010

/* MBIR */
#define RX_MEM_TEST_FAILED      0x0008
#define RX_MEM_TEST_FINISHED    0x0010
#define TX_MEM_TEST_FAILED      0x0800
#define TX_MEM_TEST_FINISHED    0x1000

/* PMCS */
#define POWER_STATE_D0          0x0000
#define POWER_STATE_D1          0x0001
#define POWER_STATE_D2          0x0002
#define POWER_STATE_D3          0x0003
#define POWER_STATE_MASK        0x0003
#define POWER_PME_ENABLE        0x0100
#define POWER_PME_STATUS        0x8000

/* WFCR */
#define WOL_MAGIC_ENABLE        0x0080
#define WOL_FRAME3_ENABLE       0x0008
#define WOL_FRAME2_ENABLE       0x0004
#define WOL_FRAME1_ENABLE       0x0002
#define WOL_FRAME0_ENABLE       0x0001


/*
 * KS8841/KS8842 interface to Host by ISA bus.
 */

#ifdef KS_ISA_BUS

/* Bank 0 */

/* BAR */
#define REG_BASE_ADDR_BANK      0
#define REG_BASE_ADDR_OFFSET    0x00

/* BDAR */
#define REG_RX_WATERMARK_BANK   0
#define REG_RX_WATERMARK_OFFSET 0x04

#define RX_HIGH_WATERMARK_2KB   0x1000

/* BESR */
#define REG_BUS_ERROR_BANK      0
#define REG_BUS_ERROR_OFFSET    0x06

/* BBLR */
#define REG_BUS_BURST_BANK      0
#define REG_BUS_BURST_OFFSET    0x08

#define BURST_LENGTH_0          0x0000
#define BURST_LENGTH_4          0x3000
#define BURST_LENGTH_8          0x5000
#define BURST_LENGTH_16         0x7000

/* Bank 2 */

#define REG_ADDR_0_BANK         2
/* MARL */
#define REG_ADDR_0_OFFSET       0x00
#define REG_ADDR_1_OFFSET       0x01
/* MARM */
#define REG_ADDR_2_OFFSET       0x02
#define REG_ADDR_3_OFFSET       0x03
/* MARH */
#define REG_ADDR_4_OFFSET       0x04
#define REG_ADDR_5_OFFSET       0x05


/* Bank 3 */

/* OBCR */
#define REG_BUS_CTRL_BANK       3
#define REG_BUS_CTRL_OFFSET     0x00

/* EEPCR */
#define REG_EEPROM_CTRL_BANK    3
#define REG_EEPROM_CTRL_OFFSET  0x02

/* MBIR */
#define REG_MEM_INFO_BANK       3
#define REG_MEM_INFO_OFFSET     0x04

/* GCR */
#define REG_GLOBAL_CTRL_BANK    3
#define REG_GLOBAL_CTRL_OFFSET  0x06

/* WFCR */
#define REG_WOL_CTRL_BANK       3
#define REG_WOL_CTRL_OFFSET     0x0A

/* WF0 */
#define REG_WOL_FRAME_0_BANK    4
#define WOL_FRAME_CRC_OFFSET    0x00
#define WOL_FRAME_BYTE0_OFFSET  0x04
#define WOL_FRAME_BYTE2_OFFSET  0x08

/* WF1 */
#define REG_WOL_FRAME_1_BANK    5

/* WF2 */
#define REG_WOL_FRAME_2_BANK    6

/* WF3 */
#define REG_WOL_FRAME_3_BANK    7


/* Bank 16 */

/* TXCR */
#define REG_TX_CTRL_BANK        16
#define REG_TX_CTRL_OFFSET      0x00

#define TX_CTRL_ENABLE          0x0001
#define TX_CTRL_CRC_ENABLE      0x0002
#define TX_CTRL_PAD_ENABLE      0x0004
#define TX_CTRL_FLOW_ENABLE     0x0008
#define TX_CTRL_MAC_LOOPBACK    0x2000

/* TXSR */
#define REG_TX_STATUS_BANK      16
#define REG_TX_STATUS_OFFSET    0x02

#define TX_FRAME_ID_MASK        0x003F
#define TX_STAT_MAX_COL         0x1000
#define TX_STAT_LATE_COL        0x2000
#define TX_STAT_UNDERRUN        0x4000
#define TX_STAT_COMPLETE        0x8000

#ifdef EARLY_TRANSMIT
#define TX_STAT_ERRORS          ( TX_STAT_MAX_COL | TX_STAT_LATE_COL | TX_STAT_UNDERRUN )
#else
#define TX_STAT_ERRORS          ( TX_STAT_MAX_COL | TX_STAT_LATE_COL )
#endif

#define TX_CTRL_DEST_PORTS      0x0F00
#define TX_CTRL_INTERRUPT_ON    0x8000

#define TX_DEST_PORTS_SHIFT     8

#define TX_FRAME_ID_MAX         (( (TX_FRAME_ID_MASK + 1) / 2 ) - 1 )
#define TX_FRAME_ID_PORT_SHIFT  5

/* RXCR */
#define REG_RX_CTRL_BANK        16
#define REG_RX_CTRL_OFFSET      0x04

#define RX_CTRL_ENABLE          0x0001
#define RX_CTRL_MULTICAST       0x0004
#define RX_CTRL_STRIP_CRC       0x0008
#define RX_CTRL_PROMISCUOUS     0x0010
#define RX_CTRL_UNICAST         0x0020
#define RX_CTRL_ALL_MULTICAST   0x0040
#define RX_CTRL_BROADCAST       0x0080
#define RX_CTRL_BAD_PACKET      0x0200
#define RX_CTRL_FLOW_ENABLE     0x0400

/* TXMIR */
#define REG_TX_MEM_INFO_BANK    16
#define REG_TX_MEM_INFO_OFFSET  0x08

/* RXMIR */
#define REG_RX_MEM_INFO_BANK    16
#define REG_RX_MEM_INFO_OFFSET  0x0A

#define MEM_AVAILABLE_MASK      0x1FFF


/* Bank 17 */

/* TXQCR */
#define REG_TXQ_CMD_BANK        17
#define REG_TXQ_CMD_OFFSET      0x00

#define TXQ_CMD_ENQUEUE_PACKET  0x0001

/* RXQCR */
#define REG_RXQ_CMD_BANK        17
#define REG_RXQ_CMD_OFFSET      0x02

#define RXQ_CMD_FREE_PACKET     0x0001

/* TXFDPR */
#define REG_TX_ADDR_PTR_BANK    17
#define REG_TX_ADDR_PTR_OFFSET  0x04

/* RXFDPR */
#define REG_RX_ADDR_PTR_BANK    17
#define REG_RX_ADDR_PTR_OFFSET  0x06

#define ADDR_PTR_MASK           0x03FF
#define ADDR_PTR_AUTO_INC       0x4000

#define REG_DATA_BANK           17
/* QDRL */
#define REG_DATA_OFFSET         0x08
/* QDRH */
#define REG_DATA_HI_OFFSET      0x0A


/* Bank 18 */

/* IER */
#define REG_INT_MASK_BANK       18
#define REG_INT_MASK_OFFSET     0x00

#define INT_RX_ERROR            0x0080
#define INT_RX_STOPPED          0x0100
#define INT_TX_STOPPED          0x0200
#define INT_RX_EARLY            0x0400
#define INT_RX_OVERRUN          0x0800
#define INT_TX_UNDERRUN         0x1000
#define INT_RX                  0x2000
#define INT_TX                  0x4000
#define INT_PHY                 0x8000
#define INT_MASK                ( INT_RX | INT_TX )


/* ISR */
#define REG_INT_STATUS_BANK     18
#define REG_INT_STATUS_OFFSET   0x02

#ifdef SH_32BIT_ACCESS_ONLY
#define INT_STATUS( intr )  (( intr ) << 16 )
#endif

/* RXSR */
#define REG_RX_STATUS_BANK      18
#define REG_RX_STATUS_OFFSET    0x04

#define RX_BAD_CRC              0x0001
#define RX_TOO_SHORT            0x0002
#define RX_TOO_LONG             0x0004
#define RX_FRAME_ETHER          0x0008
#define RX_PHY_ERROR            0x0010
#define RX_UNICAST              0x0020
#define RX_MULTICAST            0x0040
#define RX_BROADCAST            0x0080
#define RX_SRC_PORTS            0x0F00
#define RX_VALID                0x8000
#define RX_ERRORS     ( RX_BAD_CRC | RX_TOO_LONG | RX_TOO_SHORT | RX_PHY_ERROR )


#define RX_SRC_PORTS_SHIFT      8

/* RXBC */
#define REG_RX_BYTE_CNT_BANK    18
#define REG_RX_BYTE_CNT_OFFSET  0x06

#define RX_BYTE_CNT_MASK        0x07FF

/* ETXR */
#define REG_EARLY_TX_BANK       18
#define REG_EARLY_TX_OFFSET     0x08

#define EARLY_TX_THRESHOLD      0x001F
#define EARLY_TX_ENABLE         0x0080
#define EARLY_TX_MULTIPLE       64

/* ERXR */
#define REG_EARLY_RX_BANK       18
#define REG_EARLY_RX_OFFSET     0x0A

#define EARLY_RX_THRESHOLD      0x001F
#define EARLY_RX_ENABLE         0x0080
#define EARLY_RX_MULTIPLE       64


/* Bank 19 */

#define REG_MULTICAST_BANK      19
/* MTR0 */
#define REG_MULTICAST_0_OFFSET  0x00
#define REG_MULTICAST_1_OFFSET  0x01
/* MTR1 */
#define REG_MULTICAST_2_OFFSET  0x02
#define REG_MULTICAST_3_OFFSET  0x03
/* MTR2 */
#define REG_MULTICAST_4_OFFSET  0x04
#define REG_MULTICAST_5_OFFSET  0x05
/* MTR3 */
#define REG_MULTICAST_6_OFFSET  0x06
#define REG_MULTICAST_7_OFFSET  0x07


#define REG_POWER_CNTL_BANK     19
/* PMCS */
#define REG_POWER_CNTL_OFFSET   0x08


#endif /* ifdef KS_ISA_BUS */

/*
 * KS8841/KS8842 interface to Host by PCI bus.
 */

#ifdef KS_PCI_BUS

/*
 * PCI Configuration ( Space ) Registers
 *
 */

#define CFID                    0x00               /* Configuration ID Register */
#define   CFID_DEVID              0xFFFF0000           /* vendor id, 2 bytes */
#define   CFID_VENID              0x0000FFFF           /* device id, 2 bytes */

#define CFCS                    0x04               /* Command and Status Configuration Register */
#define   CFCS_STAT               0xFFFF0000           /* status register, 2 bytes */
#define   CFCS_COMM               0x0000FFFF           /* command register, 2 bytes */
#define   CFCS_COMM_MEM           0x00000002           /* memory access enable */
#define   CFCS_COMM_MASTER        0x00000004           /* master enable */
#define   CFCS_COMM_PERRSP        0x00000040           /* parity error response */
#define   CFCS_COMM_SYSERREN      0x00000100           /* system error enable */
#define   COMM_SETTING           (CFCS_COMM_MEM | CFCS_COMM_MASTER | CFCS_COMM_PERRSP | CFCS_COMM_SYSERREN)

#define   CFCS_STAT_DPR           0x0100               /* Data Parity Error */
#define   CFCS_STAT_DST           0x0600               /* Device Select Timing */
#define   CFCS_STAT_RVTAB         0x1000               /* Received Target Abort */
#define   CFCS_STAT_RVMAB         0x2000               /* Received Master Abort */
#define   CFCS_STAT_SYSERR        0x4000               /* Signal System Error */
#define   CFCS_STAT_DPERR         0x8000               /* Detected Parity Error */


#define CFRV                    0x08               /* Configuration Revision Register */
#define   CFRV_BASCLASS           0xFF000000           /* basic code, 1 byte */
#define   CFRV_SUBCLASS           0x00FF0000           /* sub-class code, 1 byte */
#define   CFRV_REVID              0x000000FF           /* revision id (Revision\Step number), 1 byte */

#define CFLT                    0x0C               /* Configuration Latency Timer Register */
#define   CFLT_LATENCY_TIMER      0x0000FF00           /* latency timer, 1 byte */
#define   CFLT_CACHE_LINESZ       0x000000FF           /* cache line size, 1 byte */
#define   LATENCY_TIMER           0x00000080           /* default latency timer - 0 */
#define   CACHE_LINESZ                     8           /* default cache line size - 8 (8-DWORD) */

#define CMBA                    0x10               /* Configuration Memory Base Address Register */

#define CSID                    0x2C               /* Subsystem ID Register */
#define   CSID_SUBSYSID           0xFFFF0000           /* Subsystem ID, 2 bytes */
#define   CSID_SUBVENID           0x0000FFFF           /* Subsystem Vendor ID, 2 bytes*/

#define CFIT                    0x3C               /* Configuration Interrupt Register */
#define   CFIT_MAX_L              0xFF000000           /* maximum latency, 1 byte */
#define   CFIT_MIN_G              0x00FF0000           /* minimum grant, 1 byte */
#define   CFIT_IPIN               0x0000FF00           /* interrupt pin, 1 byte */
#define   CFIT_ILINE              0x000000FF           /* interrupt line, 1 byte */
#define   MAX_LATENCY                   0x28           /* default maximum latency - 0x28 */
#define   MIN_GRANT                     0x14           /* default minimum grant - 0x14 */

#define CPMC                    0x54               /* Power Management Control and Status Register */


/* DMA Registers */

#define REG_DMA_TX_CTRL             0x0000
#define DMA_TX_CTRL_ENABLE          0x00000001
#define DMA_TX_CTRL_CRC_ENABLE      0x00000002
#define DMA_TX_CTRL_PAD_ENABLE      0x00000004
#define DMA_TX_CTRL_LOOPBACK        0x00000100
#define DMA_TX_CTRL_FLOW_ENABLE     0x00000200
#define DMA_TX_CTRL_CSUM_IP         0x00010000
#define DMA_TX_CTRL_CSUM_TCP        0x00020000
#define DMA_TX_CTRL_CSUM_UDP        0x00040000
#define DMA_TX_CTRL_BURST_SIZE      0x3F000000

#define REG_DMA_RX_CTRL             0x0004
#define DMA_RX_CTRL_ENABLE          0x00000001
#define DMA_RX_CTRL_MULTICAST       0x00000002
#define DMA_RX_CTRL_PROMISCUOUS     0x00000004
#define DMA_RX_CTRL_ERROR           0x00000008
#define DMA_RX_CTRL_UNICAST         0x00000010
#define DMA_RX_CTRL_ALL_MULTICAST   0x00000020
#define DMA_RX_CTRL_BROADCAST       0x00000040
#define DMA_RX_CTRL_FLOW_ENABLE     0x00000200
#define DMA_RX_CTRL_CSUM_IP         0x00010000
#define DMA_RX_CTRL_CSUM_TCP        0x00020000
#define DMA_RX_CTRL_CSUM_UDP        0x00040000
#define DMA_RX_CTRL_BURST_SIZE      0x3F000000

#define REG_DMA_TX_START            0x0008
#define REG_DMA_RX_START            0x000C
#define DMA_START                   0x00000001      /* DMA start command */

#define REG_DMA_TX_ADDR             0x0010
#define REG_DMA_RX_ADDR             0x0014

#define DMA_ADDR_LIST_MASK          0xFFFFFFFC
#define DMA_ADDR_LIST_SHIFT         2

/* MTR0 */
#define REG_MULTICAST_0_OFFSET      0x0020
#define REG_MULTICAST_1_OFFSET      0x0021
#define REG_MULTICAST_2_OFFSET      0x0022
#define REG_MULTICAST_3_OFFSET      0x0023
/* MTR1 */
#define REG_MULTICAST_4_OFFSET      0x0024
#define REG_MULTICAST_5_OFFSET      0x0025
#define REG_MULTICAST_6_OFFSET      0x0026
#define REG_MULTICAST_7_OFFSET      0x0027

/* Interrupt Registers */

/* INTEN */
#define REG_INTERRUPTS_ENABLE       0x0028
/* INTST */
#define REG_INTERRUPTS_STATUS       0x002C

#define INT_WAN_RX_STOPPED          0x02000000
#define INT_WAN_TX_STOPPED          0x04000000
#define INT_WAN_RX_BUF_UNAVAIL      0x08000000
#define INT_WAN_TX_BUF_UNAVAIL      0x10000000
#define INT_WAN_RX                  0x20000000
#define INT_WAN_TX                  0x40000000
#define INT_WAN_PHY                 0x80000000

#define INT_RX_STOPPED              INT_WAN_RX_STOPPED
#define INT_TX_STOPPED              INT_WAN_TX_STOPPED
#define INT_RX_OVERRUN              INT_WAN_RX_BUF_UNAVAIL
#define INT_TX_UNDERRUN             INT_WAN_TX_BUF_UNAVAIL
#define INT_TX_EMPTY                INT_WAN_TX_BUF_UNAVAIL
#define INT_RX                      INT_WAN_RX
#define INT_TX                      INT_WAN_TX
#define INT_PHY                     INT_WAN_PHY
#define INT_MASK                    ( INT_RX | INT_TX | INT_TX_EMPTY | INT_RX_STOPPED | INT_TX_STOPPED )


/* MAC Addition Station Address */

/* MAAL0 */
#define REG_ADD_ADDR_0_LO           0x0080
/* MAAH0 */
#define REG_ADD_ADDR_0_HI           0x0084
/* MAAL1 */
#define REG_ADD_ADDR_1_LO           0x0088
/* MAAH1 */
#define REG_ADD_ADDR_1_HI           0x008C
/* MAAL2 */
#define REG_ADD_ADDR_2_LO           0x0090
/* MAAH2 */
#define REG_ADD_ADDR_2_HI           0x0094
/* MAAL3 */
#define REG_ADD_ADDR_3_LO           0x0098
/* MAAH3 */
#define REG_ADD_ADDR_3_HI           0x009C
/* MAAL4 */
#define REG_ADD_ADDR_4_LO           0x00A0
/* MAAH4 */
#define REG_ADD_ADDR_4_HI           0x00A4
/* MAAL5 */
#define REG_ADD_ADDR_5_LO           0x00A8
/* MAAH5 */
#define REG_ADD_ADDR_5_HI           0x00AC
/* MAAL6 */
#define REG_ADD_ADDR_6_LO           0x00B0
/* MAAH6 */
#define REG_ADD_ADDR_6_HI           0x00B4
/* MAAL7 */
#define REG_ADD_ADDR_7_LO           0x00B8
/* MAAH7 */
#define REG_ADD_ADDR_7_HI           0x00BC
/* MAAL8 */
#define REG_ADD_ADDR_8_LO           0x00C0
/* MAAH8 */
#define REG_ADD_ADDR_8_HI           0x00C4
/* MAAL9 */
#define REG_ADD_ADDR_9_LO           0x00C8
/* MAAH9 */
#define REG_ADD_ADDR_9_HI           0x00CC
/* MAAL10 */
#define REG_ADD_ADDR_A_LO           0x00D0
/* MAAH10 */
#define REG_ADD_ADDR_A_HI           0x00D4
/* MAAL11 */
#define REG_ADD_ADDR_B_LO           0x00D8
/* MAAH11 */
#define REG_ADD_ADDR_B_HI           0x00DC
/* MAAL12 */
#define REG_ADD_ADDR_C_LO           0x00E0
/* MAAH12 */
#define REG_ADD_ADDR_C_HI           0x00E4
/* MAAL13 */
#define REG_ADD_ADDR_D_LO           0x00E8
/* MAAH13 */
#define REG_ADD_ADDR_D_HI           0x00EC
/* MAAL14 */
#define REG_ADD_ADDR_E_LO           0x00F0
/* MAAH14 */
#define REG_ADD_ADDR_E_HI           0x00F4
/* MAAL15 */
#define REG_ADD_ADDR_F_LO           0x00F8
/* MAAH15 */
#define REG_ADD_ADDR_F_HI           0x00FC

#define ADD_ADDR_HI_MASK            0x00FF
#define ADD_ADDR_ENABLE             0x8000

/* Miscellaneour Registers */

/* MARL */
#define REG_ADDR_0_OFFSET           0x0200
#define REG_ADDR_1_OFFSET           0x0201
/* MARM */
#define REG_ADDR_2_OFFSET           0x0202
#define REG_ADDR_3_OFFSET           0x0203
/* MARH */
#define REG_ADDR_4_OFFSET           0x0204
#define REG_ADDR_5_OFFSET           0x0205

/* OBCR */
#define REG_BUS_CTRL_OFFSET         0x0210

/* EEPCR */
#define REG_EEPROM_CTRL_OFFSET      0x0212

/* MBIR */
#define REG_MEM_INFO_OFFSET         0x0214

/* GCR */
#define REG_GLOBAL_CTRL_OFFSET      0x0216


/* WFCR */
#define REG_WOL_CTRL_OFFSET         0x021A

/* WF0 */

#define WOL_FRAME_CRC_OFFSET        0x0220
#define WOL_FRAME_BYTE0_OFFSET      0x0224
#define WOL_FRAME_BYTE2_OFFSET      0x0228


#endif /* #ifdef KS_PCI_BUS */

/*
 * ks884x Registers Bit definitions
 *
 *  Note: these bit definitions can be used by both ISA_BUS or PCI_BUS interface.
 */

/* Receive Descriptor */
#define DESC_OWN_BIT            0x80000000      /* Descriptor own bit, 1: own by ks884x, 0: own by host */

#define RFC_FS                  0x40000000      /* First Descriptor of the received frame */
#define RFC_LS                  0x20000000      /* Last Descriptor of the received frame */
#define RFC_IPE                 0x10000000      /* IP checksum generation */
#define RFC_TCPE                0x08000000      /* TCP checksum generation */
#define RFC_UDPE                0x04000000      /* UDP checksum generation */
#define RFC_ES                  0x02000000      /* Error Summary */
#define RFC_MF                  0x01000000      /* Multicast Frame */
#define RFC_RE                  0x00080000      /* Report on MII/GMII error */
#define RFC_TL                  0x00040000      /* Frame Too Long */
#define RFC_RF                  0x00020000      /* Runt Frame */
#define RFC_CRC                 0x00010000      /* CRC error */
#define RFC_FT                  0x00008000      /* Frame Type */
#define RFC_FL_MASK             0x000007ff      /* Frame Length bit mask, 0:10 */

#define RFC_ERROR_MASK          (RFC_IPE | RFC_TCPE | RFC_UDPE | RFC_ES | RFC_RE)

/* Transmit Descriptor */
#define TFC_IC                  0x80000000      /* Interrupt on completion */
#define TFC_FS                  0x40000000      /* first segment */
#define TFC_LS                  0x20000000      /* last segment */
#define TFC_IPCKG               0x10000000      /* IP checksum generation */
#define TFC_TCPCKG              0x08000000      /* TCP checksum generation */
#define TFC_UDPCKG              0x04000000      /* UDP checksum generation */
#define TFC_TER                 0x02000000      /* Transmit End of Ring */
#define TFC_TBS_MASK            0x000007ff      /* Transmit Buffer Size Mask (0:10) */


/* DMA Registers */

/* MDTXC                  0x0000 */
/* MDRXC                  0x0004 */
#define DMA_PBLTMASK            0x3f000000      /* DMA Burst Size bit mask */
#define DMA_UDPCHECKSUM         0x00040000      /* MAC UDP checksum enable */
#define DMA_TCPCHECKSUM         0x00020000      /* MAC TCP checksum enable */
#define DMA_IPCHECKSUM          0x00010000      /* MAC IP checksum enable  */
#define DMA_FLOWCTRL            0x00000200      /* MAC flow control enable */
#define DMA_ERRORFRAME          0x00000008      /* MAC will Rx error frame */
#define DMA_PADDING             0x00000004      /* MAC Tx enable padding   */
#define DMA_CRC                 0x00000002      /* MAC Tx add CRC          */

#define DMA_BROADCAST           0x00000040      /* MAC Rx all broadcast frame */
#define DMA_MULTICAST           0x00000020      /* MAC Rx all multicast frame */
#define DMA_UNICAST             0x00000010      /* MAC Rx only unicast frame  */
#define DMA_PROMISCUOUS         0x00000004      /* MAC Rx all all frame       */

/* MDTSC                  0x0008 */
/* MDRSC                  0x000C */
#define DMA_START               0x00000001      /* DMA start command */

/* TDLB                   0x0010 */
/* RDLB                   0x0014 */

/* MTR0                   0x0020 */
/* MTR1                   0x0024 */



/* Interrupt Registers */

/* INTEV                   0x0028 */
/* INTST                   0x002C */
#define INT_TX_DONE             0x40000000      /* Enable Tx completed bit */
#define INT_RX_FRAME            0x20000000      /* Enable Rx at lease a frame bit */
#define INT_TX_STOP             0x04000000      /* Enable Tx stop bit */
#define INT_RX_STOP             0x02000000      /* Enable Tx stop bit */

/* MAC Addition Station Address */

/* MAAL0                   0x0080 */
/* MAAH0                   0x0084 */
#define MAC_ADDR_ENABLE         0x80000000      /* This MAC table entry is Enabled */

/* Miscellaneour Registers */

/* MARL                    0x0200 */
/* MARM                    0x0202 */
/* MARH                    0x0204 */
/* OBCR                    0x0210 */
/* EEPCR                   0x0212 */
/* MBIR                    0x0214 */

/* GCR                     0x0216 */
#define GLOBAL_SOFTWARE_RESET   0x0001      /* pass all frames */

/* Switch Registers */

/* SIDER                   0x0400 */
#define SW_ENABLE               0x0001      /* enable switch */

/* SGCR1                   0x0402 */
#define SW_PASS_ALL_FRAMES      0x8000      /* pass all frames */
#define SW_IEEE_TX_FLOWCNTL     0x2000      /* IEEE 802.3x Tx flow control enable */
#define SW_IEEE_RX_FLOWCNTL     0x1000      /* IEEE 802.3x Rx flow control enable */
#define SW_FRAME_LEN_CHECK      0x0800      /* frame length field check */
#define SW_AGING_ENABLE         0x0400      /* Aging enable */
#define SW_FAST_AGING           0x0200      /* Fast Age enable */
#define SW_BACKOFF_EN           0x0100      /* aggressive back off enable */
#define SW_UNH_BACKOFF_EN       0x0080      /* new backoff enable for UNH */
#define SW_PASS_FLOWCNTL_FRAMES 0x0008      /* NOT filter 802.1x flow control packets */
#define SW_BUFFER_SHARE         0x0004      /* buffer share mode */
#define SW_AUTO_FAST_AGING      0x0001      /* automic fast aging when link changed detected */

/* SGCR2                   0x0404 */
#define SW_8021Q_VLAN_EN        0x8000      /* Enable IEEE 802.1Q VLAN enable */
#define SW_IGMP_SNOOP_EN        0x4000      /* Enable IGMP Snoop on switch MII interface */
#define SW_SNIFF_TX_AND_RX      0x0100      /* Sniff monitor Tx and Rx. */
#define SW_VLAN_MISMATCH_DISCARD 0x0080     /* unicast port-VLAN mismatch discard */
#define SW_NO_MCAST_STORM_INC   0x0040      /* broadcast storm protection not include multicast pkts */
#define SW_PREAMBLE_MODE        0x0020      /* carrier sense based backpressure mode */
#define SW_FLOWCTRL_FAIR        0x0010      /* flow control fair mode */
#define SW_NO_COLLISION_DROP    0x0008      /* no excessive collision drop */
#define SW_HUGE_FRAME_SIZE      0x0004      /* support huge packet size upto 1916-byte */
#define SW_NO_MAX_FRAME_SIZE    0x0002      /* NOT accept packet size upto 1536-byte */
#define SW_PRIORITY_BUF_RESERVE 0x0001      /* pre-allocated 48 buffers per port reserved for high priority pkts */

/* SGCR3                   0x0406 */
#define SW_REPEATER_MODE_EN     0x0080      /* Enable repeater mode */
#define SW_MII_HALF_DUPLEX      0x0040      /* Enable switch MII half duplex mode */
#define SW_MII_FLOW_CNTL        0x0020      /* Enable switch MII flow control */
#define SW_MII_10BT             0x0010      /* The switch MII interface is in 10Mbps mode */
#define SW_NULL_VID             0x0008      /* null VID replacement */

/* SGCR4                   0x0408 */
/* SGCR5                   0x040A */
#define SW_POWER_SAVE           0x0400      /* Enable power save mode */
#define SW_CRC_DROP             0x0200      /* drop MC loop back packets if CRCs are detected */
#define SW_TPID_MODE            0x0100      /* Special TPID mode */

/* SGCR6                   0x0410 */
/* PHAR                    0x0420 */
/* LBS21R                  0x0426 */
/* LBRCTCER                0x0428 */
/* LBRCGR                  0x042A */
/* CSCR                    0x0430 */
/* PSWIR                   0x0432 */
/* RC21R                   0x0434 */
/* RC3R                    0x0436 */
/* VMCRTCR                 0x0438 */
/* S58R                    0x0440 */
/* MVI21R                  0x0444 */
/* MM1V3IR                 0x0446 */
/* MMI32R                  0x0448 */
/* LPVI21R                 0x0450 */
/* LPM1V3IR                0x0452 */
/* LPMI32R                 0x0454 */
/* CSSR                    0x0460 */
/* ASCTR                   0x0464 */
/* MS21R                   0x0468 */
/* LPS21R                  0x046A */

/* MACAR1                  0x0470 */
/* MACAR2                  0x0472 */
/* MACAR3                  0x0474 */

/* TOSR1                   0x0480 */
/* TOSR2                   0x0482 */
/* TOSR3                   0x0484 */
/* TOSR4                   0x0486 */
/* TOSR5                   0x0488 */
/* TOSR6                   0x048A */
/* TOSR7                   0x0490 */
/* TOSR8                   0x0492 */

/* IACR                    0x04A0 */
/* IADR1                   0x04A2 */
/* IADR2                   0x04A4 */
/* IADR3                   0x04A6 */
/* IADR4                   0x04A8 */
/* IADR5                   0x04AA */

/* UDR21                   0x04B0 */
/* UDR3                    0x04B2 */

/* DTSR                    0x04C0 */
/* ATSR                    0x04C2 */
/* DTCR                    0x04C4 */
/* ATCR0                   0x04C6 */
/* ATCR1                   0x04C8 */
/* ATCR2                   0x04CA */

/* P1MBCR                  0x04D0 */
#define PHY_POWER_POWERDOWN     0x0800      /* port power down */
#define PHY_AUTO_NEGOTIATION    0x0200      /* auto-negotiation enable */

/* P1MBSR                  0x04D2 */
#define PHY_AUTONEGO_COMPLETE   0x0020      /* auto nego completed on this port */
#define PHY_LINKUP              0x0004      /* Link is up on this port */

/* PHY1ILR                 0x04D4 */
/* PHY1IHR                 0x04D6 */
/* P1ANAR                  0x04D8 */
/* P1ANLPR                 0x04DA */
#define PARTNER_100FD           0x0100      /* auto nego parterner 100 FD */
#define PARTNER_100HD           0x0080      /* auto nego parterner 100 HD */
#define PARTNER_10FD            0x0040      /* auto nego parterner 10 FD */
#define PARTNER_10HD            0x0020      /* auto nego parterner 10 HD */

/* P2MBCR                  0x04E0 */
/* P2MBSR                  0x04E2 */
/* PHY2ILR                 0x04E4 */
/* PHY2IHR                 0x04E6 */
/* P2ANAR                  0x04E8 */
/* P2ANLPR                 0x04EA */
/* P1VCT                   0x04F0 */
/* P1PHYCTRL               0x04F2 */
/* P2VCT                   0x04F4 */
/* P2PHYCTRL               0x04F6 */

/* P1CR1                   0x0500 */
#define PORT_STORM_PROCTION     0x0080      /* enable broadcast storm protection (ingress) */
#define PORT_QOS_DIFFSERV       0x0040      /* enable QoS - diffServ priority classfication */
#define PORT_QOS_8021P          0x0020      /* enable QoS - 802.1P priority classfication */
#define PORT_TAG_INSERT         0x0004      /* enable VLAN tag insert to the packet (egress) */
#define PORT_TAG_REMOVE         0x0002      /* enable VLAN tag remove from the packet (egress) */
#define PORT_MULTIPLE_Q         0x0001      /* output queue is split into four queues (egress) */

/* P1CR2                   0x0502 */

/* P1VIDCR                 0x0504 */
/* P1CR3                   0x0506 */
/* P1IRCR                  0x0508 */
/* P1ERCR                  0x050A */
/* P1SCSLMD                0x0510 */
/* P1CR4                   0x0512 */
#define PORT_AUTONEGO_RESTART   0x2000      /* auto nego restart */
#define PORT_AUTONEGO_ENABLE    0x0080      /* auto nego enable */
#define PORT_AUTONEGO_ADV_PUASE 0x0010      /* auto nego advertise PAUSE */
#define PORT_AUTONEGO_ADV_100FD 0x0008      /* auto nego advertise 100 FD */
#define PORT_AUTONEGO_ADV_100HD 0x0004      /* auto nego advertise 100 HD */
#define PORT_AUTONEGO_ADV_10FD  0x0002      /* auto nego advertise 10 FD */
#define PORT_AUTONEGO_ADV_10HD  0x0001      /* auto nego advertise 10 HD */
#define PORT_AUTONEGO_ADV_MASK  0x209F

#define PORT_DISABLE_AUTONEG    0x0000      /* port disable auto nego */
#define PORT_100BASE            0x0040      /* force 100 when auto nego disabled */
#define PORT_FULLDUPLEX         0x0020      /* force full duplex when auto nego disabled */
#define PORT_MEDIA_MASK         0x0060

/* P1SR                    0x0514 */
#define PORT_DUPLEX_FULL        0x0400      /* auto nego duplex status (solved) */
#define PORT_SPEED_100BT        0x0200      /* auto nego speed status (solved) */

/* P2CR1                   0x0520 */
/* P2CR2                   0x0522 */
/* P2VIDCR                 0x0524 */
/* P2CR3                   0x0526 */
/* P2IRCR                  0x0528 */
/* P2ERCR                  0x052A */
/* P2SCSLMD                0x0530 */
/* P2CR4                   0x0532 */
/* P2SR                    0x0534 */

/* P3CR1                   0x0540 */
/* P3CR2                   0x0542 */
/* P3VIDCR                 0x0544 */
/* P3CR3                   0x0546 */
/* P3IRCR                  0x0548 */
/* P3ERCR                  0x054A */
/* P3SR                    0x0554 */

/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
#ifdef INLINE
#ifdef SH_16BIT_WRITE
#define HardwareSelectBank( pHardware, bBank )                              \
{                                                                           \
    HW_WRITE_WORD( pHardware, REG_BANK_SEL_OFFSET, bBank );                 \
    ( pHardware )->m_bBank = bBank;                                         \
}

#else
#define HardwareSelectBank( pHardware, bBank )                              \
{                                                                           \
    HW_WRITE_BYTE( pHardware, REG_BANK_SEL_OFFSET, bBank );                 \
    ( pHardware )->m_bBank = bBank;                                         \
}
#endif

#define HardwareReadRegByte( pHardware, bBank, bOffset, pbData )            \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_READ_BYTE( pHardware, bOffset, pbData );                             \
}

#define HardwareWriteRegByte( pHardware, bBank, bOffset, bValue )           \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_WRITE_BYTE( pHardware, bOffset, bValue );                            \
}

#define HardwareReadRegWord( pHardware, bBank, bOffset, pwData )            \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_READ_WORD( pHardware, bOffset, pwData );                             \
}

#define HardwareWriteRegWord( pHardware, bBank, bOffset, wValue )           \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_WRITE_WORD( pHardware, bOffset, wValue );                            \
}

#define HardwareReadRegDWord( pHardware, bBank, bOffset, pwData )           \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_READ_DWORD( pHardware, bOffset, pwData );                            \
}

#define HardwareWriteRegDWord( pHardware, bBank, bOffset, wValue )          \
{                                                                           \
    if ( ( bBank ) != ( pHardware )->m_bBank )                              \
        HardwareSelectBank( pHardware, bBank );                             \
    HW_WRITE_DWORD( pHardware, bOffset, wValue );                           \
}

#ifdef SH_32BIT_ACCESS_ONLY
#define HardwareWriteIntMask( pHardware, ulValue )                          \
{                                                                           \
    if ( REG_INT_MASK_BANK != ( pHardware )->m_bBank )                      \
        HardwareSelectBank( pHardware, REG_INT_MASK_BANK );                 \
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, ulValue );              \
}

#define HardwareWriteIntStat( pHardware, ulValue )                          \
{                                                                           \
    ULONG ulIntEnable;                                                      \
    if ( REG_INT_STATUS_BANK != ( pHardware )->m_bBank )                    \
        HardwareSelectBank( pHardware, REG_INT_STATUS_BANK );               \
    HW_READ_DWORD( pHardware, REG_INT_MASK_OFFSET, &ulIntEnable );          \
    ulIntEnable &= 0x0000FFFF;                                              \
    ulIntEnable |= ulValue;                                                 \
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, ulIntEnable );          \
}
#endif

#else
void HardwareSelectBank (
    PHARDWARE pHardware,
    UCHAR     bBank );

void HardwareReadRegByte (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUCHAR    pbData );

void HardwareReadRegWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUSHORT   pwData );

void HardwareReadRegDWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PULONG    pwData );

void HardwareWriteRegByte (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    UCHAR     bData );

void HardwareWriteRegWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    USHORT    wData );

void HardwareWriteRegDWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    ULONG     wData );

#ifdef SH_32BIT_ACCESS_ONLY
void HardwareWriteIntMask (
    PHARDWARE pHardware,
    ULONG     ulValue );

void HardwareWriteIntStat (
    PHARDWARE pHardware,
    ULONG     ulValue );
#endif

#endif

void hw_read_dword
(
    PHARDWARE phwi,
    UCHAR     addr,
    ULONG    *data
);
void hw_write_dword
(
    PHARDWARE phwi,
    UCHAR     addr,
    ULONG     data
);

void HardwareReadBuffer (
    PHARDWARE pHardware,
    UCHAR     bOffset,
    PULONG    pdwData,
    int       length );

void HardwareWriteBuffer (
    PHARDWARE pHardware,
    UCHAR     bOffset,
    PULONG    pdwData,
    int       length );


#endif

/* -------------------------------------------------------------------------- */

/*
    Initial setup routines
*/

#ifdef KS_ISA_BUS
BOOLEAN HardwareInitialize_ISA (
    PHARDWARE pHardware );

BOOLEAN HardwareReset_ISA (
    PHARDWARE pHardware );

void HardwareSetup_ISA (
    PHARDWARE pHardware );

void HardwareSetupFunc_ISA (
    struct hw_fn* ks8842_fn );

void HardwareSetupInterrupt_ISA (
    PHARDWARE pHardware );

BOOLEAN HardwareSetBurst (
    PHARDWARE pHardware,
    UCHAR     bBurstLength );

#endif

#ifdef KS_PCI
#ifdef DBG
void CheckDescriptors (
    PTDescInfo pInfo );
#endif

void CheckDescriptorNum (
    PTDescInfo pInfo );

BOOLEAN HardwareInitialize_PCI (
    PHARDWARE pHardware );

BOOLEAN HardwareReset_PCI (
    PHARDWARE pHardware );

void HardwareSetup_PCI (
    PHARDWARE pHardware );

void HardwareSetupFunc_PCI (
    struct hw_fn* ks8842_fn );

void HardwareSetupInterrupt_PCI (
    PHARDWARE pHardware );
#endif

void HardwareSwitchSetup
(
    PHARDWARE pHardware );


void HardwareReadChipID
(
    PHARDWARE pHardware,
    PUSHORT   pChipID,
    PUCHAR    pDevRevisionID
);

#ifdef KS_PCI_BUS
#define HardwareInitialize      HardwareInitialize_PCI
#define HardwareReset           HardwareReset_PCI
#define HardwareSetup           HardwareSetup_PCI
#define HardwareSetupFunc       HardwareSetupFunc_PCI
#define HardwareSetupInterrupt  HardwareSetupInterrupt_PCI

#else
#define HardwareInitialize      HardwareInitialize_ISA
#define HardwareReset           HardwareReset_ISA
#define HardwareSetup           HardwareSetup_ISA
#define HardwareSetupFunc       HardwareSetupFunc_ISA
#define HardwareSetupInterrupt  HardwareSetupInterrupt_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Link processing primary routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#ifdef SH_32BIT_ACCESS_ONLY
#define HardwareAcknowledgeLink_ISA( pHardware )                            \
{                                                                           \
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_PHY ));                \
}
#else
#define HardwareAcknowledgeLink_ISA( pHardware )                            \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK,                   \
        REG_INT_STATUS_OFFSET, INT_PHY );                                   \
}
#endif

#else
void HardwareAcknowledgeLink_ISA (
    PHARDWARE pHardware );
#endif

void HardwareCheckLink_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#ifdef INLINE
#define HardwareAcknowledgeLink_PCI( pHardware )                            \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_PHY );            \
}

#else
void HardwareAcknowledgeLink_PCI (
    PHARDWARE pHardware );
#endif

void HardwareCheckLink_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareAcknowledgeLink  HardwareAcknowledgeLink_PCI
#define HardwareCheckLink        HardwareCheckLink_PCI

#else
#define HardwareAcknowledgeLink  HardwareAcknowledgeLink_ISA
#define HardwareCheckLink        HardwareCheckLink_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Receive processing primary routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#define HardwareReleaseReceive_ISA( pHardware )                             \
{                                                                           \
    HardwareWriteRegByte( pHardware, REG_RXQ_CMD_BANK, REG_RXQ_CMD_OFFSET,  \
                          RXQ_CMD_FREE_PACKET );                            \
}

#ifdef SH_32BIT_ACCESS_ONLY
#define HardwareAcknowledgeReceive_ISA( pHardware )                         \
{                                                                           \
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_RX ));                 \
}
#else
#define HardwareAcknowledgeReceive_ISA( pHardware )                         \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,  \
                          INT_RX );                                         \
}
#endif

#else

void HardwareReleaseReceive_ISA (
    PHARDWARE pHardware );

void HardwareAcknowledgeReceive_ISA (
    PHARDWARE pHardware );
#endif

void HardwareStartReceive_ISA (
    PHARDWARE pHardware );

void HardwareStopReceive_ISA (
    PHARDWARE pHardware );

#endif /* #ifdef INLINE */

#ifdef KS_PCI_BUS
#ifdef INLINE
#define HardwareAcknowledgeReceive_PCI( pHardware )                         \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_RX );             \
}

#else
void HardwareReleaseReceive_PCI (
    PHARDWARE pHardware );

void HardwareAcknowledgeReceive_PCI (
    PHARDWARE pHardware );
#endif

#define HardwareResumeReceive( pHardware )                                  \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_START, DMA_START );               \
}

void HardwareStartReceive_PCI (
    PHARDWARE pHardware );

void HardwareStopReceive_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareReleaseReceive      HardwareReleaseReceive_PCI
#define HardwareAcknowledgeReceive  HardwareAcknowledgeReceive_PCI
#define HardwareStartReceive        HardwareStartReceive_PCI
#define HardwareStopReceive         HardwareStopReceive_PCI

#else
#define HardwareReleaseReceive      HardwareReleaseReceive_ISA
#define HardwareAcknowledgeReceive  HardwareAcknowledgeReceive_ISA
#define HardwareStartReceive        HardwareStartReceive_ISA
#define HardwareStopReceive         HardwareStopReceive_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Transmit processing primary routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#ifdef SH_32BIT_ACCESS_ONLY
#define HardwareAcknowledgeTransmit_ISA( pHardware )                        \
{                                                                           \
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_TX ));                 \
}
#else
#define HardwareAcknowledgeTransmit_ISA( pHardware )                        \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK,                   \
        REG_INT_STATUS_OFFSET, INT_TX );                                    \
}
#endif

#else
void HardwareAcknowledgeTransmit_ISA (
    PHARDWARE pHardware );
#endif

void HardwareStartTransmit_ISA (
    PHARDWARE pHardware );

void HardwareStopTransmit_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#ifdef INLINE
#define HardwareAcknowledgeTransmit_PCI( pHardware )                        \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_TX );             \
}

#else
void HardwareAcknowledgeTransmit_PCI (
    PHARDWARE pHardware );
#endif

void HardwareStartTransmit_PCI (
    PHARDWARE pHardware );

void HardwareStopTransmit_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareAcknowledgeTransmit  HardwareAcknowledgeTransmit_PCI
#define HardwareStartTransmit        HardwareStartTransmit_PCI
#define HardwareStopTransmit         HardwareStopTransmit_PCI

#else
#define HardwareAcknowledgeTransmit  HardwareAcknowledgeTransmit_ISA
#define HardwareStartTransmit        HardwareStartTransmit_ISA
#define HardwareStopTransmit         HardwareStopTransmit_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Interrupt processing primary routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#ifdef SH_32BIT_ACCESS_ONLY
#define HardwareAcknowledgeInterrupt_ISA( pHardware, wInterrupt )           \
{                                                                           \
    HardwareWriteIntStat( pHardware, INT_STATUS( wInterrupt ));             \
}

#define HardwareDisableInterrupt_ISA( pHardware )                           \
{                                                                           \
    HardwareWriteIntMask( pHardware, 0 );                                   \
}

#define HardwareEnableInterrupt_ISA( pHardware )                            \
{                                                                           \
    HardwareWriteIntMask( pHardware, ( pHardware )->m_wInterruptMask );     \
}

#define HardwareDisableInterruptBit_ISA( pHardware, wInterrupt )            \
{                                                                           \
    ULONG dwReadInterrupt;                                                  \
    HardwareReadRegDWord( pHardware, REG_INT_MASK_BANK,                     \
        REG_INT_MASK_OFFSET, &dwReadInterrupt );                            \
    dwReadInterrupt &= 0x0000FFFF;                                          \
    dwReadInterrupt &= ~wInterrupt;                                         \
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, dwReadInterrupt );      \
}

#define HardwareEnableInterruptBit_ISA( pHardware, wInterrupt )             \
{                                                                           \
    ULONG dwReadInterrupt;                                                  \
    HardwareReadRegDWord( pHardware, REG_INT_MASK_BANK,                     \
        REG_INT_MASK_OFFSET, &dwReadInterrupt );                            \
    dwReadInterrupt &= 0x0000FFFF;                                          \
    dwReadInterrupt |= wInterrupt;                                          \
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, dwReadInterrupt );      \
}

#define HardwareReadInterrupt_ISA( pHardware, pwStatus )                    \
{                                                                           \
    HardwareReadRegWord( pHardware, REG_INT_STATUS_BANK,                    \
        REG_INT_STATUS_OFFSET, pwStatus );                                  \
}

#define HardwareSetInterrupt_ISA( pHardware, wInterrupt )                   \
{                                                                           \
    HardwareWriteIntMask( pHardware, wInterrupt );                          \
}
#else
#define HardwareAcknowledgeInterrupt_ISA( pHardware, wInterrupt )           \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK,                   \
        REG_INT_STATUS_OFFSET, wInterrupt );                                \
}

#define HardwareDisableInterrupt_ISA( pHardware )                           \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK,                     \
        REG_INT_MASK_OFFSET, 0 );                                           \
}

#define HardwareEnableInterrupt_ISA( pHardware )                            \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK,                     \
        REG_INT_MASK_OFFSET, ( pHardware )->m_wInterruptMask );             \
}

#define HardwareDisableInterruptBit_ISA( pHardware, wInterrupt )            \
{                                                                           \
    USHORT     wReadInterrupt;                                              \
    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET, \
                          &wReadInterrupt );                                \
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,\
                          (wReadInterrupt & ~wInterrupt) );                 \
}

#define HardwareEnableInterruptBit_ISA( pHardware, wInterrupt )             \
{                                                                           \
    USHORT     wReadInterrupt;                                              \
    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET, \
                          &wReadInterrupt );                                \
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,\
                          (wReadInterrupt | wInterrupt) );                  \
}

#define HardwareReadInterrupt_ISA( pHardware, pwStatus )                    \
{                                                                           \
    HardwareReadRegWord( pHardware, REG_INT_STATUS_BANK,                    \
        REG_INT_STATUS_OFFSET, pwStatus );                                  \
}

#define HardwareSetInterrupt_ISA( pHardware, wInterrupt )                   \
{                                                                           \
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK,                     \
        REG_INT_MASK_OFFSET, wInterrupt );                                  \
}
#endif

#else
void HardwareAcknowledgeInterrupt_ISA (
    PHARDWARE pHardware,
    USHORT    wInterrupt );

void HardwareDisableInterrupt_ISA (
    PHARDWARE pHardware );

void HardwareEnableInterrupt_ISA (
    PHARDWARE pHardware );

void HardwareDisableInterruptBit_ISA (
    PHARDWARE pHardware,
    USHORT    wInterrupt );

void HardwareEnableInterruptBit_ISA (
    PHARDWARE pHardware,
    USHORT    wInterrupt );

void HardwareReadInterrupt_ISA (
    PHARDWARE pHardware,
    PUSHORT   wStatus );

void HardwareSetInterrupt_ISA (
    PHARDWARE pHardware,
    USHORT    wInterrupt );
#endif

USHORT HardwareBlockInterrupt_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#ifdef INLINE
#define HardwareAcknowledgeInterrupt_PCI( pHardware, ulInterrupt )          \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, ulInterrupt );        \
}

#define HardwareDisableInterrupt_PCI( pHardware )                           \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, 0 );                  \
    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE,                        \
        &( pHardware )->m_ulInterruptSet );                                 \
}

#define HardwareEnableInterrupt_PCI( pHardware )                            \
{                                                                           \
    ( pHardware )->m_ulInterruptSet = ( pHardware )->m_ulInterruptMask;     \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE,                       \
        ( pHardware )->m_ulInterruptMask );                                 \
}

#define HardwareDisableInterruptBit_PCI( pHardware, ulInterrupt )           \
{                                                                           \
    ULONG     ulReadInterrupt;                                              \
    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE, &ulReadInterrupt );    \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, ( ulReadInterrupt & ~ulInterrupt) ); \
}

#define HardwareEnableInterruptBit_PCI( pHardware, ulInterrupt )            \
{                                                                           \
    ULONG     ulReadInterrupt;                                              \
    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE, &ulReadInterrupt );    \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, ( ulReadInterrupt | ulInterrupt) ); \
}

#define HardwareReadInterrupt_PCI( pHardware, pulStatus )                   \
{                                                                           \
    HW_READ_DWORD( pHardware, REG_INTERRUPTS_STATUS, pulStatus );           \
}

#define HardwareSetInterrupt_PCI( pHardware, ulInterrupt )                  \
{                                                                           \
    ( pHardware )->m_ulInterruptSet = ulInterrupt;                          \
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, ulInterrupt );        \
}

#else
void HardwareAcknowledgeInterrupt_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterrupt );

void HardwareDisableInterrupt_PCI (
    PHARDWARE pHardware );

void HardwareEnableInterrupt_PCI (
    PHARDWARE pHardware );

void HardwareDisableInterruptBit_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterrupt );

void HardwareEnableInterruptBit_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterrupt );

void HardwareReadInterrupt_PCI (
    PHARDWARE pHardware,
    PULONG    pulStatus );

void HardwareSetInterrupt_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterrupt );
#endif

ULONG HardwareBlockInterrupt_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareAcknowledgeInterrupt  HardwareAcknowledgeInterrupt_PCI
#define HardwareDisableInterrupt      HardwareDisableInterrupt_PCI
#define HardwareEnableInterrupt       HardwareEnableInterrupt_PCI
#define HardwareDisableInterruptBit   HardwareDisableInterruptBit_PCI
#define HardwareEnableInterruptBit    HardwareEnableInterruptBit_PCI
#define HardwareReadInterrupt         HardwareReadInterrupt_PCI
#define HardwareSetInterrupt          HardwareSetInterrupt_PCI
#define HardwareBlockInterrupt        HardwareBlockInterrupt_PCI

#else
#define HardwareAcknowledgeInterrupt  HardwareAcknowledgeInterrupt_ISA
#define HardwareDisableInterrupt      HardwareDisableInterrupt_ISA
#define HardwareEnableInterrupt       HardwareEnableInterrupt_ISA
#define HardwareDisableInterruptBit   HardwareDisableInterruptBit_ISA
#define HardwareEnableInterruptBit    HardwareEnableInterruptBit_ISA
#define HardwareReadInterrupt         HardwareReadInterrupt_ISA
#define HardwareSetInterrupt          HardwareSetInterrupt_ISA
#define HardwareBlockInterrupt        HardwareBlockInterrupt_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Other interrupt primary routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#define HardwareTurnOffInterrupt_ISA( pHardware, wInterruptBit )            \
{                                                                           \
    ( pHardware )->m_wInterruptMask &= ~( wInterruptBit );                  \
}

#else
void HardwareTurnOffInterrupt_ISA (
    PHARDWARE pHardware,
    USHORT    wInterruptBit );
#endif

void HardwareTurnOnInterrupt_ISA (
    PHARDWARE pHardware,
    USHORT    wInterruptBit,
    PUSHORT   pwInterruptMask );

#define HardwareTurnOffInterrupt    HardwareTurnOffInterrupt_ISA
#define HardwareTurnOnInterrupt     HardwareTurnOnInterrupt_ISA

#define HardwareAcknowledgeUnderrun( pHardware )                            \
    HardwareAcknowledgeInterrupt( pHardware, INT_TX_UNDERRUN )

#define HardwareAcknowledgeEarly( pHardware )                               \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_EARLY )

#define HardwareAcknowledgeOverrun( pHardware )                             \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_OVERRUN )

#define HardwareAcknowledgeErrorFrame( pHardware )                          \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_ERROR )

#define HardwareAcknowledgeRxStop( pHardware )                              \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_STOPPED )

#define HardwareAcknowledgeTxStop( pHardware )                              \
    HardwareAcknowledgeInterrupt( pHardware, INT_TX_STOPPED )


#define HardwareTurnOffEarlyInterrupt( pHardware )                          \
    HardwareTurnOffInterrupt( pHardware, INT_RX_EARLY )


#define HardwareTurnOnEarlyInterrupt( pHardware, pbInterruptMask )          \
    HardwareTurnOnInterrupt( pHardware, INT_RX_EARLY, pbInterruptMask )

#endif

#ifdef KS_PCI_BUS
#ifdef INLINE
#define HardwareTurnOffInterrupt_PCI( pHardware, ulInterruptBit )           \
{                                                                           \
    ( pHardware )->m_ulInterruptMask &= ~( ulInterruptBit );                \
}

#else
void HardwareTurnOffInterrupt_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterruptBit );
#endif

void HardwareTurnOnInterrupt_PCI (
    PHARDWARE pHardware,
    ULONG     ulInterruptBit,
    PULONG    pulInterruptMask );

#define HardwareTurnOffInterrupt    HardwareTurnOffInterrupt_PCI
#define HardwareTurnOnInterrupt     HardwareTurnOnInterrupt_PCI

#define HardwareAcknowledgeEmpty( pHardware )                               \
    HardwareAcknowledgeInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL )

#define HardwareAcknowledgeOverrun( pHardware )                             \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_OVERRUN )

#define HardwareTurnOffEmptyInterrupt( pHardware )                          \
    HardwareTurnOffInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL )

#define HardwareTurnOnEmptyInterrupt( pHardware, pulInterruptMask )         \
    HardwareTurnOnInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL,             \
        pulInterruptMask )
#endif

/* -------------------------------------------------------------------------- */

/*
    Register saving routines
*/

#ifdef KS_ISA_BUS
#ifdef INLINE
#define HardwareRestoreBank( pHardware )                                    \
{                                                                           \
    HardwareSelectBank( pHardware, ( pHardware )->m_bSavedBank );           \
}

#define HardwareSaveBank( pHardware )                                       \
{                                                                           \
    HW_READ_BYTE( pHardware, REG_BANK_SEL_OFFSET, &( pHardware )->m_bBank );  \
    ( pHardware )->m_bSavedBank = ( pHardware )->m_bBank;                   \
}

#define HardwareRestoreRegs( pHardware )                                    \
{                                                                           \
    HardwareRestoreBank( pHardware );                                       \
}

#define HardwareSaveRegs( pHardware )                                       \
{                                                                           \
    HardwareSaveBank( pHardware );                                          \
}


#else
void HardwareRestoreBank (
    PHARDWARE pHardware );

void HardwareSaveBank (
    PHARDWARE pHardware );

void HardwareRestoreRegs (
    PHARDWARE pHardware );

void HardwareSaveRegs (
    PHARDWARE pHardware );
#endif
#endif

/* -------------------------------------------------------------------------- */

/*
    Hardware enable/disable secondary routines
*/

#ifdef KS_ISA_BUS
void HardwareDisable_ISA (
    PHARDWARE pHardware );

void HardwareEnable_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
void HardwareDisable_PCI (
    PHARDWARE pHardware );

void HardwareEnable_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareDisable     HardwareDisable_PCI
#define HardwareEnable      HardwareEnable_PCI

#else
#define HardwareDisable     HardwareDisable_ISA
#define HardwareEnable      HardwareEnable_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Receive processing secondary routines
*/

#ifdef KS_ISA_BUS
void HardwareReceiveEarly (
    PHARDWARE pHardware );

USHORT HardwareReceiveMoreDataAvailable (
      PHARDWARE pHardware );

BOOLEAN HardwareReceiveStatus (
      PHARDWARE pHardware,
      PUSHORT   pwStatus,
      PUSHORT   pwLength );

int HardwareReceiveLength (
    PHARDWARE pHardware );

void HardwareReceiveBuffer (
    PHARDWARE pHardware,
    void*     pBuffer,
    int       length );

void HardwareReceive_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#ifdef INLINE
#define FreeReceivedPacket( pInfo, iNext )                                  \
{                                                                           \
    iNext++;                                                                \
    iNext &= ( pInfo )->iMax;                                               \
}

#define GetReceivedPacket( pInfo, iNext, pDesc, status )                    \
{                                                                           \
    pDesc = &( pInfo )->pRing[ iNext ];                                     \
    status = LE32_TO_CPU( pDesc->phw->Control.ulData );                     \
}

#define GetRxPacket( pInfo, pDesc )                                         \
{                                                                           \
    pDesc = &( pInfo )->pRing[( pInfo )->iLast ];                           \
    ( pInfo )->iLast++;                                                     \
    ( pInfo )->iLast &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail--;                                                   \
    ( pDesc )->sw.BufSize.ulData &= ~DESC_RX_MASK;                          \
}
#else
void HardwareFreeReceivedPacket (
    PTDescInfo pInfo,
    int*       piNext );

#define FreeReceivedPacket( pInfo, iNext )                                  \
    HardwareFreeReceivedPacket( pInfo, &( iNext ))

PTDesc HardwareGetReceivedPacket (
    PTDescInfo pInfo,
    int        iNext,
    ULONG*     pulData );

#define GetReceivedPacket( pInfo, iNext, pDesc, status )                    \
    pDesc = HardwareGetReceivedPacket( pInfo, iNext, &( status ))

PTDesc HardwareGetRxPacket (
    PTDescInfo pInfo );

#define GetRxPacket( pInfo, pDesc )                                         \
    pDesc = HardwareGetRxPacket( pInfo )
#endif


void HardwareReceive_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareReceive  HardwareReceive_PCI

#else
#define HardwareReceive  HardwareReceive_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Transmit processing secondary routines
*/

#ifdef KS_ISA_BUS
int HardwareAllocPacket_ISA (
    PHARDWARE pHardware,
    int       length );


BOOLEAN HardwareSendPacket_ISA (
    PHARDWARE pHardware );


ULONG HardwareSetTransmitLength_ISA (
    PHARDWARE pHardware,
    int       length );

BOOLEAN HardwareTransmitDone_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
int HardwareAllocPacket_PCI (
    PHARDWARE pHardware,
    int       length,
    int       physical );

#ifdef INLINE
#define FreeTransmittedPacket( pInfo, iLast )                               \
{                                                                           \
    iLast++;                                                                \
    iLast &= ( pInfo )->iMax;                                               \
    ( pInfo )->cnAvail++;                                                   \
}

#define HardwareFreeTxPacket( pInfo )                                       \
{                                                                           \
    ( pInfo )->iNext--;                                                     \
    ( pInfo )->iNext &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail++;                                                   \
}

#define GetTransmittedPacket( pInfo, iLast, pDesc, status )                 \
{                                                                           \
    pDesc = &( pInfo )->pRing[ iLast ];                                     \
    status = LE32_TO_CPU( pDesc->phw->Control.ulData );                     \
}

#define GetTxPacket( pInfo, pDesc )                                         \
{                                                                           \
    pDesc = &( pInfo )->pRing[( pInfo )->iNext ];                           \
    ( pInfo )->iNext++;                                                     \
    ( pInfo )->iNext &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail--;                                                   \
    pDesc->sw.BufSize.ulData &= ~DESC_TX_MASK;                              \
}

#else
void HardwareFreeTransmittedPacket (
    PTDescInfo pInfo,
    int*       piLast );

#define FreeTransmittedPacket( pInfo, iLast )                               \
    HardwareFreeTransmittedPacket( pInfo, &iLast )

void HardwareFreeTxPacket (
    PTDescInfo pInfo );

PTDesc HardwareGetTransmittedPacket (
    PTDescInfo pInfo,
    int        iLast,
    ULONG*     pulData );

#define GetTransmittedPacket( pInfo, iLast, pDesc, status )                 \
    pDesc = HardwareGetTransmittedPacket( pInfo, iLast, &( status ))

PTDesc HardwareGetTxPacket (
    PTDescInfo pInfo );

#define GetTxPacket( pInfo, pDesc )                                         \
    pDesc = HardwareGetTxPacket( pInfo )

#endif

BOOLEAN HardwareSendPacket_PCI (
    PHARDWARE pHardware );

#define SetTransmitBuffer( pDesc, addr )                                    \
    ( pDesc )->phw->ulBufAddr = CPU_TO_LE32( addr )

#define SetTransmitLength( pDesc, len )                                     \
    ( pDesc )->sw.BufSize.tx.wBufSize = len

#ifdef INLINE
#define HardwareSetTransmitBuffer( pHardware, addr )                        \
    SetTransmitBuffer(( pHardware )->m_TxDescInfo.pCurrent, addr )

#else

void HardwareSetTransmitBuffer (
    PHARDWARE pHardware,
    ULONG     ulBufAddr );
#endif

#define HardwareSetTransmitLength( pHardware, len )                         \
    SetTransmitLength(( pHardware )->m_TxDescInfo.pCurrent, len )

BOOLEAN HardwareTransmitDone_PCI (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI_BUS
#define HardwareAllocPacket         HardwareAllocPacket_PCI
#define HardwareSendPacket          HardwareSendPacket_PCI
#define HardwareTransmitDone        HardwareTransmitDone_PCI

#else
#define HardwareAllocPacket         HardwareAllocPacket_ISA
#define HardwareSendPacket          HardwareSendPacket_ISA
#define HardwareSetTransmitLength   HardwareSetTransmitLength_ISA
#define HardwareTransmitDone        HardwareTransmitDone_ISA
#endif

/* -------------------------------------------------------------------------- */

/*
    Other secondary routines
*/

unsigned long ether_crc (
    int  length,
    unsigned char *data );

#ifdef KS_ISA_BUS
BOOLEAN HardwareReadAddress_ISA (
    PHARDWARE pHardware );

void HardwareSetAddress_ISA (
    PHARDWARE pHardware );

void HardwareClearMulticast_ISA (
    PHARDWARE pHardware );

BOOLEAN HardwareSetGroupAddress_ISA (
    PHARDWARE pHardware );

BOOLEAN HardwareSetMulticast_ISA (
    PHARDWARE pHardware,
    UCHAR     bMulticast );

BOOLEAN HardwareSetPromiscuous_ISA (
    PHARDWARE pHardware,
    UCHAR     bPromiscuous );
#endif

#ifdef KS_PCI_BUS
BOOLEAN HardwareReadAddress_PCI (
    PHARDWARE pHardware );

void HardwareSetAddress_PCI (
    PHARDWARE pHardware );

void HardwareClearMulticast_PCI (
    PHARDWARE pHardware );

BOOLEAN HardwareSetGroupAddress_PCI (
    PHARDWARE pHardware );

BOOLEAN HardwareSetMulticast_PCI (
    PHARDWARE pHardware,
    UCHAR     bMulticast );

BOOLEAN HardwareSetPromiscuous_PCI (
    PHARDWARE pHardware,
    UCHAR     bPromiscuous );
#endif

#ifdef KS_PCI_BUS
#define HardwareReadAddress         HardwareReadAddress_PCI
#define HardwareSetAddress          HardwareSetAddress_PCI
#define HardwareClearMulticast      HardwareClearMulticast_PCI
#define HardwareSetGroupAddress     HardwareSetGroupAddress_PCI
#define HardwareSetMulticast        HardwareSetMulticast_PCI
#define HardwareSetPromiscuous      HardwareSetPromiscuous_PCI

#else
#define HardwareReadAddress         HardwareReadAddress_ISA
#define HardwareSetAddress          HardwareSetAddress_ISA
#define HardwareClearMulticast      HardwareClearMulticast_ISA
#define HardwareSetGroupAddress     HardwareSetGroupAddress_ISA
#define HardwareSetMulticast        HardwareSetMulticast_ISA
#define HardwareSetPromiscuous      HardwareSetPromiscuous_ISA
#endif

void HardwareAddrFilterClear (
    PHARDWARE  pHardware  );

int  HardwareAddrFilterAdd (
    PHARDWARE  pHardware,
    UCHAR     *pMacAddress );

int  HardwareAddrFilterDel (
    PHARDWARE  pHardware,
    UCHAR     *pMacAddress );


/* -------------------------------------------------------------------------- */

void HardwareClearCounters (
    PHARDWARE pHardware );


/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
void HardwareGenerateInterrupt (
    PHARDWARE pHardware );

void HardwareServiceInterrupt (
    PHARDWARE pHardware );
#endif

/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
void HardwareGetCableStatus_ISA (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer );

void HardwareGetLinkStatus_ISA (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer );

void HardwareSetCapabilities_ISA (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    ULONG     ulCapabilities );
#endif

#ifdef KS_PCI_BUS
void HardwareGetCableStatus_PCI (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer );

void HardwareGetLinkStatus_PCI (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer );

void HardwareSetCapabilities_PCI (
    PHARDWARE pHardware,
    UCHAR     bPhy,
    ULONG     ulCapabilities );
#endif

#ifdef KS_PCI_BUS
#define HardwareGetCableStatus      HardwareGetCableStatus_PCI
#define HardwareGetLinkStatus       HardwareGetLinkStatus_PCI
#define HardwareSetCapabilities     HardwareSetCapabilities_PCI

#else
#define HardwareGetCableStatus      HardwareGetCableStatus_ISA
#define HardwareGetLinkStatus       HardwareGetLinkStatus_ISA
#define HardwareSetCapabilities     HardwareSetCapabilities_ISA
#endif

/* -------------------------------------------------------------------------- */

#ifdef KS_PCI_BUS
#define ReleaseDescriptor( pDesc, status )                                  \
{                                                                           \
    status.rx.fHWOwned = FALSE;                                             \
    ( pDesc )->phw->Control.ulData = CPU_TO_LE32( status.ulData );          \
}

#define ReleasePacket( pDesc )                                              \
{                                                                           \
    ( pDesc )->sw.Control.tx.fHWOwned = TRUE;                               \
    if ( ( pDesc )->sw.ulBufSize != ( pDesc )->sw.BufSize.ulData )          \
    {                                                                       \
        ( pDesc )->sw.ulBufSize = ( pDesc )->sw.BufSize.ulData;             \
        ( pDesc )->phw->BufSize.ulData =                                    \
            CPU_TO_LE32(( pDesc )->sw.BufSize.ulData );                     \
    }                                                                       \
    ( pDesc )->phw->Control.ulData =                                        \
        CPU_TO_LE32(( pDesc )->sw.Control.ulData );                         \
}

#define SetReceiveBuffer( pDesc, addr )                                     \
    ( pDesc )->phw->ulBufAddr = CPU_TO_LE32( addr )

#define SetReceiveLength( pDesc, len )                                      \
    ( pDesc )->sw.BufSize.rx.wBufSize = len

/* -------------------------------------------------------------------------- */

void HardwareInitDescriptors (
    PTDescInfo pDescInfo,
    int        fTransmit );

void HardwareSetDescriptorBase (
    PHARDWARE  pHardware,
    ULONG      TxDescBaseAddr,
    ULONG      RxDescBaseAddr );

void HardwareResetPackets (
    PTDescInfo pInfo );

#define HardwareResetRxPackets( pHardware )                                 \
    HardwareResetPackets( &pHardware->m_RxDescInfo )

#define HardwareResetTxPackets( pHardware )                                 \
    HardwareResetPackets( &pHardware->m_TxDescInfo )
#endif

/* -------------------------------------------------------------------------- */

void HardwareEnableWolMagicPacket
(
    PHARDWARE pHardware
);

void HardwareEnableWolFrame
(
    PHARDWARE pHardware,
    UINT32    dwFrame
);

void HardwareSetWolFrameCRC
(
    PHARDWARE pHardware,
    UINT32    dwFrame,
    UINT32    dwCRC
);

void HardwareSetWolFrameByteMask
(
    PHARDWARE pHardware,
    UINT32    dwFrame,
    UINT8     bByteMask
);

BOOLEAN HardwareCheckWolPMEStatus
(
    PHARDWARE pHardware
);

void HardwareClearWolPMEStatus
(
    PHARDWARE pHardware
);

/* -------------------------------------------------------------------------- */

void HardwareReadPhy (
    PHARDWARE pHardware,
    int       port,
    USHORT    wPhyReg,
    PUSHORT   pwData );

void HardwareWritePhy (
    PHARDWARE pHardware,
    int       port,
    USHORT    wPhyReg,
    USHORT    wRegData );

/* -------------------------------------------------------------------------- */

USHORT EepromReadWord (
    PHARDWARE pHardware,
    UCHAR     Reg );

void EepromWriteWord (
    PHARDWARE pHardware,
    UCHAR     Reg,
    USHORT    Data );

/* -------------------------------------------------------------------------- */

#include "ks_config.h"

#endif
