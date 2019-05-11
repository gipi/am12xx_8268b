/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_rate.c - KS884X switch rate limit functions.

    Author      Date        Description
    THa         02/13/04    Created file.
    THa         09/29/04    Updated for PCI version.
    THa         10/11/04    Updated with latest specs.
    THa         02/23/06    Removed KS884X_HW conditional.
    THa         02/28/06    Do not use HW_WRITE_BYTE because of limitation of
                            some hardware platforms.
   ---------------------------------------------------------------------------
*/


#ifdef DEF_KS8842
#include "target.h"
#include "hardware.h"


/* -------------------------------------------------------------------------- */

/* 148,800 frames * 67 ms / 100 */
#define BROADCAST_STORM_VALUE  9969

/*
    SwitchConfigBroadcastStorm

    Description:
        This routine configures the broadcast storm threshold of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int percent
            Broadcast storm threshold in percent of transmit rate.

    Return (None):
*/

static
void SwitchConfigBroadcastStorm (
    PHARDWARE pHardware,
    int       percent )
{
    USHORT usData;
    int    value = BROADCAST_STORM_VALUE * percent / 100;

    if ( value > BROADCAST_STORM_RATE )
        value = BROADCAST_STORM_RATE;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif
    HW_READ_WORD( pHardware, REG_SWITCH_CTRL_3_OFFSET, &usData );
    usData &= ~(( BROADCAST_STORM_RATE_LO << 8 ) | BROADCAST_STORM_RATE_HI );
    usData |= (( value & 0x00FF ) << 8 ) | (( value & 0xFF00 ) >> 8 );
    HW_WRITE_WORD( pHardware, REG_SWITCH_CTRL_3_OFFSET, usData );
}  /* SwitchConfigBroadcastStorm */


/*
    SwitchGetBroadcastStorm

    Description:
        This routine retrieves the broadcast storm threshold of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int* percent
            Buffer to store the broadcast storm threshold percent.

    Return (None):
*/

static
void SwitchGetBroadcastStorm (
    PHARDWARE pHardware,
    int*      pnPercent )
{
    int   Data;
    UCHAR bData;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif
    HW_READ_BYTE( pHardware, REG_SWITCH_CTRL_3_OFFSET, &bData );
    Data = ( bData & BROADCAST_STORM_RATE_HI );
    Data <<= 8;
    HW_READ_BYTE( pHardware, REG_SWITCH_CTRL_3_HI_OFFSET, &bData );
    Data |= ( bData & BROADCAST_STORM_RATE_LO );
    *pnPercent = ( Data * 100 + BROADCAST_STORM_VALUE / 2 ) /
        BROADCAST_STORM_VALUE;
}  /* SwitchGetBroadcastStorm */


/*
    SwitchDisableBroadcastStorm

    Description:
        This routine disables the broadcast storm limit function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableBroadcastStorm_PCI
#else
void SwitchDisableBroadcastStorm_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigBroadcastStorm( pHardware, bPort, FALSE );

}  /* SwitchDisableBroadcastStorm */


/*
    SwitchEnableBroadcastStorm

    Description:
        This routine enables the broadcast storm limit function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableBroadcastStorm_PCI
#else
void SwitchEnableBroadcastStorm_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    SwitchConfigBroadcastStorm( pHardware, pHardware->m_bBroadcastPercent );
    PortConfigBroadcastStorm( pHardware, bPort, TRUE );
    SwitchConfigSet( pHardware, REG_SWITCH_CTRL_2_OFFSET,
        MULTICAST_STORM_DISABLE, TRUE );
}  /* SwitchEnableBroadcastStorm */


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchInitBroadcastStorm

    Description:
        This routine initializes the broadcast storm limit function of the
        switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchInitBroadcastStorm (
    PHARDWARE pHardware )
{
    UCHAR bPort;

    pHardware->m_bBroadcastPercent = 1;
    SwitchConfigBroadcastStorm( pHardware, pHardware->m_bBroadcastPercent );
    for ( bPort = 0; bPort < TOTAL_PORT_NUM; bPort++ )
    {
        SwitchDisableBroadcastStorm( pHardware, bPort );
    }
}  /* SwitchInitBroadcastStorm */
#endif


/*
    HardwareConfigBroadcastStorm

    Description:
        This routine configures the broadcast storm threshold of the switch.
        It is called by user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int percent
            Broadcast storm threshold in percent of transmit rate.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareConfigBroadcastStorm_PCI
#else
void HardwareConfigBroadcastStorm_ISA
#endif
(
    PHARDWARE pHardware,
    int       percent )
{
    ASSERT( pHardware->m_bAcquire );

    if ( percent < 0 )
        percent = 1;
    else if ( percent > 100 )
        percent = 100;
    SwitchConfigBroadcastStorm( pHardware, percent );
    SwitchGetBroadcastStorm( pHardware, &percent );
    pHardware->m_bBroadcastPercent = ( UCHAR ) percent;
}  /* HardwareConfigBroadcastStorm */

/* -------------------------------------------------------------------------- */

#define RATE_LIMIT_TABLE_ENTRY   15

typedef struct rateToRegValue
{
      ULONG    rate ;
      UCHAR    regValue ;

} RATETOREGVALUE ;

RATETOREGVALUE rateToRegValueTable[] =
{
    {   64,   0x01 },   /*  64Kbps */
    {   128,  0x02 },   /* 128Kbps */
    {   256,  0x03 },   /* 256Kbps */
    {   512,  0x04 },   /* 512Kbps */
    {  1024,  0x05 },   /*   1Mbps */
    {  2048,  0x06 },   /*   2Mbps */
    {  4096,  0x07 },   /*   4Mbps */
    {  8192,  0x08 },   /*   8Mbps */
    { 16384,  0x09 },   /*  16Mbps */
    { 32768,  0x0A },   /*  32Mbps */
    { 49152,  0x0B },   /*  48Mbps */
    { 65536,  0x0C },   /*  64Mbps */
    { 73728,  0x0D },   /*  72Mbps */
    { 81920,  0x0E },   /*  80Mbps */
    { 90112,  0x0F }    /*  88Mbps */
} ;


/*
    getRateToRegValue

    Description:
        This routine configures the priority rate of the port.

    Parameters:
        ULONG dwRate
            The priority rate in number of Kbps.

    Return
       Register value for this Rate limiting.
*/

static
int getRateToRegValue (
    ULONG     dwRate )
{
    UCHAR   factor = 0;
    UCHAR   i;


    for (i=0; i < (RATE_LIMIT_TABLE_ENTRY-1); i++)
    {
        if ( ( dwRate >= rateToRegValueTable[i].rate ) &&
             ( dwRate < rateToRegValueTable[i+1].rate ) )
        {
            return (rateToRegValueTable[i].regValue);
        }
    }

    if ( dwRate >= rateToRegValueTable[i].rate )
         return (rateToRegValueTable[i].regValue);
    else
         return (factor);
}


/*
    getRegValueToRate

    Description:
        This routine configures the priority rate of the port.

    Parameters:
        ULONG dwRate
            The priority rate in number of Kbps.

    Return
       Register value for this Rate limiting.
*/

static
ULONG getRegValueToRate (
    UCHAR   bRegValue )
{
    ULONG   rate = 0;
    UCHAR   i;


    for (i=0; i < RATE_LIMIT_TABLE_ENTRY; i++)
    {
        if ( bRegValue == rateToRegValueTable[i].regValue )
        {
            return (rateToRegValueTable[i].rate);
        }
    }

    /* not set, return zero (not rate limit) */
    return (rate);
}


/*
    PortConfigRate

    Description:
        This routine configures the priority rate of the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bOffset
            The receive or transmit, high or low priority, rate offset.

        UCHAR bShift
            The shift position to set the value.

        ULONG dwRate
            The rate limit in number of Kbps.

    Return (None):
*/

static
void PortConfigRate (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    UCHAR     bShift,
    ULONG     dwRate )
{
#ifdef SH_16BIT_WRITE
    USHORT    usData;
    UCHAR     bByteShift = bOffset & 1;

#endif
    UCHAR     factor = 0;
    UCHAR     bData;

    factor = ( UCHAR ) getRateToRegValue ( dwRate );

#ifdef SH_16BIT_WRITE
    bOffset &= ~1;
    bByteShift <<= 3;
    PortConfigReadWord( pHardware, bPort,
        REG_PORT_CTRL_BANK,
        bOffset, &usData );
    bData = ( UCHAR )( usData >> bByteShift );

#else
    PortConfigReadByte( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        bOffset, &bData );
#endif

    bData &= ~( PORT_PRIORITY_RATE << bShift );
    bData |= ( UCHAR )( factor << bShift );

#ifdef SH_16BIT_WRITE
    usData &= 0x00FF << bByteShift;
    usData |= ( USHORT ) bData << bByteShift;
    PortConfigWriteWord( pHardware, bPort,
        REG_PORT_CTRL_BANK,
        bOffset, usData );

#else
    PortConfigWriteByte( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        bOffset, bData );
#endif
}  /* PortConfigRate */


/*
    PortGetRate

    Description:
        This routine retrieves the priority rate of the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bOffset
            The receive or transmit, high or low priority, rate offset.

        UCHAR bShift
            The shift position to get the value.

        PULONG pdwBytes
            Buffer to store the data rate in number of  of Kbps.

    Return (None):
*/

static
void PortGetRate (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    UCHAR     bShift,
    PULONG    pdwBytes )
{
    UCHAR bData;

    PortConfigReadByte( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        bOffset, &bData );

    bData >>= bShift;
    bData &= PORT_PRIORITY_RATE;

    *pdwBytes = getRegValueToRate ( bData );

}  /* PortGetRate */


/*
    HardwareConfigRxPriorityRate

    Description:
        This routine configures the receive priority rate of the port.  It
        is called by user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bPriority
            The priority index to configure.

        ULONG dwRate
            The rate limit in number of Kbps.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareConfigRxPriorityRate_PCI
#else
void HardwareConfigRxPriorityRate_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwRate )
{
    UCHAR bOffset;
    UCHAR bShift;

    ASSERT( pHardware->m_bAcquire );

    bOffset = REG_PORT_IN_RATE_OFFSET;
    bOffset += bPriority / 2;
    bShift = PORT_PRIORITY_RATE_SHIFT * ( bPriority & 1 );

    PortConfigRate( pHardware, bPort, bOffset, bShift, dwRate );
    PortGetRate( pHardware, bPort, bOffset, bShift,
        &pHardware->m_Port[ bPort ].dwRxRate[ bPriority ]);

}  /* HardwareConfigRxPriorityRate */


/*
    HardwareConfigTxPriorityRate

    Description:
        This routine configures the transmit priority rate of the port.  It
        is called by user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bPriority
            The priority index to configure.

        ULONG dwRate
            The rate limit in number of Kbps.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareConfigTxPriorityRate_PCI
#else
void HardwareConfigTxPriorityRate_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwRate )
{
    UCHAR bOffset;
    UCHAR bShift;

    ASSERT( pHardware->m_bAcquire );

    bOffset = REG_PORT_OUT_RATE_OFFSET;
    bOffset += bPriority / 2;
    bShift = PORT_PRIORITY_RATE_SHIFT * ( bPriority & 1 );

    PortConfigRate( pHardware, bPort, bOffset, bShift, dwRate );
    PortGetRate( pHardware, bPort, bOffset, bShift,
        &pHardware->m_Port[ bPort ].dwTxRate[ bPriority ]);
}  /* HardwareConfigTxPriorityRate */


/*
    SwitchDisablePriorityRate

    Description:
        This routine disables the priority rate function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisablePriorityRate_PCI
#else
void SwitchDisablePriorityRate_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    {

#ifdef KS_PCI_BUS
        ULONG ulAddr;

        PORT_CTRL_ADDR( bPort, ulAddr );
        ulAddr += REG_PORT_IN_RATE_OFFSET;
        HW_WRITE_DWORD( pHardware, ulAddr, 0 );

#else
        UCHAR bBank = REG_PORT_CTRL_BANK + bPort * PORT_BANK_INTERVAL;

        HardwareSelectBank( pHardware, bBank );
        HW_WRITE_WORD( pHardware, REG_PORT_IN_RATE_OFFSET, 0 );
        HW_WRITE_WORD( pHardware, REG_PORT_OUT_RATE_OFFSET, 0 );
#endif
    }
}  /* SwitchDisablePriorityRate */


/*
    SwitchEnablePriorityRate

    Description:
        This routine enables the priority rate function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnablePriorityRate_PCI
#else
void SwitchEnablePriorityRate_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort
)
{
    UCHAR bPriority;

    ASSERT( pHardware->m_bAcquire );

    for ( bPriority = 0; bPriority < 4; bPriority++ )
    {
            HardwareConfigRxPriorityRate( pHardware, bPort, bPriority,
                pHardware->m_Port[ bPort ].dwRxRate[ bPriority ]);
            HardwareConfigTxPriorityRate( pHardware, bPort, bPriority,
                pHardware->m_Port[ bPort ].dwTxRate[ bPriority ]);
    }
}  /* SwitchEnablePriorityRate */


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchInitPriorityRate

    Description:
        This routine initializes the priority rate function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchInitPriorityRate (
    PHARDWARE pHardware )
{
    UCHAR bPort;
    UCHAR bPriority;

    for ( bPort = 0; bPort < TOTAL_PORT_NUM; bPort++ )
    {
        for ( bPriority = 0; bPriority < 4; bPriority++ )
        {
            pHardware->m_Port[ bPort ].dwRxRate[ bPriority ] =
            pHardware->m_Port[ bPort ].dwTxRate[ bPriority ] = 0;
        }
        SwitchDisablePriorityRate( pHardware, bPort );
    }
}  /* SwitchInitPriorityRate */
#endif
#endif
