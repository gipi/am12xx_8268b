/*---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
  ---------------------------------------------------------------------------

  ks_config.c - KS884X switch configuration functions.

  Author  Date      Version  Description
  PCD     03/17/06  1.0.0    took out workaround (disable DMA flow control when link is half duplex mode)
                             for "KS8841/2/P can't release backpressue". A4 device fix this problem.
  THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                             some hardware platforms.
  THa     02/23/06           Removed KS884X_HW conditional.
  PCD     11/14/05  0.2.1    enable (1) Aggressive back off algorithm,
                             (2).No excessive collision drop.
                             (3).Flow control. (4).Back pressure
  PCD     10/20/05  0.2.0    incorrect register offset in SwitchSetAddress().
  THa     10/06/05           Disable flow control if half-duplex mode.
  THa     07/07/05           Added auto fast aging.
  THa     05/10/05           Updated file.
  THa     10/14/04           Updated with latest specs.
  THa     09/30/04           Updated for PCI version.
  THa     02/13/04           Created file.
  ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"

//#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);

#if 1
#define LINK_CHECK_FIX
#endif


/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
ULONG SwapBytes (
    ULONG dwData )
{
    ULONG  dwValue;
    PUCHAR pSrc = ( PUCHAR ) &dwData;
    PUCHAR pDst = ( PUCHAR ) &dwValue;

    pDst[ 0 ] = pSrc[ 3 ];
    pDst[ 1 ] = pSrc[ 2 ];
    pDst[ 2 ] = pSrc[ 1 ];
    pDst[ 3 ] = pSrc[ 0 ];
    return( dwValue );
}  /* SwapBytes */
#endif

/* -------------------------------------------------------------------------- */

/*
    PortConfigGet

    Description:
        This function checks whether the specified bits of the port register
        are set or not.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        UCHAR bBits
        ULONG ulBits
            The data bits to check.

    Return (BOOLEAN):
        TRUE if the bits are set; otherwise FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN PortConfigGet_PCI (
#else
BOOLEAN PortConfigGet_ISA (
#endif
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,
    UCHAR     bBits )
{
    UCHAR bData;

#ifdef KS_PCI_BUS
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_READ_BYTE( pHardware, ulAddr, &bData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_READ_BYTE( pHardware, bOffset, &bData );
#endif
    if ( ( bData & bBits ) == bBits )
        return TRUE;
    else
        return FALSE;
}  /* PortConfigGet */


/*
    PortConfigSet

    Description:
        This routine sets or resets the specified bits of the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        UCHAR bBits
        ULONG ulBits
            The data bits to set.

        BOOLEAN fSet
            The flag indicating whether the bits are to be set or not.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigSet_PCI (
#else
void PortConfigSet_ISA (
#endif
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,

#ifdef SH_16BIT_WRITE
    USHORT    bBits,

#else
    UCHAR     bBits,
#endif
    BOOLEAN   fSet )
{
#ifdef KS_PCI_BUS
    ULONG ulAddr;
    UCHAR bData;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_READ_BYTE( pHardware, ulAddr, &bData );
    if ( fSet )
        bData |= bBits;
    else
        bData &= ~bBits;
    HW_WRITE_BYTE( pHardware, ulAddr, bData );

#else
#ifdef SH_16BIT_WRITE
    USHORT RegData;
    UCHAR  bShift = bOffset & 1;

    bOffset &= ~1;
    bBits <<= ( bShift << 3 );
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_READ_WORD( pHardware, bOffset, &RegData );
    if ( fSet )
        RegData |= bBits;
    else
        RegData &= ~bBits;
    HW_WRITE_WORD( pHardware, bOffset, RegData );

#else
    UCHAR bData;

    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_READ_BYTE( pHardware, bOffset, &bData );
    if ( fSet )
        bData |= bBits;
    else
        bData &= ~bBits;
    HW_WRITE_BYTE( pHardware, bOffset, bData );
#endif
#endif
}  /* PortConfigSet */


#ifdef KS_PCI_BUS_
/*
    PortConfigGetShift

    Description:
        This function checks whether the specified bits of the port register
        are set or not.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        ULONG ulOffset
            The offset of the port register.

        UCHAR bShift
            Number of bits to shift.

    Return (BOOLEAN):
        TRUE if the bits are set; otherwise FALSE.
*/

BOOLEAN PortConfigGetShift (
    PHARDWARE pHardware,
    UCHAR     bPort,
    ULONG     ulOffset,
    UCHAR     bShift )
{
    ULONG ulData;
    ULONG ulBits = 1UL << bPort;

    HW_READ_DWORD( pHardware, ulOffset, &ulData );
    ulData >>= bShift;
    if ( ( ulData & ulBits ) == ulBits )
        return TRUE;
    else
        return FALSE;
}  /* PortConfigGetShift */


/*
    PortConfigSetShift

    Description:
        This routine sets or resets the specified bits of the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        ULONG ulOffset
            The offset of the port register.

        UCHAR bShift
            Number of bits to shift.

        BOOLEAN fSet
            The flag indicating whether the bits are to be set or not.

    Return (None):
*/

void PortConfigSetShift (
    PHARDWARE pHardware,
    UCHAR     bPort,
    ULONG     ulOffset,
    UCHAR     bShift,
    BOOLEAN   fSet )
{
    ULONG ulData;
    ULONG ulBits = 1UL << bPort;

    HW_READ_DWORD( pHardware, ulOffset, &ulData );
    ulBits <<= bShift;
    if ( fSet )
        ulData |= ulBits;
    else
        ulData &= ~ulBits;
    HW_WRITE_DWORD( pHardware, ulOffset, ulData );
}  /* PortConfigSetShift */
#endif


/*
    PortConfigReadByte

    Description:
        This routine reads a byte from the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        PUCHAR pbData
            Buffer to store the data.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigReadByte_PCI
#else
void PortConfigReadByte_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,
    PUCHAR    pbData )
{
#ifdef KS_PCI_BUS
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_READ_BYTE( pHardware, ulAddr, pbData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_READ_BYTE( pHardware, bOffset, pbData );
#endif
}  /* PortConfigReadByte */


/*
    PortConfigWriteByte

    Description:
        This routine writes a byte to the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        UCHAR bData
            Data to write.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigWriteByte_PCI
#else
void PortConfigWriteByte_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,
    UCHAR     bData )
{
#ifdef KS_PCI_BUS
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_WRITE_BYTE( pHardware, ulAddr, bData );

#else

#ifdef SH_16BIT_WRITE
    ASSERT( FALSE );
#endif
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_WRITE_BYTE( pHardware, bOffset, bData );
#endif
}  /* PortConfigWriteByte */


/*
    PortConfigReadWord

    Description:
        This routine reads a word from the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        PUSHORT pwData
            Buffer to store the data.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigReadWord_PCI
#else
void PortConfigReadWord_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,
    PUSHORT   pwData )
{
#ifdef KS_PCI_BUS
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_READ_WORD( pHardware, ulAddr, pwData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_READ_WORD( pHardware, bOffset, pwData );
#endif
}  /* PortConfigReadWord */


/*
    PortConfigWriteWord

    Description:
        This routine writes a word to the port register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bBank
            The bank of the port register.

        UCHAR bOffset
            The offset of the port register.

        USHORT usData
            Data to write.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigWriteWord_PCI
#else
void PortConfigWriteWord_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,

#ifdef KS_ISA_BUS
    UCHAR     bBank,
#endif
    UCHAR     bOffset,
    USHORT    usData )
{
#ifdef KS_PCI_BUS
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += bOffset;
    HW_WRITE_WORD( pHardware, ulAddr, usData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( bBank + bPort *
        PORT_BANK_INTERVAL ));
    HW_WRITE_WORD( pHardware, bOffset, usData );
#endif
}  /* PortConfigWriteWord */

/* -------------------------------------------------------------------------- */

/*
    SwitchConfigGet

    Description:
        This function checks whether the specified bits of the switch register
        are set or not.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bOffset
        ULONG ulOffset
            The offset of the switch register.

        UCHAR bBits
        ULONG ulBits
            The data bits to check.

    Return (BOOLEAN):
        TRUE if the bits are set; otherwise FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN SwitchConfigGet_PCI (
#else
BOOLEAN SwitchConfigGet_ISA (
#endif
    PHARDWARE pHardware,
    int       Offset,
    UCHAR     bBits )
{
    UCHAR bData;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif
    HW_READ_BYTE( pHardware, Offset, &bData );
    if ( ( bData & bBits ) == bBits )
        return TRUE;
    else
        return FALSE;
}  /* SwitchConfigGet */


/*
    SwitchConfigSet

    Description:
        This function sets or resets the specified bits of the switch register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bOffset
        ULONG ulOffset
            The offset of the switch register.

        UCHAR bBits
        ULONG ulBits
            The data bits to check.

        BOOLEAN fSet
            The flag indicating whether the bits are to be set or not.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchConfigSet_PCI (
#else
void SwitchConfigSet_ISA (
#endif
    PHARDWARE pHardware,
    int       Offset,

#ifdef SH_16BIT_WRITE
    USHORT    bBits,

#else
    UCHAR     bBits,
#endif
    BOOLEAN   fSet )
{
#ifdef SH_16BIT_WRITE
    USHORT RegData;
    UCHAR  bShift = Offset & 1;

#else
    UCHAR bData;
#endif

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif

#ifdef SH_16BIT_WRITE
    Offset &= ~1;
    bBits <<= ( bShift << 3 );
    HW_READ_WORD( pHardware, Offset, &RegData );
    if ( fSet )
        RegData |= bBits;
    else
        RegData &= ~bBits;
    HW_WRITE_WORD( pHardware, Offset, RegData );

#else
    HW_READ_BYTE( pHardware, Offset, &bData );
    if ( fSet )
        bData |= bBits;
    else
        bData &= ~bBits;
    HW_WRITE_BYTE( pHardware, Offset, bData );
#endif
}  /* SwitchConfigSet */

/* -------------------------------------------------------------------------- */

/*
    SwitchGetAddress

    Description:
        This function retrieves the MAC address of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        PUCHAR MacAddr
            Buffer to store the MAC address.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchGetAddress_PCI (
#else
void SwitchGetAddress_ISA (
#endif
    PHARDWARE pHardware,
    PUCHAR    MacAddr )
{
    int i;

    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_MAC_ADDR_BANK );
#endif
    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {
        HW_READ_BYTE( pHardware, ( ULONG )( REG_MAC_ADDR_0_OFFSET + i ),
            &MacAddr[ MAC_ADDR_ORDER( i )]);
    }
}  /* SwitchGetAddress */


/*
    SwitchSetAddress

    Description:
        This function configures the MAC address of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        PUCHAR MacAddr
            The MAC address.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchSetAddress_PCI (
#else
void SwitchSetAddress_ISA (
#endif
    PHARDWARE pHardware,
    PUCHAR    MacAddr )
{
    int i;

    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_MAC_ADDR_BANK );
#endif
    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {

#ifdef SH_16BIT_WRITE
        if ( ( i & 1 ) )
        {
            HW_WRITE_WORD( pHardware, (( REG_MAC_ADDR_0_OFFSET + i ) & ~1 ),
                ( MacAddr[ MAC_ADDR_ORDER( i )] << 8 ) |
                MacAddr[ MAC_ADDR_ORDER( i - 1 )]);
        }

#else
        HW_WRITE_BYTE( pHardware, ( ULONG )( REG_MAC_ADDR_0_OFFSET + i ),
            MacAddr[ MAC_ADDR_ORDER( i )]);
#endif
    }
}  /* SwitchSetAddress */


/*
    SwitchGetLinkStatus

    Description:
        This routine reads PHY registers to determine the current link status
        of the switch ports.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchGetLinkStatus_PCI (
#else
void SwitchGetLinkStatus_ISA (
#endif
    PHARDWARE pHardware )
{
    PPORT_INFO pInfo;

#ifdef KS_PCI_BUS
    ULONG      InterruptMask;

#else
    USHORT     InterruptMask;
#endif
    int        change = FALSE;
    UCHAR      bData;
    UCHAR      bStatus;
    UCHAR      bLinkStatus;
    UCHAR      bPort;

    /* Save the current interrupt mask and block all interrupts. */
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef DEF_KS8842
    for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
#else
    bPort = 0;
#endif
    {
        pInfo = &pHardware->m_PortInfo[ bPort ];

        /* Read Port Control Register */
        PortConfigReadByte( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_CTRL_BANK,
#endif
                            REG_PORT_CTRL_4_OFFSET, &bData );

        /* Clean previous latch Port Operation Status Register */
        PortConfigReadByte( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_STATUS_BANK,
#endif
                            REG_PORT_STATUS_HI_OFFSET, &bStatus );

        /* Read Port Operation Status Register */
        PortConfigReadByte( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_STATUS_BANK,
#endif
                            REG_PORT_STATUS_HI_OFFSET, &bStatus );

#ifdef LINK_CHECK_FIX
        /* bStatus is changing all the time even when there is no cable
           connection!
        */
#endif

        /* Clean previous latch Port Link Status Register */
        PortConfigReadByte( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_STATUS_BANK,
#endif
                            REG_PORT_STATUS_OFFSET, &bLinkStatus );

        /* Read Port Link Status Register */
        PortConfigReadByte( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_STATUS_BANK,
#endif
                            REG_PORT_STATUS_OFFSET, &bLinkStatus );

#ifdef LINK_CHECK_FIX
        /* bLinkStatus is changing all the time even when there is no cable
           connection!
        */
        bLinkStatus &=
            PORT_AUTO_NEG_COMPLETE |
            PORT_STATUS_LINK_GOOD;
        if ( ( bLinkStatus & (UCHAR)PORT_STATUS_LINK_GOOD ) )
        {
            if ( MediaStateConnected != pInfo->ulHardwareState )
                change = TRUE;
            pInfo->ulHardwareState = MediaStateConnected;
        }
        else
        {
            if ( MediaStateDisconnected != pInfo->ulHardwareState )
                change = TRUE;
            pInfo->ulHardwareState = MediaStateDisconnected;
        }
#endif
        if ( bData != pInfo->bAdvertised  ||
                bLinkStatus != pInfo->bLinkPartner )
        {
#ifndef LINK_CHECK_FIX
            pInfo->ulHardwareState = MediaStateDisconnected;
            if ( ( bLinkStatus & (UCHAR)PORT_STATUS_LINK_GOOD ) )
                pInfo->ulHardwareState = MediaStateConnected;
#endif

#ifdef DBG
            DBG_PRINT( "advertised: %02X - %02X; partner: %02X - %02X"
                NEWLINE, bData, pInfo->bAdvertised, bLinkStatus,
                pInfo->bLinkPartner );
#endif
            change = TRUE;

            pInfo->ulSpeed = 100000;
#if 1
            if ( ( bStatus & (UCHAR)PORT_STAT_SPEED_100MBIT ) )
                pInfo->ulSpeed = 1000000;
#else
            if ( (( bData & PORT_AUTO_NEG_100BTX )  &&
                    ( bLinkStatus & PORT_REMOTE_100BTX ))  ||
                    (( bData & PORT_AUTO_NEG_100BTX_FD )  &&
                    ( bLinkStatus & PORT_REMOTE_100BTX_FD )) )
                pInfo->ulSpeed = 1000000;
#endif
            pInfo->bDuplex = 1;
#if 1
            if ( ( bStatus & (UCHAR)PORT_STAT_FULL_DUPLEX ) )
                pInfo->bDuplex = 2;

#else
            if ( (( bData & PORT_AUTO_NEG_100BTX_FD )  &&
                    ( bLinkStatus & PORT_REMOTE_100BTX_FD ))  ||
                    (( bData & PORT_AUTO_NEG_10BT_FD )  &&
                    ( bLinkStatus & PORT_REMOTE_10BT_FD )  &&
                    ( !( bData & PORT_AUTO_NEG_100BTX )  ||
                    !( bLinkStatus & PORT_REMOTE_100BTX ))) )
                pInfo->bDuplex = 2;
#endif
            pInfo->bAdvertised = bData;
            pInfo->bLinkPartner = bLinkStatus;
        }
    }

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
    if ( change )
    {
        PPORT_INFO pLinked = NULL;

#ifdef DEF_KS8842
        for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
#else
        bPort = 0;
#endif
        {
            pInfo = &pHardware->m_PortInfo[ bPort ];

            if ( MediaStateConnected == pInfo->ulHardwareState )
            {
                if ( !pLinked )
                    pLinked = pInfo;
#if ( defined( DEF_LINUX ) || defined( _WIN32 )) && defined( DBG )
                DBG_PRINT( "link %d: %d, %d"NEWLINE, bPort,
                    ( int ) pInfo->ulSpeed,
                    ( int ) pInfo->bDuplex );
#endif /* #ifdef DEF_LINUX */
            }
            else
            {
#if ( defined( DEF_LINUX ) || defined( _WIN32 )) && defined( DBG )
                DBG_PRINT( "link %d disconnected"NEWLINE, bPort );
#endif /* #ifdef DEF_LINUX */
            }

        }  /* for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ ) */

        if ( pLinked )
            pInfo = pLinked;
        else
            pInfo = &pHardware->m_PortInfo[ 0 ];

        pHardware->m_ulHardwareState = pInfo->ulHardwareState;
        pHardware->m_ulTransmitRate = pInfo->ulSpeed;
        pHardware->m_ulDuplex = pInfo->bDuplex;

    } /* if ( change ) */

}  /* SwitchGetLinkStatus */


#define PHY_RESET_TIMEOUT  10

/*
    SwitchSetLinkSpeed

    Description:
        This routine sets the link speed of the switch ports.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchSetLinkSpeed_PCI (
#else
void SwitchSetLinkSpeed_ISA (
#endif
    PHARDWARE pHardware )
{
    USHORT usData;
    UCHAR  bPort;

#ifdef DEF_KS8842
    for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
#else
    bPort = 0;
#endif
    {
        /* Enable Flow control in the full duplex mode */
        PortConfigForceFlowCtrl ( pHardware, bPort, TRUE );

        /* Enable Back pressure in the half duplex mode */
        PortConfigBackPressure ( pHardware, bPort, TRUE );

        /* Read Port Control register 4 (PnCR4) */
        PortConfigReadWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_CTRL_BANK,
#endif
                            REG_PORT_CTRL_4_OFFSET, &usData );

        usData |= PORT_AUTO_NEG_ENABLE;
        usData |= PORT_AUTO_NEG_SYM_PAUSE;
        usData |= PORT_AUTO_NEG_100BTX_FD | PORT_AUTO_NEG_100BTX |
            PORT_AUTO_NEG_10BT_FD | PORT_AUTO_NEG_10BT;

        /* Check if manual configuration is specified by the user. */
        if ( pHardware->m_bSpeed  ||  pHardware->m_bDuplex )
        {
            if ( 10 == pHardware->m_bSpeed )
            {
                usData &= ~( PORT_AUTO_NEG_100BTX_FD | PORT_AUTO_NEG_100BTX );
            }
            else if ( 100 == pHardware->m_bSpeed )
            {
                usData &= ~( PORT_AUTO_NEG_10BT_FD | PORT_AUTO_NEG_10BT );
            }
            if ( 1 == pHardware->m_bDuplex )
            {
                usData &= ~( PORT_AUTO_NEG_100BTX_FD | PORT_AUTO_NEG_10BT_FD );
            }
            else if ( 2 == pHardware->m_bDuplex )
            {
                usData &= ~( PORT_AUTO_NEG_100BTX | PORT_AUTO_NEG_10BT );
            }
        }

        /* Write Port Control register 4 (PnCR4) */
        PortConfigWriteWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                             REG_PORT_LINK_CTRL_BANK,
#endif
                             REG_PORT_CTRL_4_OFFSET, usData );

        /* Restart Port auto-negotiation */
        usData |= TO_HI_BYTE( PORT_AUTO_NEG_RESTART );
        PortConfigWriteWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                             REG_PORT_LINK_CTRL_BANK,
#endif
                             REG_PORT_CTRL_4_OFFSET, usData );
    }

#ifdef FIBER_100FX
    /* No auto-negotiation for link speed/duplex mode in KSZ8861/2 with 100FX fiber mode. 
       when user define "FIBER_100FX", the driver set KS8861/2 port 1 to 100BT/full duplex 
     */
    bPort = 0;

    /* Read Port Control register 4 (PnCR4) */
    PortConfigReadWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                        REG_PORT_LINK_CTRL_BANK,
#endif
                        REG_PORT_CTRL_4_OFFSET, &usData );

    /* Set Port 1 to 100BT/full duplex */
    usData &= ~( PORT_AUTO_NEG_ENABLE );
    usData |= ( PORT_FORCE_100_MBIT | PORT_FORCE_FULL_DUPLEX);

                DBG_PRINT( "port %d usData %x"NEWLINE, bPort, ( int ) usData );

    PortConfigWriteWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                             REG_PORT_LINK_CTRL_BANK,
#endif
                             REG_PORT_CTRL_4_OFFSET, usData );

#endif /* #ifdef FIBER_100FX */

}  /* SwitchSetLinkSpeed */


/*
    SwitchRestartAutoNego

    Description:
        This routine restart auto-negotiation when the link is down of the switch ports.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchRestartAutoNego_PCI (
#else
void SwitchRestartAutoNego_ISA (
#endif
    PHARDWARE pHardware )
{
    USHORT usData;
    UCHAR  bPort;

    /* Restart Port auto-negotiation */
#ifdef DEF_KS8842
    for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
#else
    bPort = 0;
#endif
    {
        PortConfigReadWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                             REG_PORT_LINK_CTRL_BANK,
#endif
                             REG_PORT_CTRL_4_OFFSET, &usData );

        usData |= TO_HI_BYTE( PORT_AUTO_NEG_RESTART );

        PortConfigWriteWord( pHardware, bPort,
#ifdef KS_ISA_BUS
                             REG_PORT_LINK_CTRL_BANK,
#endif
                             REG_PORT_CTRL_4_OFFSET, usData );

    }

    /* Wait for auto negotiation to complete. */
    DelayMillisec( 150 );

}  /* SwitchRestartAutoNego */


/*
    SwitchSetGlobalControl

    Description:
        This routine sets the global control of the switch function.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchSetGlobalControl_PCI (
#else
void SwitchSetGlobalControl_ISA (
#endif
    PHARDWARE pHardware )
{
    USHORT RegData = 0;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif

    /*
     * Set Switch Global Control Register 3 SGCR3
     */

    /* Enable Switch MII Flow Control */

    HW_READ_WORD( pHardware, REG_SWITCH_CTRL_3_OFFSET, &RegData );
    RegData |= SWITCH_FLOW_CTRL;
    HW_WRITE_WORD( pHardware, REG_SWITCH_CTRL_3_OFFSET, RegData );

    /*
     * Set Switch Global Control Register 1 SGCR1
     */

    HW_READ_WORD( pHardware, REG_SWITCH_CTRL_1_OFFSET, &RegData );

    /* Enable Aggressive back off algorithm in half duplex mode */
    RegData |= SW_BACKOFF_EN;

#ifdef AUTO_FAST_AGING
    /* Enable automic fast aging when link changed detected */
    RegData |= SW_AUTO_FAST_AGING;
#endif
    HW_WRITE_WORD( pHardware, REG_SWITCH_CTRL_1_OFFSET, RegData );

    /*
     * Set Switch Global Control Register 2 SGCR2
     */

    /* Enable No excessive collision drop */
    HW_READ_WORD( pHardware, REG_SWITCH_CTRL_2_OFFSET, &RegData );
    RegData |= SW_NO_COLLISION_DROP;
    HW_WRITE_WORD( pHardware, REG_SWITCH_CTRL_2_OFFSET, RegData );

}  /* SwitchSetGlobalControl */


#ifdef DEF_KS8842
/*
 * SwitchEnable
 *	This function is used to enable/disable Switch Engine.
 *  Only KS8842 has switch function.
 *
 * Argument(s)
 *  pHardware   Pointer to hardware instance.
 *  fEnable     1: enable switch, 0: disable switch
 *
 * Return(s)
 *	NONE.
 */
#ifdef KS_PCI_BUS
void SwitchEnable_PCI
#else
void SwitchEnable_ISA
#endif
(
    PHARDWARE  pHardware,
    BOOLEAN    fEnable
)
{

#ifdef TWO_NETWORK_INTERFACE

    /* Set Port 1 port-base vlan membership with Port 3 */
    HardwareConfigPortBaseVlan ( pHardware, 0, 0x05 );

    /* Set Port 2 port-base vlan membership with Port 3 */
    HardwareConfigPortBaseVlan ( pHardware, 1, 0x06 );
#endif

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_CTRL_BANK );
#endif

    /* High byte is read-only. */
    if (fEnable)
    {
        HW_WRITE_WORD( pHardware, REG_CHIP_ID_OFFSET, (UCHAR)SWITCH_START );
    }
    else
        HW_WRITE_WORD( pHardware, REG_CHIP_ID_OFFSET, 0 );

}
#endif /* #ifdef DEF_KS8842 */
