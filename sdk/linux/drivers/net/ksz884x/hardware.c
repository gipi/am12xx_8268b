/*---------------------------------------------------------------------------
          Copyright (c) 2003-2006 Micrel, Inc.  All rights reserved.
  ---------------------------------------------------------------------------

  hardware.c - Target independent hardware functions

  Author  Date      Version  Description
  THa     06/02/06           Add NO_STATS conditional.
  THa     04/06/06           Implement AT93C46 EEPROM access functions.
  THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                             some hardware platforms.
  THa     01/25/06           Add HardwareWriteIntMask and HardwareWriteIntStat
                             functions for 32-bit I/O access only.
  PCD     10/20/05  0.2.0    (1).HardwareSetup() includes enable WOL by magic packet.
                             (2).Added Wake-on-LAN APIs.
                             (3) incorrect register offset in HardwareClearMulticast(), HardwareSetGroupAddress().
  THa     10/06/05           Changed descriptor structure.
  PCD     04/07/05  0.1.1    don't enable "INT_RX_OVERRUN". It waste too much CPU time
                             to handle it if SmartBit send out continuced packets to target.
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


#include "target.h"
#include "hardware.h"
#include <asm/mach-am7x/actions_regs.h>
#include <asm/mach-am7x/actions_io.h>
#undef SOFTWARE_STP_SUPPORT


//#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);
#define ACT_DEBUG_	

UCHAR DEFAULT_MAC_ADDRESS[] = { 0x7c, 0xe7, 0x19, 0xa5, 0x13, 0x13 };
//UCHAR DEFAULT_MAC_ADDRESS[] = { 0x00, 0x50, 0xc2, 0x1e, 0xae, 0xfe };

/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
#ifndef INLINE
/*
    HardwareSelectBank

    Description:
        This routine changes the bank of registers and keeps track of current
        bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The new bank of registers.

    Return (None):
*/

void HardwareSelectBank (
    PHARDWARE pHardware,
    UCHAR     bBank )
{
#ifdef SH_16BIT_WRITE
    HW_WRITE_WORD( pHardware, (UCHAR)REG_BANK_SEL_OFFSET, bBank );

#else
    HW_WRITE_BYTE( pHardware, (UCHAR)REG_BANK_SEL_OFFSET, bBank );
#endif
    pHardware->m_bBank = bBank;
}  /* HardwareSelectBank */


/*
    HardwareReadRegByte

    Description:
        This routine reads a byte from specified bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        PUCHAR pbData
            Pointer to byte to store the data.

    Return (None):
*/

void HardwareReadRegByte (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUCHAR    pbData )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_READ_BYTE( pHardware, bOffset, pbData );
}  /* HardwareReadRegByte */


/*
    HardwareWriteRegByte

    Description:
        This routine writess a byte to specific bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        UCHAR bValue
            The data value.

    Return (None):
*/

void HardwareWriteRegByte (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    UCHAR     bValue )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_WRITE_BYTE( pHardware, bOffset, bValue );
}  /* HardwareWriteRegByte */


/*
    HardwareReadRegWord

    Description:
        This routine reads a word from specified bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        PUSHORT pwData
            Pointer to word to store the data.

    Return (None):
*/

void HardwareReadRegWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUSHORT   pwData )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_READ_WORD( pHardware, bOffset, pwData );
}  /* HardwareReadRegWord */


/*
    HardwareWriteRegWord

    Description:
        This routine writess a word to specific bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        USHORT wValue
            The data value.

    Return (None):
*/

void HardwareWriteRegWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    USHORT    wValue )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_WRITE_WORD( pHardware, bOffset, wValue );
}  /* HardwareWriteRegWord */

/*
    HardwareReadRegDWord

    Description:
        This routine reads a double word (32-bit) from specified bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        PULONG pwData
            Pointer to long to store the data.

    Return (None):
*/

void HardwareReadRegDWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    PULONG    pwData )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_READ_DWORD( pHardware, bOffset, pwData );
}  /* HardwareReadRegDWord */


/*
    HardwareWriteRegDWord

    Description:
        This routine writess a double word (32-bit) to specific bank and register.  It calls
        HardwareSelectBank if the bank is different than the current bank.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bBank
            The bank of registers.

        UCHAR bOffset
            The register offset.

        ULONG wValue
            The data value.

    Return (None):
*/

void HardwareWriteRegDWord (
    PHARDWARE pHardware,
    UCHAR     bBank,
    UCHAR     bOffset,
    ULONG     wValue )
{
    if ( bBank != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, bBank );
    }
    HW_WRITE_DWORD( pHardware, bOffset, wValue );
}  /* HardwareWriteRegDWord */


#ifdef SH_32BIT_ACCESS_ONLY
void HardwareWriteIntMask (
    PHARDWARE pHardware,
    ULONG     ulValue )
{
    if ( REG_INT_MASK_BANK != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, REG_INT_MASK_BANK );
    }
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, ulValue );
}  /* HardwareWriteIntMask */


void HardwareWriteIntStat (
    PHARDWARE pHardware,
    ULONG     ulValue )
{
    ULONG ulIntEnable;

    if ( REG_INT_STATUS_BANK != pHardware->m_bBank )
    {
        HardwareSelectBank( pHardware, REG_INT_STATUS_BANK );
    }
    HW_READ_DWORD( pHardware, REG_INT_MASK_OFFSET, &ulIntEnable );
    ulIntEnable &= 0x0000FFFF;
    ulIntEnable |= ulValue;
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, ulIntEnable );
}  /* HardwareWriteIntStat */
#endif

#endif  /* #ifndef INLINE */

/* -------------------------------------------------------------------------- */

/*
    HardwareReadBuffer

    Description:
        This routine is used to read data into a buffer if the operating system
        does not provide one.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bOffset
            The register offset.

        PULONG pdwData
            Pointer to word buffer.

        USHORT wLength
            The length of the buffer.

    Return (None):
*/

void HardwareReadBuffer (
    PHARDWARE pHardware,
    UCHAR     bOffset,
    PULONG    pdwData,
    int       length )
{
    int lengthInDWord = length >> 2;
    int remainLengthInByte = length % 4;

    HardwareSelectBank( pHardware, REG_DATA_BANK );

    while ( lengthInDWord--)
    {
        HW_READ_DWORD( pHardware, bOffset, pdwData++ );
    }

    if ( remainLengthInByte > 0 )
    {
        HW_READ_DWORD( pHardware, bOffset, pdwData++ );
    }

}  /* HardwareReadBuffer */



/*
    HardwareWriteBuffer

    Description:
        This routine is used to write data from a buffer if the operating system
        does not provide one.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bOffset
            The register offset.

        PULONG pdwData
            Pointer to word buffer.

        USHORT wLength
            The length of the buffer.

    Return (None):
*/

void HardwareWriteBuffer (
    PHARDWARE pHardware,
    UCHAR     bOffset,
    PULONG    pdwData,
    int       length )
{
    int lengthInDWord = length >> 2;
    int remainLengthInByte = length % 4;

    HardwareSelectBank( pHardware, REG_DATA_BANK );

    while ( lengthInDWord-- )
    {
        HW_WRITE_DWORD( pHardware, bOffset, *pdwData++ );
    }

    if ( remainLengthInByte > 0 )
    {
        HW_WRITE_DWORD( pHardware, bOffset, *pdwData++ );
    }

}  /* HardwareWriteBuffer */


#endif /* #ifdef KS_ISA_BUS */

/* -------------------------------------------------------------------------- */


#define PHY_STAT_SPEED_100MBIT      ( PORT_STAT_SPEED_100MBIT << 8 )
#define PHY_STAT_FULL_DUPLEX        ( PORT_STAT_FULL_DUPLEX << 8 )


#ifdef KS_PCI_BUS

/* PCI Bus Interface */

#define HW_READ_PHY_CTRL( phwi, phy, data )                                 \
    HW_READ_WORD( phwi, phy + REG_PHY_CTRL_OFFSET, &data )

#define HW_WRITE_PHY_CTRL( phwi, phy, data )                                \
    HW_WRITE_WORD( phwi, phy + REG_PHY_CTRL_OFFSET, data )

#define HW_READ_PHY_LINK_STATUS( phwi, phy, data )                          \
    HW_READ_WORD( phwi, phy + REG_PHY_STATUS_OFFSET, &data )

#define HW_READ_PHY_AUTO_NEG( phwi, phy, data )                             \
    HW_READ_WORD( phwi, phy + REG_PHY_AUTO_NEG_OFFSET, &data )

#define HW_WRITE_PHY_AUTO_NEG( phwi, phy, data )                            \
    HW_WRITE_WORD( phwi, phy + REG_PHY_AUTO_NEG_OFFSET, data )

#define HW_READ_PHY_REM_CAP( phwi, phy, data )                              \
    HW_READ_WORD( phwi, phy + REG_PHY_REMOTE_CAP_OFFSET, &data )

#define HW_READ_PHY_CROSSOVER( phwi, phy, data )                            \
    HW_READ_WORD( phwi, phy + REG_PHY_CTRL_OFFSET, &data )

#define HW_WRITE_PHY_CROSSOVER( phwi, phy, data )                           \
    HW_WRITE_WORD( phwi, phy + REG_PHY_CTRL_OFFSET, data )

#define HW_READ_PHY_POLARITY( phwi, phy, data )                             \
    HW_READ_WORD( phwi, phy + REG_PHY_PHY_CTRL_1_OFFSET, &data )

#define HW_READ_PHY_LINK_MD( phwi, phy, data )                              \
    HW_READ_WORD( phwi, phy + REG_PHY_LINK_MD_1_OFFSET, &data )

#define HW_WRITE_PHY_LINK_MD( phwi, phy, data )                             \
    HW_WRITE_WORD( phwi, phy + REG_PHY_LINK_MD_1_OFFSET, data )

#else

/* Generic Bus Interface */

#define HW_READ_PHY_CTRL( phwi, phy, data )                                 \
    HW_READ_WORD( phwi, REG_PHY_CTRL_OFFSET, &data )

#define HW_WRITE_PHY_CTRL( phwi, phy, data )                                \
    HW_WRITE_WORD( phwi, REG_PHY_CTRL_OFFSET, data )

#define HW_READ_PHY_LINK_STATUS( phwi, phy, data )                          \
    HW_READ_WORD( phwi, REG_PHY_STATUS_OFFSET, &data )

#define HW_READ_PHY_AUTO_NEG( phwi, phy, data )                             \
    HW_READ_WORD( phwi, REG_PHY_AUTO_NEG_OFFSET, &data )

#define HW_WRITE_PHY_AUTO_NEG( phwi, phy, data )                            \
    HW_WRITE_WORD( phwi, REG_PHY_AUTO_NEG_OFFSET, data )

#define HW_READ_PHY_REM_CAP( phwi, phy, data )                              \
    HW_READ_WORD( phwi, REG_PHY_REMOTE_CAP_OFFSET, &data )

#define HW_READ_PHY_CROSSOVER( phwi, phy, data )                            \
    HW_READ_WORD( phwi, REG_PHY_CTRL_OFFSET, &data )

#define HW_WRITE_PHY_CROSSOVER( phwi, phy, data )                           \
    HW_WRITE_WORD( phwi, REG_PHY_CTRL_OFFSET, data )

#define HW_READ_PHY_POLARITY( phwi, phy, data )                             \
    HW_READ_WORD( phwi, REG_PHY_PHY_CTRL_1_OFFSET + phy *                   \
        PHY_SPECIAL_INTERVAL, &data )

#define HW_READ_PHY_LINK_MD( phwi, phy, data )                              \
    HW_READ_WORD( phwi, REG_PHY_LINK_MD_1_OFFSET + phy *                    \
        PHY_SPECIAL_INTERVAL, &data )

#define HW_WRITE_PHY_LINK_MD( phwi, phy, data )                             \
    HW_WRITE_WORD( phwi, REG_PHY_LINK_MD_1_OFFSET + phy *                   \
        PHY_SPECIAL_INTERVAL, data )
#endif


#define PHY_RESET_TIMEOUT  10


/*
    HardwareReadPhy

    Description:
        This routine reads data from the PHY register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int port
            Port to read.

        USHORT wPhyReg
            PHY register to read.

        PUSHORT pwData
            Pointer to word to store the read data.

    Return (None):
*/

void HardwareReadPhy (
    PHARDWARE pHardware,
    int       port,
    USHORT    wPhyReg,
    PUSHORT   pwData )
{
#ifdef KS_PCI_BUS
    int phy;

    phy = REG_PHY_1_CTRL_OFFSET + port * PHY_CTRL_INTERVAL + wPhyReg;
    HW_READ_WORD( pHardware, phy, pwData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + port *
        PHY_BANK_INTERVAL ));
    HW_READ_WORD( pHardware, wPhyReg, pwData );
#endif
}  /* HardwareReadPhy */


/*
    HardwareWritePhy

    Description:
        This routine writes data to the PHY register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int port
            Port to write.

        USHORT wPhyReg
            PHY register to write.

        USHORT wData
            Word data to write.

    Return (None):
*/

void HardwareWritePhy (
    PHARDWARE pHardware,
    int       port,
    USHORT    wPhyReg,
    USHORT    wRegData )
{
#ifdef KS_PCI_BUS
    int phy;

    phy = REG_PHY_1_CTRL_OFFSET + port * PHY_CTRL_INTERVAL + wPhyReg;
    HW_WRITE_WORD( pHardware, phy, wRegData );

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + port *
        PHY_BANK_INTERVAL ));
    HW_WRITE_WORD( pHardware, wPhyReg, wRegData );
#endif
}  /* HardwareWritePhy */


#if 0
/*
    HardwareSetupPhy

    Description:
        This routine setup the PHY registers for proper operation and
        determines the initial link status after auto negotiation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

static
void HardwareSetupPhy (
    PHARDWARE pHardware )
{
    USHORT RegData;
    USHORT wStatus;


    {
        UCHAR bStatus;
        UCHAR bPhy = 0;
        int   phy;

#ifdef KS_PCI_BUS
        phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
        phy = bPhy;
        HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
            PHY_BANK_INTERVAL ));
#endif /* #ifdef KS_PCI_BUS */

        HW_READ_PHY_LINK_STATUS( pHardware, phy, RegData );
        PortConfigReadByte( pHardware, bPhy,
#ifdef KS_ISA_BUS
                            REG_PORT_LINK_STATUS_BANK,
#endif
                            REG_PORT_STATUS_HI_OFFSET, &bStatus );
        wStatus = bStatus << 8;
    }

    /* Default 10 MBit. */
    pHardware->m_ulTransmitRate = 100000;
    pHardware->m_ulDuplex = 1;
    if ( ( RegData & PHY_LINK_STATUS ) )
    {
        pHardware->m_ulHardwareState = MediaStateConnected;
        if ( ( wStatus & PHY_STAT_SPEED_100MBIT ) )
        {
            pHardware->m_ulTransmitRate = 1000000;
        }
        if ( ( wStatus & PHY_STAT_FULL_DUPLEX ) )
        {
            pHardware->m_ulDuplex = 2;
        }
    }
    else
        pHardware->m_ulHardwareState = MediaStateDisconnected;

#ifdef DBG
    DBG_PRINT( "link status: %04X; %d, %d, %d"NEWLINE, wStatus,
        ( int ) pHardware->m_ulHardwareState,
        ( int ) pHardware->m_ulTransmitRate,
        ( int ) pHardware->m_ulDuplex);
#endif
}  /* HardwareSetupPhy */
#endif

/* -------------------------------------------------------------------------- */

#if defined( KS_ISA_BUS )  &&  defined( DBG )
void DisplayRegisters (
    PHARDWARE pHardware )
{
    UCHAR b;
    UCHAR i;

#ifdef UNDER_CE
    pHardware->reg[ 1 ][ 15 ] = 1;
    for ( b = 4; b < 16; b++ )
        pHardware->reg[ b ][ 15 ] = 1;
    for ( b = 20; b < 32; b++ )
        pHardware->reg[ b ][ 15 ] = 1;
    for ( b = 0; b < 54; b++ )
    {
        if ( pHardware->reg[ b ][ 15 ] )
            continue;
        HardwareSelectBank( pHardware, b );
        for ( i = 0; i < 0x0E; i += 1 )
        {
            HW_READ_BYTE( pHardware, i, &pHardware->reg[ b ][ i ]);
        }
    }
    for ( b = 0; b < 54; b++ )
    {
        if ( pHardware->reg[ b ][ 15 ] )
            continue;
DBG_PRINT( "%2d: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x"NEWLINE,
        b,
        pHardware->reg[ b ][ 0 ], pHardware->reg[ b ][ 1 ],
        pHardware->reg[ b ][ 2 ], pHardware->reg[ b ][ 3 ],
        pHardware->reg[ b ][ 4 ], pHardware->reg[ b ][ 5 ],
        pHardware->reg[ b ][ 6 ], pHardware->reg[ b ][ 7 ],
        pHardware->reg[ b ][ 8 ], pHardware->reg[ b ][ 9 ],
        pHardware->reg[ b ][ 10 ], pHardware->reg[ b ][ 11 ],
        pHardware->reg[ b ][ 12 ], pHardware->reg[ b ][ 13 ]);
    }

#else
    USHORT RegData;
    UCHAR  bBank[ 54 ];

    memset( bBank, 0, 54 );
    bBank[ 1 ] = 1;
    for ( b = 4; b < 16; b++ )
        bBank[ b ] = 1;
    for ( b = 20; b < 32; b++ )
        bBank[ b ] = 1;
    for ( b = 0; b < 54; b++ )
    {
        if ( bBank[ b ] )
            continue;
        DBG_PRINT( "bank %2d: ", b );
        for ( i = 0; i < 0x0E; i += 2 )
        {
            HardwareReadRegWord( pHardware, b, i, &RegData );
            DBG_PRINT( "%04X ", RegData );
        }
        DBG_PRINT( NEWLINE );
    }
#endif
}  /* DisplayRegisters */
#endif

/* -------------------------------------------------------------------------- */

#define AT93C_CODE    0
#define AT93C_WR_OFF  0x00
#define AT93C_WR_ALL  0x10
#define AT93C_ER_ALL  0x20
#define AT93C_WR_ON   0x30

#define AT93C_WRITE   1
#define AT93C_READ    2
#define AT93C_ERASE   3

#define EEPROM_DELAY  4


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
void DropGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    data &= ~gpio;
    HW_WRITE_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, data );
}  /* DropGPIO */


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
void RaiseGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    data |= gpio;
    HW_WRITE_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, data );
}  /* RaiseGPIO */


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
UCHAR StateGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    return(( UCHAR )( data & gpio ));
}  /* StateGPIO */


#define EEPROM_CLOCK( pHardware )                                           \
{                                                                           \
    RaiseGPIO( pHardware, EEPROM_SERIAL_CLOCK );                            \
    DelayMicrosec( EEPROM_DELAY );                                          \
    DropGPIO( pHardware, EEPROM_SERIAL_CLOCK );                             \
    DelayMicrosec( EEPROM_DELAY );                                          \
}


static
USHORT SPIMRead (
    PHARDWARE pHardware )
{
    int    i;
    USHORT bTemp = 0;

    for ( i = 15; i >= 0; i-- ) {
        RaiseGPIO( pHardware, EEPROM_SERIAL_CLOCK );
        DelayMicrosec( EEPROM_DELAY );

        bTemp |= ( StateGPIO( pHardware, EEPROM_DATA_IN )) ? 1 << i : 0;

        DropGPIO( pHardware, EEPROM_SERIAL_CLOCK );
        DelayMicrosec( EEPROM_DELAY );
    }
    return bTemp;
}  /* SPIMRead */


static
void SPIMWrite (
    PHARDWARE pHardware,
    USHORT    Data )
{
    int i;

    for ( i = 15; i >= 0; i-- ) {
        ( Data & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }
}  /* SPIMWrite */


static
void SPIMReg (
    PHARDWARE pHardware,
    UCHAR     Data,
    UCHAR     Reg )
{
    int i;

    /* Initial start bit */
    RaiseGPIO( pHardware, EEPROM_DATA_OUT );
    EEPROM_CLOCK( pHardware );

    /* AT93C operation */
    for ( i = 1; i >= 0; i-- ) {
        ( Data & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }

    /* Address location */
    for ( i = 5; i >= 0; i-- ) {
        ( Reg & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }
}  /* SPIMReg */


/*
    EepromReadWord

    Description:
        This function reads a word from the AT93C46 EEPROM.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR Reg
            The register offset.

    Return (USHORT):
        The data value.
*/

USHORT EepromReadWord (
    PHARDWARE pHardware,
    UCHAR     Reg )
{
    USHORT Data;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_EEPROM_CTRL_BANK );
#endif
    RaiseGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    SPIMReg( pHardware, AT93C_READ, Reg );
    Data = SPIMRead( pHardware );

    DropGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    return( Data );
}  /* EepromReadWord */


/*
    EepromWriteWord

    Description:
        This procedure writes a word to the AT93C46 EEPROM.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR Reg
            The register offset.

        USHORT Data
            The data value.

    Return (None):
*/

void EepromWriteWord (
    PHARDWARE pHardware,
    UCHAR     Reg,
    USHORT    Data )
{
    int timeout;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_EEPROM_CTRL_BANK );
#endif
    RaiseGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    /* Enable write. */
    SPIMReg( pHardware, AT93C_CODE, AT93C_WR_ON );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Erase the register. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_ERASE, Reg );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Check operation complete. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    timeout = 8;
    DelayMillisec( 2 );
    do {
        DelayMillisec( 1 );
    } while ( !StateGPIO( pHardware, EEPROM_DATA_IN )  &&  --timeout );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Write the register. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_WRITE, Reg );
    SPIMWrite( pHardware, Data );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Check operation complete. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    timeout = 8;
    DelayMillisec( 2 );
    do {
        DelayMillisec( 1 );
    } while ( !StateGPIO( pHardware, EEPROM_DATA_IN )  &&  --timeout );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Disable write. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_CODE, AT93C_WR_OFF );

    DropGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );
}  /* EepromWriteWord */

/* -------------------------------------------------------------------------- */

/*
    HardwareInitialize

    Description:
        This function checks the hardware is correct for this driver and resets
        the hardware for proper initialization.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/
#ifdef KS_PCI_BUS
BOOLEAN HardwareInitialize_PCI
#else
BOOLEAN HardwareInitialize_ISA
#endif
(
    PHARDWARE pHardware )
{
	
#if CONFIG_AM_CHIP_ID == 1211
	extern void mem_dump_long(void *, int);
	volatile u32 *reg;
    
    /* Enable MFP NOR_CEB1 */
	reg  = (volatile u32*)(0xb01c004c); 
	*reg = (*reg & 0xffffff0f)|(0x4<<4);
    
    /* Enable MFP EXT_INT1 */
    reg  = (volatile u32*)(0xb01c0054); 
	*reg = (*reg & 0x0fffffff)|(0x5<<28); 
	
	/* Enable EXT INT1 */
	reg  =(volatile u32*)(0xb0020014); 
	*reg = 0x03000000;

	mem_dump_long(0xb01c0040, 24);
	return TRUE;
#endif

#if 0	
    USHORT RegData;

    /*
     * Set Bus Speed to 125MHz
     */

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_BUS_CTRL_BANK );
#endif

    HW_WRITE_WORD ( pHardware, REG_BUS_CTRL_OFFSET, BUS_SPEED_125_MHZ );

    /*
     * Check ks884x Chip ID
     */

    {

#ifdef KS_ISA_BUS
        HardwareReadRegWord( pHardware,
                             (UCHAR)REG_SWITCH_CTRL_BANK,
                             (UCHAR)REG_CHIP_ID_OFFSET,
                             (PUSHORT)&RegData
                           );
#else
        HW_READ_WORD( pHardware, REG_CHIP_ID_OFFSET, &RegData );

#endif

#ifdef DBG
    DBG_PRINT( "id: %X"NEWLINE, RegData );
#endif

#ifdef DEF_KS8841
        if ( ( RegData & SWITCH_CHIP_ID_MASK_41 ) != REG_CHIP_ID_41 )
#else
        if ( ( RegData & SWITCH_CHIP_ID_MASK_41 ) != REG_CHIP_ID_42 )
#endif
        {

#ifdef DEBUG_COUNTER
            pHardware->m_nBad[ COUNT_BAD_CMD_WRONG_CHIP ]+=1;
#endif
            return FALSE;
        }

#if defined( KS_ISA_BUS )  &&  defined( DBG_ )
        DisplayRegisters( pHardware );
#endif

        return( TRUE );
    }

#ifdef DEBUG_COUNTER
    pHardware->m_nBad[ COUNT_BAD_CMD_INITIALIZE ]+=1;
#endif

    return FALSE;
#endif
}  /* HardwareInitialize */

/*
    HardwareReadChipID

    Description:
        This function read family\chip ID, and device revision ID.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return: none
*/

void HardwareReadChipID
(
    PHARDWARE pHardware,
    PUSHORT   pChipID,
    PUCHAR    pDevRevisionID
)
{
    USHORT RegData;

    /* Read ks884x Chip ID */

#ifdef KS_ISA_BUS
    HardwareReadRegWord( pHardware,
                         (UCHAR)REG_SWITCH_CTRL_BANK,
                         (UCHAR)REG_CHIP_ID_OFFSET,
                         (PUSHORT)&RegData
                       );
#else
    HW_READ_WORD( pHardware, REG_CHIP_ID_OFFSET, &RegData );
#endif


    *pChipID        = RegData & SWITCH_CHIP_ID_MASK;
    *pDevRevisionID = RegData & SWITCH_REVISION_MASK;

}  /* HardwareReadChipID */


/*
    HardwareReset

    Description:
        This function resets the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareReset_PCI
#else
BOOLEAN HardwareReset_ISA
#endif
(
    PHARDWARE pHardware )
{

    /* Write 1 to reset device */
#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware, REG_GLOBAL_CTRL_BANK,
        REG_GLOBAL_CTRL_OFFSET, GLOBAL_SOFTWARE_RESET );
#endif

#ifdef KS_PCI_BUS
    HW_WRITE_WORD( pHardware,
        REG_GLOBAL_CTRL_OFFSET, GLOBAL_SOFTWARE_RESET );
#endif

    /* Wait for device to reset */
    DelayMillisec( 10 );

    /* Write 0 to clear device reset */
#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware, REG_GLOBAL_CTRL_BANK,
        REG_GLOBAL_CTRL_OFFSET, 0 );
#endif

#ifdef KS_PCI_BUS
    HW_WRITE_WORD( pHardware,
        REG_GLOBAL_CTRL_OFFSET, 0 );
#endif

#ifdef DEBUG_COUNTER
    pHardware->m_nGood[ COUNT_GOOD_CMD_RESET ]+=1;
#endif
    return TRUE;
}  /* HardwareReset */


#ifdef KS_ISA_BUS
/*
    HardwareSetBurst

    Description:
        This function is to setup Burst mode with its burst length.
        Note: The burst length should be the same as Host's burst length.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR  wBurstLength
            burst length (0, 4, 8,16).

    Return (BOOLEAN):
        TRUE successful; otherwise, FALSE.
*/

BOOLEAN HardwareSetBurst (
    PHARDWARE pHardware,
    UCHAR     bBurstLength )
{
    USHORT     RegData;


    switch (bBurstLength)
    {
        case 0:
            RegData = BURST_LENGTH_0 ;
            break;
        case 4:
            RegData = BURST_LENGTH_4 ;
            break;
        case 8:
            RegData = BURST_LENGTH_8 ;
            break;
        case 16:
            RegData = BURST_LENGTH_16 ;
            break;
        default:
            return (FALSE);
    }

    HardwareWriteRegWord( pHardware, REG_BUS_BURST_BANK, REG_BUS_BURST_OFFSET,
                          RegData );
    return (TRUE);

}  /* HardwareSetBurst */

#endif /* #ifdef KS_ISA_BUS */


/*
    HardwareSetup

    Description:
        This routine setup the hardware for proper operation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareSetup_PCI
#else
void HardwareSetup_ISA
#endif
(
    PHARDWARE pHardware )
{
#if defined( DEF_KS8841 ) && ( defined( EARLY_TRANSMIT ) || defined( EARLY_RECEIVE ))
    USHORT RegData;
#endif

    /*
     * Initialize Tx\Rx control setting.
     */

    /* KS_PCI_BUS */

#ifdef KS_PCI_BUS

    /* Setup Transmit Control */

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup_PCI(): Initialize Tx/Rx control setting"NEWLINE );
#endif

    pHardware->m_dwTransmitConfig = ( DMA_TX_CTRL_PAD_ENABLE | DMA_TX_CTRL_CRC_ENABLE |
                                     (PBL_DEFAULT << 24) | DMA_TX_CTRL_ENABLE );
#if FLOWCONTROL_DEFAULT
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_FLOW_ENABLE;
#else
    pHardware->m_dwTransmitConfig &= ~DMA_TX_CTRL_FLOW_ENABLE;
#endif

#if TXCHECKSUM_DEFAULT
    /* Hardware cannot handle UDP packet in IP fragments. */
    pHardware->m_dwTransmitConfig |= (DMA_TX_CTRL_CSUM_TCP | DMA_TX_CTRL_CSUM_IP);
#else
    pHardware->m_dwTransmitConfig &= ~(DMA_TX_CTRL_CSUM_UDP | DMA_TX_CTRL_CSUM_TCP | DMA_TX_CTRL_CSUM_IP);
#endif /* TXCHECKSUM_DEFAULT */

#if 0
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_LOOPBACK;
#endif

    /* Setup Receive Control */

    pHardware->m_dwReceiveConfig = ( DMA_RX_CTRL_BROADCAST | DMA_RX_CTRL_UNICAST |
                                    (PBL_DEFAULT << 24) | DMA_RX_CTRL_ENABLE );

#if FLOWCONTROL_DEFAULT
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_FLOW_ENABLE;
#else
    pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_FLOW_ENABLE;
#endif

#if RXCHECKSUM_DEFAULT
    /* Hardware cannot handle UDP packet in IP fragments. */
    pHardware->m_dwReceiveConfig |= (DMA_RX_CTRL_CSUM_TCP | DMA_RX_CTRL_CSUM_IP);
#else
    pHardware->m_dwReceiveConfig &= ~(DMA_RX_CTRL_CSUM_UDP | DMA_RX_CTRL_CSUM_TCP | DMA_RX_CTRL_CSUM_IP);
#endif
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_MULTICAST;

    if ( pHardware->m_bAllMulticast )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ALL_MULTICAST;
    if ( pHardware->m_bPromiscuous )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_PROMISCUOUS;

#ifdef CHECK_RCV_ERRORS
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ERROR;
#endif

#else /* #ifdef KS_ISA_BUS  */

    /* KS_ISA_BUS  */


    /* Setup Transmit Control */

    pHardware->m_wTransmitConfig = TX_CTRL_PAD_ENABLE | TX_CTRL_ENABLE;

#if 0
    pHardware->m_wTransmitConfig |= TX_CTRL_EPH_LOOPBACK;
#endif

    pHardware->m_wTransmitConfig |= ( TX_CTRL_CRC_ENABLE | TX_CTRL_FLOW_ENABLE );


    /* Setup Transmit Frame Data Pointer Auto-Increment */
    HardwareWriteRegWord( pHardware, REG_TX_ADDR_PTR_BANK,
        REG_TX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );


  #if defined (EARLY_TRANSMIT) && defined(DEF_KS8841)

    /* Setup Early Transmit function */
    pHardware->m_wTransmitThreshold = EARLY_TX_MULTIPLE;

    HardwareReadRegWord( pHardware, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET,
                        &RegData );
    RegData &= ~EARLY_TX_THRESHOLD;
    RegData |= pHardware->m_wTransmitThreshold / EARLY_TX_MULTIPLE;
    RegData |= EARLY_TX_ENABLE;

    HardwareWriteRegWord( pHardware, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET,
                          RegData );

  #endif /* #ifdef EARLY_TRANSMIT */



    /* Setup Receive Control */

    pHardware->m_wReceiveConfig = RX_CTRL_STRIP_CRC | RX_CTRL_ENABLE;
    pHardware->m_wReceiveConfig |= ( RX_CTRL_UNICAST | RX_CTRL_BROADCAST | RX_CTRL_FLOW_ENABLE );
    pHardware->m_wReceiveConfig |= RX_CTRL_MULTICAST;

    if ( pHardware->m_bAllMulticast )
        pHardware->m_wReceiveConfig |= RX_CTRL_ALL_MULTICAST;
    if ( pHardware->m_bPromiscuous )
        pHardware->m_wReceiveConfig |= RX_CTRL_PROMISCUOUS;

    /* Setup Receive Frame Data Pointer Auto-Increment */
    HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
                          REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 0 );

    /* Setup Receive High Water Mark to 2KBytes to avoid loss packets (big packet size) under flow control */
    HardwareWriteRegWord( pHardware, REG_RX_WATERMARK_BANK,
                          REG_RX_WATERMARK_OFFSET, RX_HIGH_WATERMARK_2KB );

  #if defined (EARLY_RECEIVE) && defined(DEF_KS8841)

    /* Setup Early Receive function */
    pHardware->m_wReceiveThreshold = EARLY_RX_MULTIPLE;

    HardwareReadRegWord( pHardware, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET,
                         &RegData );

    RegData &= ~EARLY_RX_THRESHOLD;
    RegData |= pHardware->m_wReceiveThreshold / EARLY_RX_MULTIPLE;
    RegData |= EARLY_RX_ENABLE;

    HardwareWriteRegWord( pHardware, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET,
                          RegData );

  #endif /* #if defined ( EARLY_RECEIVE ) && defined( DEF_KS8841 )  */




#endif /* #ifdef KS_PCI_BUS */

    /*
     * Initialize Port control setting.
     */

    SwitchSetGlobalControl( pHardware );

    /* Enable WOL by detection of magic packet */
    HardwareEnableWolMagicPacket ( pHardware );
    HardwareClearWolPMEStatus ( pHardware );

}  /* HardwareSetup */


/*
    HardwareSwitchSetup

    Description:
        This routine setup the hardware Switch engine for default operation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSwitchSetup
(
    PHARDWARE pHardware )
{
#ifdef DEF_KS8842
    UCHAR  bPort;
#endif

    /*
     * Initialize Port control setting.
     */

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchSetLinkSpeed()"NEWLINE );
#endif
    SwitchSetLinkSpeed( pHardware );

#ifdef DEF_KS8842

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchInitBroadcastStorm()"NEWLINE );
#endif
    /* Enable Switch Ports broacast storm protection at 10% percent rate */
    SwitchInitBroadcastStorm( pHardware );
    HardwareConfigBroadcastStorm( pHardware, BROADCAST_STORM_PROTECTION_RATE );
    for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
        SwitchEnableBroadcastStorm( pHardware, bPort );

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchInitPriority()"NEWLINE );
#endif
    SwitchInitPriority( pHardware );

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchInitMirror()"NEWLINE );
#endif
    SwitchInitMirror( pHardware );

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchInitPriorityRate()"NEWLINE );
#endif
    SwitchInitPriorityRate( pHardware );

#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling SwitchInitVlan()"NEWLINE );
#endif
    SwitchInitVlan( pHardware );

#ifdef SOFTWARE_STP_SUPPORT
#ifdef DEBUG_HARDWARE_SETUP
    DBG_PRINT( "HardwareSetup(): calling HardwareInit_STP()"NEWLINE );
#endif
    HardwareInit_STP( pHardware );
#endif

    SwitchEnable( pHardware, TRUE );

#endif  /* #ifdef DEF_KS8842 */

}  /* HardwareSwitchSetup */


#ifdef KS_PCI_BUS
void HardwareSetupFunc_PCI
#else
void HardwareSetupFunc_ISA
#endif
(
    struct hw_fn* ks8842_fn )
{
#ifdef KS_PCI_BUS
    ks8842_fn->m_fPCI = TRUE;

#ifdef DEF_KS8842
    ks8842_fn->fnSwitchDisableMirrorSniffer =
        SwitchDisableMirrorSniffer_PCI;
    ks8842_fn->fnSwitchEnableMirrorSniffer =
        SwitchEnableMirrorSniffer_PCI;
    ks8842_fn->fnSwitchDisableMirrorReceive =
        SwitchDisableMirrorReceive_PCI;
    ks8842_fn->fnSwitchEnableMirrorReceive =
        SwitchEnableMirrorReceive_PCI;
    ks8842_fn->fnSwitchDisableMirrorTransmit =
        SwitchDisableMirrorTransmit_PCI;
    ks8842_fn->fnSwitchEnableMirrorTransmit =
        SwitchEnableMirrorTransmit_PCI;
    ks8842_fn->fnSwitchDisableMirrorRxAndTx =
        SwitchDisableMirrorRxAndTx_PCI;
    ks8842_fn->fnSwitchEnableMirrorRxAndTx =
        SwitchEnableMirrorRxAndTx_PCI;

    ks8842_fn->fnHardwareConfig_TOS_Priority =
        HardwareConfig_TOS_Priority_PCI;
    ks8842_fn->fnSwitchDisableDiffServ =
        SwitchDisableDiffServ_PCI;
    ks8842_fn->fnSwitchEnableDiffServ =
        SwitchEnableDiffServ_PCI;

    ks8842_fn->fnHardwareConfig802_1P_Priority =
        HardwareConfig802_1P_Priorit_PCI;
    ks8842_fn->fnSwitchDisable802_1P =
        SwitchDisable802_1P_PCI;
    ks8842_fn->fnSwitchEnable802_1P =
        SwitchEnable802_1P_PCI;
    ks8842_fn->fnSwitchDisableDot1pRemapping =
        SwitchDisableDot1pRemapping_PCI;
    ks8842_fn->fnSwitchEnableDot1pRemapping =
        SwitchEnableDot1pRemapping_PCI;

    ks8842_fn->fnSwitchConfigPortBased =
        SwitchConfigPortBased_PCI;

    ks8842_fn->fnSwitchDisableMultiQueue =
        SwitchDisableMultiQueue_PCI;
    ks8842_fn->fnSwitchEnableMultiQueue =
        SwitchEnableMultiQueue_PCI;

    ks8842_fn->fnSwitchDisableBroadcastStorm =
        SwitchDisableBroadcastStorm_PCI;
    ks8842_fn->fnSwitchEnableBroadcastStorm =
        SwitchEnableBroadcastStorm_PCI;
    ks8842_fn->fnHardwareConfigBroadcastStorm =
        HardwareConfigBroadcastStorm_PCI;

    ks8842_fn->fnSwitchDisablePriorityRate =
        SwitchDisablePriorityRate_PCI;
    ks8842_fn->fnSwitchEnablePriorityRate =
        SwitchEnablePriorityRate_PCI;

    ks8842_fn->fnHardwareConfigRxPriorityRate =
        HardwareConfigRxPriorityRate_PCI;
    ks8842_fn->fnHardwareConfigTxPriorityRate =
        HardwareConfigTxPriorityRate_PCI;

    ks8842_fn->fnPortSet_STP_State =
        PortSet_STP_State_PCI;

    ks8842_fn->fnSwitchEnableVlan =
        SwitchEnableVlan_PCI;
#endif

    ks8842_fn->fnPortReadMIBCounter =
        PortReadMIBCounter_PCI;
    ks8842_fn->fnPortReadMIBPacket =
        PortReadMIBPacket_PCI;

#else
    ks8842_fn->m_fPCI = FALSE;

#ifdef DEF_KS8842
    ks8842_fn->fnSwitchDisableMirrorSniffer =
        SwitchDisableMirrorSniffer_ISA;
    ks8842_fn->fnSwitchEnableMirrorSniffer =
        SwitchEnableMirrorSniffer_ISA;
    ks8842_fn->fnSwitchDisableMirrorReceive =
        SwitchDisableMirrorReceive_ISA;
    ks8842_fn->fnSwitchEnableMirrorReceive =
        SwitchEnableMirrorReceive_ISA;
    ks8842_fn->fnSwitchDisableMirrorTransmit =
        SwitchDisableMirrorTransmit_ISA;
    ks8842_fn->fnSwitchEnableMirrorTransmit =
        SwitchEnableMirrorTransmit_ISA;
    ks8842_fn->fnSwitchDisableMirrorRxAndTx =
        SwitchDisableMirrorRxAndTx_ISA;
    ks8842_fn->fnSwitchEnableMirrorRxAndTx =
        SwitchEnableMirrorRxAndTx_ISA;

    ks8842_fn->fnHardwareConfig_TOS_Priority =
        HardwareConfig_TOS_Priority_ISA;
    ks8842_fn->fnSwitchDisableDiffServ =
        SwitchDisableDiffServ_ISA;
    ks8842_fn->fnSwitchEnableDiffServ =
        SwitchEnableDiffServ_ISA;

    ks8842_fn->fnHardwareConfig802_1P_Priority =
        HardwareConfig802_1P_Priorit_ISA;
    ks8842_fn->fnSwitchDisable802_1P =
        SwitchDisable802_1P_ISA;
    ks8842_fn->fnSwitchEnable802_1P =
        SwitchEnable802_1P_ISA;
    ks8842_fn->fnSwitchDisableDot1pRemapping =
        SwitchDisableDot1pRemapping_ISA;
    ks8842_fn->fnSwitchEnableDot1pRemapping =
        SwitchEnableDot1pRemapping_ISA;

    ks8842_fn->fnSwitchConfigPortBased =
        SwitchConfigPortBased_ISA;

    ks8842_fn->fnSwitchDisableMultiQueue =
        SwitchDisableMultiQueue_ISA;
    ks8842_fn->fnSwitchEnableMultiQueue =
        SwitchEnableMultiQueue_ISA;

    ks8842_fn->fnSwitchDisableBroadcastStorm =
        SwitchDisableBroadcastStorm_ISA;
    ks8842_fn->fnSwitchEnableBroadcastStorm =
        SwitchEnableBroadcastStorm_ISA;
    ks8842_fn->fnHardwareConfigBroadcastStorm =
        HardwareConfigBroadcastStorm_ISA;

    ks8842_fn->fnSwitchDisablePriorityRate =
        SwitchDisablePriorityRate_ISA;
    ks8842_fn->fnSwitchEnablePriorityRate =
        SwitchEnablePriorityRate_ISA;

    ks8842_fn->fnHardwareConfigRxPriorityRate =
        HardwareConfigRxPriorityRate_ISA;
    ks8842_fn->fnHardwareConfigTxPriorityRate =
        HardwareConfigTxPriorityRate_ISA;

    ks8842_fn->fnPortSet_STP_State =
        PortSet_STP_State_ISA;

    ks8842_fn->fnSwitchEnableVlan =
        SwitchEnableVlan_ISA;
#endif

    ks8842_fn->fnPortReadMIBCounter =
        PortReadMIBCounter_ISA;
    ks8842_fn->fnPortReadMIBPacket =
        PortReadMIBPacket_ISA;
#endif
}  /* HardwareSetupFunc */


/*
    HardwareSetupInterrupt

    Description:
        This routine setup the interrupt mask for proper operation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareSetupInterrupt_PCI
#else
void HardwareSetupInterrupt_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    pHardware->m_ulInterruptMask = INT_MASK;
    pHardware->m_ulInterruptMask |= INT_PHY;
    pHardware->m_ulInterruptMask |= INT_RX_OVERRUN;

#else
    pHardware->m_wInterruptMask = INT_MASK;
    pHardware->m_wInterruptMask |= INT_PHY;

#ifdef DBG
    pHardware->m_wInterruptMask |= INT_RX_ERROR;
#endif

#if defined (EARLY_RECEIVE) && defined(DEF_KS8841)
    pHardware->m_wInterruptMask |= ( INT_RX_EARLY );
#endif

#if defined (EARLY_TRANSMIT) && defined(DEF_KS8841)
    pHardware->m_wInterruptMask |= INT_TX_UNDERRUN;
#endif

#if defined( DEF_LINUX ) && defined( DBG )
    pHardware->m_wInterruptMask |= INT_RX_OVERRUN;
#endif

    /* Acknowledge PHY interrupt after PHY reset. */
    HardwareAcknowledgeInterrupt( pHardware, INT_PHY );
#endif
}  /* HardwareSetupInterrupt */


/*
    HardwareClearCounters

    Description:
        This routine resets all hardware counters to zero.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )

void HardwareClearCounters (
    PHARDWARE pHardware )
{
    memset(( void* ) pHardware->m_cnCounter, 0,
        SWITCH_PORT_NUM * ( sizeof( ULONGLONG ) * OID_COUNTER_LAST ) );

    PortInitCounters( pHardware, pHardware->m_bPortSelect );
}  /* HardwareClearCounters */



#endif

/* -------------------------------------------------------------------------- */

#ifdef KS_PCI_BUS
#ifdef DBG
void CheckDescriptors (
    PTDescInfo pInfo )
{
    int    i;
    PTDesc pDesc = pInfo->pRing;

    DBG_PRINT( "  %p: %d, %d, %d"NEWLINE, pInfo->phwRing, pInfo->cnAvail,
        pInfo->iLast, pInfo->iNext );
    for ( i = 0; i < pInfo->cnAlloc; i++ )
    {
        DBG_PRINT( "%08lx %08lx %08lx %08lx; %08lx %08lx"NEWLINE,
            pDesc->phw->Control.ulData,
            pDesc->phw->BufSize.ulData,
            pDesc->phw->ulBufAddr,
            pDesc->phw->ulNextPtr,
            pDesc->sw.Control.ulData,
            pDesc->sw.BufSize.ulData );
        pDesc++;
    }
}  /* CheckDescriptors */
#endif


void CheckDescriptorNum (
    PTDescInfo pInfo )
{
    int cnAlloc = pInfo->cnAlloc;
    int nShift;

    nShift = 0;
    while ( !( cnAlloc & 1 ) )
    {
        nShift++;
        cnAlloc >>= 1;
    }
    if ( cnAlloc != 1  ||  nShift < 2 )
    {
        DBG_PRINT( "Hardware descriptor numbers not right!"NEWLINE );
        while ( cnAlloc )
        {
            nShift++;
            cnAlloc >>= 1;
        }
        if ( nShift < 2 )
        {
            nShift = 2;
        }
        cnAlloc = 1 << nShift;
        pInfo->cnAlloc = cnAlloc;
    }
    pInfo->iMax = pInfo->cnAlloc - 1;
}  /* CheckDescriptorNum */


void HardwareInitDescriptors (
    PTDescInfo pDescInfo,
    int        fTransmit )
{
    int       i;
    ULONG     ulPhysical = pDescInfo->ulRing;
    PTHw_Desc pDesc = pDescInfo->phwRing;
    PTDesc    pCurrent = pDescInfo->pRing;
    PTDesc    pPrevious = NULL;

#ifdef CHECK_OVERRUN
    int       check = ( pDescInfo->cnAlloc * 15 ) / 16;
#endif

    for ( i = 0; i < pDescInfo->cnAlloc; i++ )
    {
        pCurrent->phw = pDesc++;
        ulPhysical += pDescInfo->nSize;
        if ( fTransmit )
        {
#if TXCHECKSUM_DEFAULT
            /* Hardware cannot handle UDP packet in IP fragments. */
            pCurrent->sw.BufSize.ulData =
                ( DESC_TX_CSUM_GEN_TCP | DESC_TX_CSUM_GEN_IP );
#endif
        }
        pPrevious = pCurrent++;
        pPrevious->phw->ulNextPtr = CPU_TO_LE32( ulPhysical );

#ifdef CHECK_OVERRUN
        pPrevious->pCheck = &pDescInfo->phwRing[(( i + check ) &
            pDescInfo->iMax )];
#endif
    }
    pPrevious->phw->ulNextPtr = CPU_TO_LE32( pDescInfo->ulRing );
    pPrevious->sw.BufSize.rx.fEndOfRing = TRUE;
    pPrevious->phw->BufSize.ulData =
        CPU_TO_LE32( pPrevious->sw.BufSize.ulData );

    pDescInfo->cnAvail = pDescInfo->cnAlloc;
    pDescInfo->iLast = pDescInfo->iNext = 0;

    pDescInfo->pCurrent = pDescInfo->pRing;
}  /* HardwareInitDescriptors */


/*
 * HardwareSetDescriptorBase
 *	This function sets MAC address to given type (WAN, LAN or HPHA)
 *
 * Argument(s)
 *  pHardware        Pointer to hardware instance.
 *  pTxDescBaseAddr	 pointer to base address of Tx descripotr.
 *  pRxDescBaseAddr	 pointer to base address of Rx descripotr.
 *
 * Return(s)
 *	NONE.
 */
void HardwareSetDescriptorBase
(
    PHARDWARE  pHardware,
    ULONG      TxDescBaseAddr,
    ULONG      RxDescBaseAddr
)
{
    /* Set base address of Tx/Rx descriptors */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_ADDR, TxDescBaseAddr );
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_ADDR, RxDescBaseAddr );
}  /* HardwareSetDescriptorBase */


void HardwareResetPackets (
    PTDescInfo pInfo )
{
    pInfo->pCurrent = pInfo->pRing;
    pInfo->cnAvail = pInfo->cnAlloc;
    pInfo->iLast = pInfo->iNext = 0;
}  /* HardwareResetPackets */
#endif


/* -------------------------------------------------------------------------- */

/*
    Link processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeLink

    Description:
        This routine acknowledges the link change interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareAcknowledgeLink_PCI
#else
void HardwareAcknowledgeLink_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_PHY );

#else
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_PHY ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
        INT_PHY );
#endif
#endif
}  /* HardwareAcknowledgeLink */
#endif


/*
    HardwareCheckLink

    Description:
        This routine checks the link status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareCheckLink_PCI
#else
void HardwareCheckLink_ISA
#endif
(
    PHARDWARE pHardware )
{
    SwitchGetLinkStatus( pHardware );

}  /* HardwareCheckLink */

/* -------------------------------------------------------------------------- */

/*
    Receive processing primary routines
*/

#ifndef INLINE

/*
    HardwareReleaseReceive

    Description:
        This routine release the receive packet memory.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareReleaseReceive_PCI
#else
void HardwareReleaseReceive_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_ISA_BUS
#ifdef SH_16BIT_WRITE
    HardwareWriteRegWord( pHardware, REG_RXQ_CMD_BANK, REG_RXQ_CMD_OFFSET,
        RXQ_CMD_FREE_PACKET );

#else
    HardwareWriteRegByte( pHardware, REG_RXQ_CMD_BANK, REG_RXQ_CMD_OFFSET,
        RXQ_CMD_FREE_PACKET );
#endif
#endif /* KS_ISA_BUS */

}  /* HardwareReleaseReceive */



/*
    HardwareAcknowledgeReceive

    Description:
        This routine acknowledges the receive interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareAcknowledgeReceive_PCI
#else
void HardwareAcknowledgeReceive_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_RX );

#else
    /* Acknowledge the Receive interrupt. */
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_RX ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
        INT_RX );
#endif

#endif
}  /* HardwareAcknowledgeReceive */

#endif


/*
    HardwareStartReceive

    Description:
        This routine starts the receive function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareStartReceive_PCI
#else
void HardwareStartReceive_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_CTRL,
        pHardware->m_dwReceiveConfig );

    /* Notify when the receive stops. */
    pHardware->m_ulInterruptMask |= INT_RX_STOPPED;
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_START, DMA_START );
    pHardware->m_bReceiveStop++;

    /* Variable overflows. */
    if ( 0 == pHardware->m_bReceiveStop )
        pHardware->m_bReceiveStop = 2;

#else
    HardwareWriteRegWord( pHardware, REG_RX_CTRL_BANK, REG_RX_CTRL_OFFSET,
        pHardware->m_wReceiveConfig );

    /* Clear the receive stopped interrupt status. */
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_RX_STOPPED ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK,
        REG_INT_STATUS_OFFSET, INT_RX_STOPPED );
#endif

#ifdef DBG
    /* Notify when the receive stops. */
    pHardware->m_wInterruptMask |= INT_RX_STOPPED;
#endif
#endif
}  /* HardwareStartReceive */


/*
    HardwareStopReceive

    Description:
        This routine stops the receive function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareStopReceive_PCI
#else
void HardwareStopReceive_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    pHardware->m_bReceiveStop = 0;
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_CTRL,
                    (pHardware->m_dwReceiveConfig & ~DMA_RX_CTRL_ENABLE ) );

#else

    /* Interrupt will always trigger if not stopped. */
    HardwareTurnOffInterrupt( pHardware, INT_RX_STOPPED );
    HardwareDisableInterruptBit( pHardware, INT_RX_STOPPED );
    HardwareWriteRegWord( pHardware, REG_RX_CTRL_BANK, REG_RX_CTRL_OFFSET,
        0 );
#endif
}  /* HardwareStopReceive */

/* -------------------------------------------------------------------------- */

/*
    Transmit processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeTransmit

    Description:
        This routine acknowledges the trasnmit interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareAcknowledgeTransmit_PCI
#else
void HardwareAcknowledgeTransmit_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, INT_TX );

#else
    /* Acknowledge the interrupt and remove the packet from TX FIFO. */
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_TX ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
        INT_TX );
#endif

#endif
}  /* HardwareAcknowledgeTransmit */
#endif


/*
    HardwareStartTransmit

    Description:
        This routine starts the transmit function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareStartTransmit_PCI
#else
void HardwareStartTransmit_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
        pHardware->m_dwTransmitConfig );

#ifdef DEVELOP
{
    PTDescInfo pDescInfo = &pHardware->m_TxDescInfo;
    ULONG      ulIntStatus;
    int        i;
    int        timeout;
    PTDesc     pCurrent = pDescInfo->pRing;

    /* Find out the current descriptor pointed by hardware. */
    for ( i = 0; i < pDescInfo->cnAlloc - 1; i++ )
    {
        /* This descriptor will be skipped. */
        pCurrent->phw->BufSize.ulData = 0;
        pCurrent->sw.Control.tx.fHWOwned = TRUE;
        pCurrent->phw->Control.ulData =
            CPU_TO_LE32( pCurrent->sw.Control.ulData );
        pCurrent++;
    }

    /* Stop at the last descriptor. */
    pCurrent->sw.Control.tx.fHWOwned = FALSE;
    pCurrent->phw->Control.ulData =
        CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* Let the hardware goes through the descriptors. */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, DMA_START );

    timeout = 10;
    do {
        DelayMillisec( 10 );
        HardwareReadInterrupt( pHardware, &ulIntStatus );
    } while ( !( ulIntStatus & INT_TX_EMPTY )  &&  timeout-- );
    if ( !( ulIntStatus & INT_TX_EMPTY ) ) {
        DBG_PRINT( "Tx not working!  Reset the hardware!"NEWLINE );
    }

    /* Acknowledge the interrupt. */
    HardwareAcknowledgeEmpty( pHardware );

    /* Last descriptor */
    pCurrent->sw.BufSize.tx.wBufSize = 0;
    pCurrent->phw->BufSize.ulData =
        CPU_TO_LE32( pCurrent->sw.BufSize.ulData );
    pCurrent->sw.Control.tx.fHWOwned = TRUE;
    pCurrent->phw->Control.ulData = CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* First descriptor */
    pCurrent = pDescInfo->pRing;
    pCurrent->sw.Control.tx.fHWOwned = FALSE;
    pCurrent->phw->Control.ulData =
        CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* Let the hardware goes to the first descriptor. */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, DMA_START );

    timeout = 10;
    do {
        DelayMillisec( 10 );
        HardwareReadInterrupt( pHardware, &ulIntStatus );
    } while ( !( ulIntStatus & INT_TX_EMPTY )  &&  timeout-- );
    if ( !( ulIntStatus & INT_TX_EMPTY ) ) {
        DBG_PRINT( "Tx not working!  Reset the hardware!"NEWLINE );
    }

    /* Acknowledge the interrupt. */
    HardwareAcknowledgeEmpty( pHardware );

    /* Reset all the descriptors. */
    pCurrent = pDescInfo->pRing;
    for ( i = 0; i < pDescInfo->cnAlloc; i++ )
    {
        pCurrent->sw.Control.tx.fHWOwned = FALSE;
        pCurrent->phw->Control.ulData =
            CPU_TO_LE32( pCurrent->sw.Control.ulData );
        pCurrent++;
    }
    pDescInfo->pCurrent = pDescInfo->pRing;
}
#endif

#else
    HardwareWriteRegWord( pHardware, REG_TX_CTRL_BANK, REG_TX_CTRL_OFFSET,
        pHardware->m_wTransmitConfig );

    /* Clear the transmit stopped interrupt status. */
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( INT_TX_STOPPED ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK,
        REG_INT_STATUS_OFFSET, INT_TX_STOPPED );
#endif
#endif
}  /* HardwareStartTransmit */


/*
    HardwareStopTransmit

    Description:
        This routine stops the transmit function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareStopTransmit_PCI
#else
void HardwareStopTransmit_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
                   (pHardware->m_dwTransmitConfig & ~DMA_TX_CTRL_ENABLE ) );

#else
    HardwareWriteRegWord( pHardware, REG_TX_CTRL_BANK, REG_TX_CTRL_OFFSET,
        0 );
#endif
}  /* HardwareStopTransmit */

/* -------------------------------------------------------------------------- */

/*
    Interrupt processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeInterrupt

    Description:
        This routine acknowledges the specified interrupts.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterrupt
        ULONG  ulInterrupt
            The interrupt masks to be acknowledged.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareAcknowledgeInterrupt_PCI
#else
void HardwareAcknowledgeInterrupt_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    ULONG     ulInterrupt )

#else
    USHORT    wInterrupt )
#endif
{
#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_STATUS, ulInterrupt );

#else
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntStat( pHardware, INT_STATUS( wInterrupt ));
#else
    HardwareWriteRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
        wInterrupt );
#endif

#endif
}  /* HardwareAcknowledgeInterrupt */


/*
    HardwareDisableInterrupt

    Description:
        This routine disables the interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareDisableInterrupt_PCI
#else
void HardwareDisableInterrupt_ISA
#endif
(
    PHARDWARE pHardware )
{

#ifdef KS_PCI_BUS
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, 0 );
    pHardware->m_ulInterruptSet = 0;

#else
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntMask( pHardware, 0 );
#else
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        0 );
#endif

#endif
}  /* HardwareDisableInterrupt */


/*
    HardwareEnableInterrupt

    Description:
        This routine enables the interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareEnableInterrupt_PCI
#else
void HardwareEnableInterrupt_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    pHardware->m_ulInterruptSet = pHardware->m_ulInterruptMask;
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
        pHardware->m_ulInterruptMask );

#else
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntMask( pHardware, pHardware->m_wInterruptMask );
#else
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        pHardware->m_wInterruptMask );
#endif

#endif
}  /* HardwareEnableInterrupt */


/*
    HardwareDisableInterruptBit

    Description:
        This routine disables the individual interrupt bit of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterrupt
        ULONG  ulInterrupt
            The interrupt masks bit to be disabled.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareDisableInterruptBit_PCI
#else
void HardwareDisableInterruptBit_ISA
#endif
(
    PHARDWARE pHardware,
#ifdef KS_PCI_BUS
    ULONG     ulInterrupt

#else
    USHORT    wInterrupt
#endif
)
{
#ifdef KS_PCI_BUS
    ULONG     ulReadInterrupt;

    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
                   &ulReadInterrupt );
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
                   ( ulReadInterrupt & ~ulInterrupt) );

#else
#ifdef SH_32BIT_ACCESS_ONLY
    ULONG dwReadInterrupt;

    HardwareReadRegDWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        &dwReadInterrupt );
    dwReadInterrupt &= 0x0000FFFF;
    dwReadInterrupt &= ~wInterrupt;
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, dwReadInterrupt );
#else
    USHORT    wReadInterrupt;

    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
                          &wReadInterrupt );
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
                          (USHORT) (wReadInterrupt & ~wInterrupt) );
#endif

#endif
}  /* HardwareDisableInterruptBit */


/*
    HardwareEnableInterruptBit

    Description:
        This routine enables the individual interrupt bit of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterrupt
        ULONG  ulInterrupt
            The interrupt masks bit to be enabled.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareEnableInterruptBit_PCI
#else
void HardwareEnableInterruptBit_ISA
#endif
(
    PHARDWARE pHardware,
#ifdef KS_PCI_BUS
    ULONG     ulInterrupt

#else
    USHORT    wInterrupt
#endif
)
{
#ifdef KS_PCI_BUS
    ULONG     ulReadInterrupt;

    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
                   &ulReadInterrupt );
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
                   ( ulReadInterrupt | ulInterrupt) );
#else
#ifdef SH_32BIT_ACCESS_ONLY
    ULONG dwReadInterrupt;

    HardwareReadRegDWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        &dwReadInterrupt );
    dwReadInterrupt &= 0x0000FFFF;
    dwReadInterrupt |= wInterrupt;
    HW_WRITE_DWORD( pHardware, REG_INT_MASK_OFFSET, dwReadInterrupt );
#else
    USHORT    wReadInterrupt;

    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
                          &wReadInterrupt );
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
                          (USHORT) (wReadInterrupt | wInterrupt) );
#endif

#endif
}  /* HardwareEnableInterruptBit */


/*
    HardwareReadInterrupt

    Description:
        This routine reads the current interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        PUSHORT pwStatus
        PULONG  pulStatus
            Buffer to store the interrupt mask.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareReadInterrupt_PCI
#else
void HardwareReadInterrupt_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    PULONG    pulStatus )

#else
    PUSHORT   pwStatus )
#endif
{
#ifdef KS_PCI_BUS
    HW_READ_DWORD( pHardware, REG_INTERRUPTS_STATUS, pulStatus );

#else
    HardwareReadRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
        pwStatus );

#endif
}  /* HardwareReadInterrupt */


/*
    HardwareSetInterrupt

    Description:
        This routine enables the specified interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterrupt
        ULONG  ulInterrupt
            The interrupt mask to enable.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareSetInterrupt_PCI
#else
void HardwareSetInterrupt_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    ULONG     ulInterrupt )

#else
    USHORT    wInterrupt )
#endif
{
#ifdef KS_PCI_BUS
    pHardware->m_ulInterruptSet = ulInterrupt;
    HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE, ulInterrupt );

#else
#ifdef SH_32BIT_ACCESS_ONLY
    HardwareWriteIntMask( pHardware, wInterrupt );
#else
    HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        wInterrupt );
#endif

#endif
}  /* HardwareSetInterrupt */
#endif


/*
    HardwareBlockInterrupt

    Description:
        This function blocks all interrupts of the hardware and returns the
        current interrupt enable mask so that interrupts can be restored later.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (USHORT):
    Return (ULONG):
        The current interrupt enable mask.
*/

#ifdef KS_PCI_BUS
ULONG  HardwareBlockInterrupt_PCI
#else
USHORT HardwareBlockInterrupt_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    ULONG Interrupt;

    HW_READ_DWORD( pHardware, REG_INTERRUPTS_ENABLE, &Interrupt );

#else
    USHORT Interrupt;

    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        &Interrupt );

#endif
    HardwareDisableInterrupt( pHardware );
    return( Interrupt );
}  /* HardwareBlockInterrupt */

/* -------------------------------------------------------------------------- */

/*
    Other interrupt primary routines
*/

#ifndef INLINE
/*
    HardwareTurnOffInterrupt

    Description:
        This routine turns off the specified interrupts in the interrupt mask
        so that those interrupts will not be enabled.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterruptBit
        ULONG  ulInterruptBit
            The interrupt bits to be off.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareTurnOffInterrupt_PCI
#else
void HardwareTurnOffInterrupt_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    ULONG     ulInterruptBit )

#else
    USHORT    wInterruptBit )
#endif
{
#ifdef KS_PCI_BUS
    pHardware->m_ulInterruptMask &= ~ulInterruptBit;

#else
    pHardware->m_wInterruptMask &= ~wInterruptBit;
#endif
}  /* HardwareTurnOffInterrupt */
#endif


/*
    HardwareTurnOnInterrupt

    Description:
        This routine turns on the specified interrupts in the interrupt mask so
        that those interrupts will be enabled.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wInterruptBit
        ULONG  ulInterruptBit
            The interrupt bits to be on.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareTurnOnInterrupt_PCI
#else
void HardwareTurnOnInterrupt_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    ULONG     ulInterruptBit,
    PULONG    pulInterruptMask )

#else
    USHORT    wInterruptBit,
    PUSHORT   pwInterruptMask )
#endif
{
#ifdef KS_PCI_BUS
    pHardware->m_ulInterruptMask |= ulInterruptBit;

    /* An interrupt mask is previously retrieved to be set later. */
    if ( pulInterruptMask )
    {
        *pulInterruptMask |= ulInterruptBit;
    }
    else
    {
        pHardware->m_ulInterruptSet = pHardware->m_ulInterruptMask;
        HW_WRITE_DWORD( pHardware, REG_INTERRUPTS_ENABLE,
            pHardware->m_ulInterruptMask );
    }

#else
    pHardware->m_wInterruptMask |= wInterruptBit;

    /* An interrupt mask is previously retrieved to be set later. */
    if ( pwInterruptMask )
    {
        *pwInterruptMask |= wInterruptBit;
    }
    else
    {
#ifdef SH_32BIT_ACCESS_ONLY
        HardwareWriteIntMask( pHardware, pHardware->m_wInterruptMask );
#else
        HardwareWriteRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
            pHardware->m_wInterruptMask );
#endif
    }
#endif
}  /* HardwareTurnOnInterrupt */

/* -------------------------------------------------------------------------- */

/*
    Register saving routines
*/

#ifdef KS_ISA_BUS
#ifndef INLINE
/*
    HardwareRestoreBank

    Description:
        This routine restores the current register bank saved during interrupt
        processing.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareRestoreBank (
    PHARDWARE pHardware )
{
    HardwareSelectBank( pHardware, pHardware->m_bSavedBank );
}  /* HardwareRestoreBank */


/*
    HardwareSaveBank

    Description:
        This routine saves the current register bank during interrupt
        processing to be restored later.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSaveBank (
    PHARDWARE pHardware )
{
    HW_READ_BYTE( pHardware, REG_BANK_SEL_OFFSET, &pHardware->m_bBank );
    pHardware->m_bSavedBank = pHardware->m_bBank;
}  /* HardwareSaveBank */


/*
    HardwareRestoreRegs

    Description:
        This routine restores the registers saved during interrupt processing.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareRestoreRegs (
    PHARDWARE pHardware )
{
    HardwareRestoreBank( pHardware );
}  /* HardwareRestoreRegs */


/*
    HardwareSaveRegs

    Description:
        This routine saves the registers during interrupt processing to be
        restored later.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSaveRegs (
    PHARDWARE pHardware )
{
    HardwareSaveBank( pHardware );

}  /* HardwareSaveRegs */
#endif
#endif

/* -------------------------------------------------------------------------- */

/*
    HardwareDisable

    Description:
        This routine disables the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareDisable_PCI
#else
void HardwareDisable_ISA
#endif
(
    PHARDWARE pHardware )
{
    HardwareStopReceive( pHardware );
    HardwareStopTransmit( pHardware );
    pHardware->m_bEnabled = FALSE;
}  /* HardwareDisable */


/*
    HardwareEnable

    Description:
        This routine enables the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareEnable_PCI
#else
void HardwareEnable_ISA
#endif
(
    PHARDWARE pHardware )
{
    HardwareStartTransmit( pHardware );
    HardwareStartReceive( pHardware );
    pHardware->m_bEnabled = TRUE;

#if defined( KS_ISA_BUS )  &&  defined( DBG_ )
    DisplayRegisters( pHardware );
#endif
}  /* HardwareEnable */

/* -------------------------------------------------------------------------- */

/*
    Receive processing routines
*/

#ifdef KS_PCI_BUS
#ifndef INLINE
/*
    HardwareFreeReceivedPacket

    Description:
        This routine frees the received packet and puts it back in the queue
        for use.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int* piNext
            Pointer to descriptor index of received packet.

    Return (None):
*/

void HardwareFreeReceivedPacket (
    PTDescInfo pInfo,
    int*       piNext )
{
    ASSERT( pInfo->cnAvail > 0 );
    ( *piNext )++;
    *piNext &= pInfo->iMax;
    pInfo->cnAvail--;

#ifdef DEBUG_RX_DESC
    DBG_PRINT( "free rx: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeReceivedPacket */


/*
    HardwareGetReceivedPacket

    Description:
        This function retrieves the received packet.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int iNext
            Descriptor index of next packet.

        ULONG* pulData
            Pointer to descriptor status.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetReceivedPacket (
    PTDescInfo pInfo,
    int        iNext,
    ULONG*     pulData )
{
    PTDesc pDesc = &pInfo->pRing[ iNext ];

    *pulData = LE32_TO_CPU( pDesc->phw->Control.ulData );

#ifdef DEBUG_RX_DESC_CHECK
    DBG_PRINT( "rx? %d: %x=%08X"NEWLINE, pInfo->cnAvail,
        iNext, ( int ) *pulData );
#endif
    if ( !( *pulData & DESC_HW_OWNED ) )
    {
        pInfo->pCurrent = pDesc;
        pInfo->cnAvail++;

#ifdef DEBUG_RX_DESC
        DBG_PRINT( "got rx: %d: %x"NEWLINE, pInfo->cnAvail,
            ( int ) pInfo->pCurrent );
#endif
        return( pInfo->pCurrent );
    }
    return NULL;
}  /* HardwareGetReceivedPacket */


/*
    HardwareGetRxPacket

    Description:
        This function retrieves the packet for receive setup.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetRxPacket (
    PTDescInfo pInfo )
{
    pInfo->pCurrent = &pInfo->pRing[ pInfo->iLast ];
    pInfo->iLast++;
    pInfo->iLast &= pInfo->iMax;
    pInfo->cnAvail--;

    pInfo->pCurrent->sw.BufSize.ulData &= ~DESC_RX_MASK;

    return( pInfo->pCurrent );
}  /* HardwareGetRxPacket */
#endif
#endif


#ifdef KS_ISA_BUS
/*
    HardwareReceiveEarly

    Description:
        This routine handles the receive early interrupt processing.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareReceiveEarly (
    PHARDWARE pHardware )
{
    /* Setup read address pointer. */
    HardwareWriteRegWord( pHardware, REG_RX_ADDR_PTR_BANK,
        REG_RX_ADDR_PTR_OFFSET, ADDR_PTR_AUTO_INC | 4 );

    HW_READ_BUFFER( pHardware, REG_DATA_OFFSET, pHardware->m_bLookahead,
        pHardware->m_wReceiveThreshold );


/* Hardware continues to generate interrupts as more data are received in the
   packet already acknowledged.
*/
#if 1
    HardwareTurnOffEarlyInterrupt( pHardware );
#endif
}  /* HardwareReceiveEarly */



/*
    HardwareReceiveMoreDataAvailable

    Description:
        This function returns the remain packet length of the received packet
        in the RXQ. If remain packet length is not zero, there are more
        packets waiting in the RXQ.


    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (USHORT):
        The length of the total received packet in the RXQ.
*/

USHORT HardwareReceiveMoreDataAvailable (
      PHARDWARE pHardware )
{
    USHORT wMoreDataLength=0;



    /* Read from the RXMIR. */
    HardwareReadRegWord( pHardware, REG_RX_MEM_INFO_BANK,
        REG_RX_MEM_INFO_OFFSET, &wMoreDataLength );

    wMoreDataLength &= MEM_AVAILABLE_MASK;

    return ( wMoreDataLength );

}  /* HardwareReceiveMoreDataAvailable */


/*
    HardwareReceiveStatus

    Description:
        This function checks the Receive Status.
        information.


    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        PUSHORT  pwStatus
            Pointer to a USHORT with Receive Status.

        PUSHORT  pwLength
            Pointer to a USHORT with Receive Packet Length.

    Return (BOOLEAN):
        TRUE if received packet is valid and good packet; otherwise, FALSE.
*/

BOOLEAN HardwareReceiveStatus (
      PHARDWARE pHardware,
      PUSHORT   pwStatus,
      PUSHORT   pwLength )
{
    USHORT  wStatus;
    BOOLEAN fStatus=TRUE;
    int     port = 0;


    /* Read Receive Status */
    HardwareReadRegWord( pHardware, REG_RX_STATUS_BANK, REG_RX_STATUS_OFFSET,
                         pwStatus );
    wStatus = *pwStatus;

    /* Read Receive Status */
    HardwareReadRegWord( pHardware, REG_RX_BYTE_CNT_BANK, REG_RX_BYTE_CNT_OFFSET,
                         pwLength );
    *pwLength &= RX_BYTE_CNT_MASK;


    #ifdef DEF_KS8842
    /* only valid if Switch enagine enabled (under ks8842) */
    pHardware->m_bPortRX = ( wStatus & RX_SRC_PORTS ) >> RX_SRC_PORTS_SHIFT;
    #else
    pHardware->m_bPortRX = 1;
    #endif /* #ifdef DEF_KS8842 */


    if ( pHardware->m_bPortRX > 0)
        port = pHardware->m_bPortRX - 1;


    /*
     *  Receive Valid Frame.
     */
    if ( ( wStatus & RX_VALID ) )
    {
        /*
         *  Receive with error.
         */
        if ( ( wStatus & RX_ERRORS ) )
        {
            /* Update receive error statistics. */
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_RCV_ERROR ]+=1;

#ifndef NO_STATS
            if ( (wStatus & RX_PHY_ERROR) == RX_PHY_ERROR )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_MII ]+=1;

            if ( (wStatus & RX_BAD_CRC) == RX_BAD_CRC )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_CRC ]+=1;

            if ( (wStatus & RX_TOO_SHORT) == RX_TOO_SHORT )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_RUNT ]+=1;

            if ( (wStatus & RX_TOO_LONG) == RX_TOO_LONG )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_TOOLONG ]+=1;
#endif

            fStatus = FALSE;
        }

        /*
         *  Receive without error.
         */
        else
        {
#ifndef NO_STATS
            if  ( ( (wStatus & RX_MULTICAST) == RX_MULTICAST ) ||
                  ( (wStatus & RX_BROADCAST) == RX_BROADCAST ) )
            {
                if ( (wStatus & RX_BROADCAST) == RX_BROADCAST )
                    pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_BROADCAST_FRAMES_RCV ]+=1;
                else
                    pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_MULTICAST_FRAMES_RCV ]+=1;



            }
            if ( (wStatus & RX_UNICAST) == RX_UNICAST )
                pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_UNICAST_FRAMES_RCV ]+=1;
#endif
        }
    }
    /*
     *  Receive Invalid Frame.
     */
    else
    {
        pHardware->m_cnCounter
            [ port ]
            [ OID_COUNTER_RCV_ERROR ]+=1;
        pHardware->m_cnCounter
            [ port ]
            [ OID_COUNTER_RCV_INVALID_FRAME ]+=1;
        fStatus = FALSE;
    }

    return ( fStatus );

}  /* HardwareReceiveStatus */


/*
    HardwareReceiveLength

    Description:
        This function returns the length of the received packet.

        No matter switch is Direct mode or Lookup mode, when
        m_bPortRX is 1 : Receive packet from Port1,
        m_bPortRX is 2 : Receive packet from Port2.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (int):
        The length of the received packet.
*/

int HardwareReceiveLength (
    PHARDWARE pHardware )
{
    ULONG dwStatus;
    int   wLength;
    int   wStatus;
    int   port = 0;


    /* Setup read address pointer. */

    HardwareReadRegDWord( pHardware, REG_DATA_BANK,
                          REG_DATA_OFFSET, &dwStatus );

    wStatus = dwStatus & 0xFFFF;
    wLength = dwStatus >> 16;

    #ifdef DEF_KS8842
    /* only valid if Switch enagine enabled (under ks8842) */
    pHardware->m_bPortRX = ( wStatus & RX_SRC_PORTS ) >> RX_SRC_PORTS_SHIFT;
    #else
    pHardware->m_bPortRX = 1;
    #endif /* #ifdef DEF_KS8842 */


    if ( pHardware->m_bPortRX > 0)
        port = pHardware->m_bPortRX - 1;


    /*
     *  Receive without error.
     */

    if ( !( wStatus & RX_ERRORS ) )
    {
#ifdef DEBUG_RX
        DBG_PRINT( "  RX: %04X %u ", wStatus, wLength );
#endif

        /* exclude 4-BYTE CRC if Receive Strip CRC is disable */
        if ( !(pHardware->m_wReceiveConfig & RX_CTRL_STRIP_CRC) )
             wLength -= 4;

        pHardware->m_nPacketLen = wLength;

#ifdef DEBUG_RX
        DBG_PRINT( "%u"NEWLINE, wLength );
#endif
        return( wLength );
    }

    /*
     *  Receive with error.
     */
    else
    {
        pHardware->m_nPacketLen = 0;

#ifdef DEBUG_RX
        DBG_PRINT( "  RX: %04X"NEWLINE, wStatus );
#endif

#ifdef DEBUG_COUNTER
        pHardware->m_nBad[ COUNT_BAD_RCV_FRAME ]+=1;
#endif

#if defined( _WIN32 ) || defined( DEF_LINUX )
#ifdef SH_16BIT_WRITE
        HW_WRITE_WORD( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );

#else
        HW_WRITE_BYTE( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );
#endif
#endif
        return( 0 );
    }
}  /* HardwareReceiveLength */


/*
    HardwareReceiveBuffer

    Description:
        This routine reads the received packet into system memory.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        void* pBuffer
            Buffer to store the packet.

        int length
            The length of the packet.

    Return (None):
*/

void HardwareReceiveBuffer (
    PHARDWARE pHardware,
    void*     pBuffer,
    int       length )
{
#ifdef SH_32BIT_ALIGNED
    HW_READ_BUFFER( pHardware, REG_DATA_OFFSET, pHardware->m_bLookahead,
        length );
    memcpy( pBuffer, pHardware->m_bLookahead, length );

#else
    HW_READ_BUFFER( pHardware, REG_DATA_OFFSET, pBuffer, length );
#endif

#if defined( _WIN32 ) || defined( DEF_LINUX )
#ifdef SH_16BIT_WRITE
    HW_WRITE_WORD( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );

#else
    HW_WRITE_BYTE( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );
#endif
#endif

#ifdef EARLY_RECEIVE
#ifdef DEBUG_COUNTER
    if ( pHardware->m_bReceiveDiscard )
        pHardware->m_nGood[ COUNT_GOOD_RCV_NOT_DISCARD ]+=1;
#endif
    pHardware->m_bReceiveDiscard = FALSE;
    HardwareTurnOnEarlyInterrupt( pHardware, &pHardware->m_wInterruptMask );
#endif

#ifdef DEBUG_RX_DATA
    do {
        int    nLength;
        UCHAR* bData = ( PUCHAR ) pBuffer;

        if ( 0xFF == bData[ 0 ] )
            break;
        for ( nLength = 0; nLength < 0x40; nLength++ )
        {
            DBG_PRINT( "%02X ", bData[ nLength ]);
            if ( ( nLength % 16 ) == 15 )
            {
                DBG_PRINT( NEWLINE );
            }
        }
        DBG_PRINT( NEWLINE );
    } while ( 0 );
#endif
}  /* HardwareReceiveBuffer */
#endif


/*
    HardwareReceive

    Description:
        This routine handles the receive processing.  The packet is read into
        the m_Lookahead buffer and its length is indicated in m_nPacketLen.

        No matter switch is Direct mode or Lookup mode, when
        m_bPortRX is 1 : Receive packet from Port1,
        m_bPortRX is 2 : Receive packet from Port2.


    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareReceive_PCI
#else
void HardwareReceive_ISA
#endif
(
    PHARDWARE pHardware )
{
    /* PCI_BUS */

#ifdef KS_PCI_BUS
    TDescStat status;
    PTDesc    pDesc = pHardware->m_RxDescInfo.pCurrent;
    int       port = 0;

    /* Assume error. */
    pHardware->m_nPacketLen = 0;

    status.ulData = pDesc->sw.Control.ulData;

    /* Status valid only when last descriptor bit is set. */
    if ( status.rx.fLastDesc )
    {

#ifdef CHECK_RCV_ERRORS
        /*
         *  Receive without error.  With receive errors disabled, packets with
         *  receive errors will be dropped, so no need to check the error bit.
         */
        if ( !status.rx.fError )
#endif
        {
            /* Get received port number */
            pHardware->m_bPortRX = ( UCHAR ) status.rx.ulSourePort;

            if ( pHardware->m_bPortRX > 0 )
                port = pHardware->m_bPortRX - 1;

#ifdef DEBUG_RX
            DBG_PRINT( "m_bPortRX: %d"NEWLINE, pHardware->m_bPortRX );
#endif

#ifndef NO_STATS
            if ( status.rx.fMulticast )
            {
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_MULTICAST_FRAMES_RCV ]+=1;
#ifdef DEBUG_RX
                DBG_PRINT( "M " );
#endif
            }
#endif

            /* received length includes 4-byte CRC */
            pHardware->m_nPacketLen = status.rx.wFrameLen - 4;
        }

#ifdef CHECK_RCV_ERRORS
        /*
         *  Receive with error.
         */
        else {

            /* Update receive error statistics. */

            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_RCV_ERROR ]+=1;

#ifndef NO_STATS
            if ( status.rx.fErrCRC )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_CRC ]+=1;

            if ( status.rx.fErrRunt )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_RUNT ]+=1;

            if ( status.rx.fErrTooLong )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_TOOLONG ]+=1;

            if ( status.rx.fErrPHY )
                pHardware->m_cnCounter
                    [ port ]
                    [ OID_COUNTER_RCV_ERROR_MII ]+=1;
#endif

#ifdef DEBUG_RX
            DBG_PRINT( "  RX: %08lX"NEWLINE, status.ulData );
#endif

#ifdef DEBUG_COUNTER
            pHardware->m_nBad[ COUNT_BAD_RCV_FRAME ]+=1;
#endif
        }

/* Hardware checksum errors are not associated with receive errors. */
#if RXCHECKSUM_DEFAULT

/* Hardware cannot handle UDP packet in IP fragments. */
#if 0
        if ( status.rx.fCsumErrUDP )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_RCV_ERROR_UDP ]+=1;
#endif

        if ( status.rx.fCsumErrTCP )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_RCV_ERROR_TCP ]+=1;

        if ( status.rx.fCsumErrIP )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_RCV_ERROR_IP ]+=1;
#endif
#endif
    }

    /* ISA_BUS */
#else

    ULONG dwStatus;

    int   wLength;
    int   wStatus;
    int   port = 0;

    HardwareSelectBank( pHardware, REG_DATA_BANK );

    HW_READ_DWORD( pHardware, REG_DATA_OFFSET, &dwStatus );
    wStatus = dwStatus & 0xFFFF;
    wLength = dwStatus >> 16;

    #ifdef DEF_KS8842
    /* only valid if Switch enagine enabled (under ks8842) */
    pHardware->m_bPortRX = ( wStatus & RX_SRC_PORTS ) >> RX_SRC_PORTS_SHIFT;
    #else
    pHardware->m_bPortRX = 1;
    #endif /* #ifdef DEF_KS8842 */


    if ( pHardware->m_bPortRX > 0)
        port = pHardware->m_bPortRX - 1;

    /*
     *  Receive without error.
     */
    if ( !( wStatus & RX_ERRORS ) )
    {

#ifndef NO_STATS
        if ( ( wStatus & RX_BROADCAST ) == RX_BROADCAST )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_BROADCAST_FRAMES_RCV ]+=1;

        if ( ( wStatus & RX_MULTICAST ) == RX_MULTICAST )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_MULTICAST_FRAMES_RCV ]+=1;

        if ( ( wStatus & RX_UNICAST ) == RX_UNICAST )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_UNICAST_FRAMES_RCV ]+=1;
#endif

        /* exclude 4-BYTE CRC if Receive Strip CRC is disable */
        if ( !(pHardware->m_wReceiveConfig & RX_CTRL_STRIP_CRC) )
            wLength -= 4;

        HW_READ_BUFFER( pHardware, REG_DATA_OFFSET,
            pHardware->m_bLookahead, wLength );

#if defined( _WIN32 ) || defined( DEF_LINUX )
#ifdef SH_16BIT_WRITE
        HW_WRITE_WORD( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );

#else
        HW_WRITE_BYTE( pHardware, REG_RXQ_CMD_OFFSET, RXQ_CMD_FREE_PACKET );
#endif
#endif

        pHardware->m_nPacketLen = wLength;

#ifdef DEBUG_RX
        DBG_PRINT( "%u"NEWLINE, wLength );
        if ( pHardware->m_bReceiveDiscard )
        {
            for ( wLength = 0; wLength < 16; wLength++ )
                DBG_PRINT( "%02X ", pHardware->m_bLookahead[ wLength ]);
            DBG_PRINT( NEWLINE );
        }
#endif /* #ifdef DEBUG_RX */

#ifdef DEBUG_RX_DATA
#ifdef UNDER_CE
if ( pHardware->m_bLookahead[ 0 ] != 0xFF )
DBG_PRINT( "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"NEWLINE,
    pHardware->m_bLookahead[ 0 ], pHardware->m_bLookahead[ 1 ],
    pHardware->m_bLookahead[ 2 ], pHardware->m_bLookahead[ 3 ],
    pHardware->m_bLookahead[ 4 ], pHardware->m_bLookahead[ 5 ],
    pHardware->m_bLookahead[ 6 ], pHardware->m_bLookahead[ 7 ],
    pHardware->m_bLookahead[ 8 ], pHardware->m_bLookahead[ 9 ],
    pHardware->m_bLookahead[ 10 ], pHardware->m_bLookahead[ 11 ],
    pHardware->m_bLookahead[ 12 ], pHardware->m_bLookahead[ 13 ],
    pHardware->m_bLookahead[ 14 ], pHardware->m_bLookahead[ 15 ]);

#else
        for ( wLength = 0; wLength < pHardware->m_nPacketLen; wLength++ )
        {
            DBG_PRINT( "%02X ",
                pHardware->m_bLookahead[ wLength ]);
            if ( ( wLength % 16 ) == 15 )
            {
                DBG_PRINT( NEWLINE );
            }
        }
        DBG_PRINT( NEWLINE );
#endif

#endif /* #ifdef DEBUG_RX_DATA */
    }
    /*
     *  Receive with error.
     */
    else
    {

        pHardware->m_nPacketLen = 0;

#ifdef DEBUG_RX
        DBG_PRINT( "  RX: %04X"NEWLINE, wStatus );
#endif

#ifdef DEBUG_COUNTER
        pHardware->m_nBad[ COUNT_BAD_RCV_FRAME ]+=1;
#endif
    }

#ifdef EARLY_RECEIVE
#ifdef DEBUG_COUNTER
    if ( pHardware->m_bReceiveDiscard )
        pHardware->m_nGood[ COUNT_GOOD_RCV_NOT_DISCARD ]+=1;
#endif
    pHardware->m_bReceiveDiscard = FALSE;
    HardwareTurnOnEarlyInterrupt( pHardware, &pHardware->m_wInterruptMask );
#endif /* #ifdef EARLY_RECEIVE */

#endif /* #ifdef KS_PCI_BUS */

}  /* HardwareReceive */

/* -------------------------------------------------------------------------- */

/*
    Transmit processing routines
*/


#ifdef KS_PCI_BUS
#define MIN_TX_BUFFER_SIZE    128
#define TX_PACKET_ARRAY_SIZE  8
#define RX_PACKET_ARRAY_SIZE  64
#endif


/*
    HardwareAllocPacket

    Description:
        This function allocates a packet for transmission.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int length
            The length of the packet.

        int physical
            Number of descriptors required in PCI version.

    Return (int):
        0 if not successful; 1 for buffer copy; otherwise, can use descriptors.
*/

#ifdef KS_PCI_BUS
int HardwareAllocPacket_PCI
#else
int HardwareAllocPacket_ISA
#endif
(
    PHARDWARE pHardware,

#ifdef KS_PCI_BUS
    int       length,
    int       physical )

#else
    int       length )
#endif
{
    int port = 0;


    /* PCI_BUS */

#ifdef KS_PCI_BUS
    /* Always leave one descriptor free. */
    if ( pHardware->m_TxDescInfo.cnAvail <= 1 )
    {
        /* Update transmit statistics. */
        pHardware->m_cnCounter[ port ][ OID_COUNTER_XMIT_ALLOC_FAIL ]+=1;
        return( 0 );
    }

#ifdef DEF_LINUX
    if ( physical > pHardware->m_TxDescInfo.cnAvail )
    {
        return( 0 );
    }
#endif

    /* Allocate a descriptor for transmission and mark it current. */
    GetTxPacket( &pHardware->m_TxDescInfo, pHardware->m_TxDescInfo.pCurrent );
    pHardware->m_TxDescInfo.pCurrent->sw.BufSize.tx.fFirstSeg = TRUE;

    if ( length < MIN_TX_BUFFER_SIZE  ||
            physical > TX_PACKET_ARRAY_SIZE  ||
            physical > pHardware->m_TxDescInfo.cnAvail )
    {
        return( 1 );
    }

    return( physical + 1 );

    /* ISA_BUS */
#else

    USHORT wStatus;

    HardwareReadRegWord( pHardware, REG_TX_MEM_INFO_BANK,
        REG_TX_MEM_INFO_OFFSET, &wStatus );
    if ( wStatus < length + 4 )
    {
        /* Update transmit statistics. */
        pHardware->m_cnCounter[ port ][ OID_COUNTER_XMIT_ALLOC_FAIL ]+=1;

#ifdef DEBUG_COUNTER
        pHardware->m_nBad[ COUNT_BAD_ALLOC ]+=1;
#endif
        return FALSE;
    }

    /* Transmit Frame ID consisted of Transmit Packet Number (bit 4~0) and Port Number (bit 5) */
    pHardware->m_bTransmitPacket++;
    pHardware->m_bTransmitPacket &= TX_FRAME_ID_MAX;

    /* if Tx to port 2, set bit_5 to 1; otherwise, set bit_5 to 0 */
    if ( (pHardware->m_bPortTX == 2) || (pHardware->m_bPortTX == 3) )
        pHardware->m_bTransmitPacket |= ( 1 << TX_FRAME_ID_PORT_SHIFT );

    return TRUE;

#endif /* #ifdef KS_PCI_BUS */
}  /* HardwareAllocPacket */



#ifdef KS_PCI_BUS
#ifndef INLINE
/*
    HardwareFreeTransmittedPacket

    Description:
        This routine frees the transmitted packet and puts it back in the queue
        for use.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int* piLast
            Pointer to descriptor index of transmitted packet.

    Return (None):
*/

void HardwareFreeTransmittedPacket (
    PTDescInfo pInfo,
    int*       piLast )
{
    ASSERT( pInfo->cnAvail < pInfo->cnAlloc );
    ( *piLast )++;
    *piLast &= pInfo->iMax;
    pInfo->cnAvail++;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "free tx: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeTransmittedPacket */


/*
    HardwareFreeTxPacket

    Description:
        This routine frees the allocated packet for transmission and puts it
        back in queue for use.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (None):
*/

void HardwareFreeTxPacket (
    PTDescInfo pInfo )
{
    pInfo->iNext--;
    pInfo->iNext &= pInfo->iMax;
    pInfo->cnAvail++;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "putback: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeTxPacket */


/*
    HardwareGetTransmittedPacket

    Description:
        This function retrieves the transmitted packet.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int iLast
            Descriptor index of transmitted packet.

        ULONG* pulData
            Pointer to descriptor status.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetTransmittedPacket (
    PTDescInfo pInfo,
    int        iLast,
    ULONG*     pulData )
{
    PTDesc pDesc = &pInfo->pRing[ iLast ];

    *pulData = LE32_TO_CPU( pDesc->phw->Control.ulData );
    if ( !( *pulData & DESC_HW_OWNED ) )
    {

#ifdef DEBUG_TX_DESC
        DBG_PRINT( "got tx: %x"NEWLINE, iLast );
#endif
    }
    return( pDesc );
}  /* HardwareGetTransmittedPacket */


/*
    HardwareGetTxPacket

    Description:
        This function retrieves the packet for transmission.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetTxPacket (
    PTDescInfo pInfo )
{
    ASSERT( pInfo->cnAvail > 0 );
    pInfo->pCurrent = &pInfo->pRing[ pInfo->iNext ];
    pInfo->iNext++;
    pInfo->iNext &= pInfo->iMax;
    pInfo->cnAvail--;

    pInfo->pCurrent->sw.BufSize.ulData &= ~DESC_TX_MASK;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "get tx: %d: %x"NEWLINE, pInfo->cnAvail,
        ( int ) pInfo->pCurrent );
#endif

    return( pInfo->pCurrent );
}  /* HardwareGetTxPacket */
#endif
#endif


/*
    HardwareSendPacket

    Description:
        This function transmits the packet marked by m_bTransmitPacket.
        This function marks the packet for transmission in PCI version.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareSendPacket_PCI
#else
BOOLEAN HardwareSendPacket_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    PTDesc pCurrent = pHardware->m_TxDescInfo.pCurrent;

    pCurrent->sw.BufSize.tx.fLastSeg = TRUE;

#if 1
    pCurrent->sw.BufSize.tx.fInterrupt = TRUE;
#endif

    pCurrent->sw.BufSize.tx.ulDestPort = pHardware->m_bPortTX;

    ReleasePacket( pCurrent );

    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, 0 );

#else
#ifdef SH_16BIT_WRITE
    HardwareWriteRegWord( pHardware, REG_TXQ_CMD_BANK, REG_TXQ_CMD_OFFSET,
        TXQ_CMD_ENQUEUE_PACKET );

#else
    HardwareWriteRegByte( pHardware, REG_TXQ_CMD_BANK, REG_TXQ_CMD_OFFSET,
        TXQ_CMD_ENQUEUE_PACKET );
#endif
#endif
    return( TRUE );
}  /* HardwareSendPacket */


#ifdef KS_PCI_BUS
#ifndef INLINE
/*
    HardwareSetTransmitBuffer

    Description:
        This routine sets the transmit buffer address.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        ULONG ulBufAddr
            Buffer address.

    Return (None):
*/

void HardwareSetTransmitBuffer (
    PHARDWARE pHardware,
    ULONG     ulBufAddr )
{
    pHardware->m_TxDescInfo.pCurrent->phw->ulBufAddr = CPU_TO_LE32( ulBufAddr );
}  /* HardwareSetTransmitBuffer */
#endif
#endif


#if defined( KS_ISA_BUS )  ||  !defined( INLINE )
/*
    HardwareSetTransmitLength

    Description:
        This routine prepares the transmit length before buffer copying.
        This routine sets the transmit length in PCI version.

        If Switch is already set to Dircet mode, when
        m_bPortTX is 0 : Transmit packet by loopkup mode,
        m_bPortTX is 1 : Transmit packet to Port1,
        m_bPortTX is 2 : Transmit packet to Port2,
        m_bPortTX is 3 : Transmit packet to Port1 and Port2.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int length
            The length of the packet.

    Return (None):
*/

#ifdef KS_PCI_BUS
ULONG  HardwareSetTransmitLength_PCI
#else
ULONG  HardwareSetTransmitLength_ISA
#endif
(
    PHARDWARE pHardware,
    int       length )
{
    ULONG dwLength=0;

    /* PCI_BUS */
#ifdef KS_PCI_BUS
    pHardware->m_TxDescInfo.pCurrent->sw.BufSize.tx.wBufSize = length;

    /* ISA_BUS */
#else

    ULONG dwDestPort=0;


    dwLength = length;
    dwLength <<= 16;

    /* transmit packet using direct mode if m_bPortTX is not zero, otherwise using lookup mode */
    dwDestPort =  pHardware->m_bPortTX;
    dwLength |= (dwDestPort << TX_DEST_PORTS_SHIFT );

    dwLength |= pHardware->m_bTransmitPacket;
    dwLength |= TX_CTRL_INTERRUPT_ON;

    HardwareWriteRegDWord( pHardware, REG_DATA_BANK,
        REG_DATA_OFFSET, dwLength );

#endif /* #ifdef KS_PCI_BUS */

    return (dwLength);
}  /* HardwareSetTransmitLength */

#endif /* #if defined( KS_ISA_BUS )  ||  !defined( INLINE ) */


/*
    HardwareTransmitDone

    Description:
        This function handles the transmit done interrupt processing.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if packet is sent successful; otherwise, FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareTransmitDone_PCI
#else
BOOLEAN HardwareTransmitDone_ISA
#endif
(
    PHARDWARE pHardware )
{
    /* ISA_BUS */

#ifdef KS_ISA_BUS
    USHORT wStatus;
    UCHAR  bPacket;
    int    port = 0;


    HardwareReadRegWord( pHardware, REG_TX_STATUS_BANK, REG_TX_STATUS_OFFSET,
        &wStatus );
    bPacket = ( UCHAR )( wStatus & TX_FRAME_ID_MASK );
    pHardware->m_bSentPacket = bPacket;

#ifdef DEBUG_TX
    if ( bPacket != pHardware->m_bTransmitPacket )
    {
        DBG_PRINT( "sent != transmit: %x, %x"NEWLINE, bPacket,
            pHardware->m_bTransmitPacket );
    }
#endif

    port = bPacket >> TX_FRAME_ID_PORT_SHIFT;

#ifdef DEBUG_TX
    DBG_PRINT( "send status: %x"NEWLINE, wStatus );
#endif

    if ( ( wStatus & TX_STAT_ERRORS ) )
    {
        /* Update transmit error statistics. */

        pHardware->m_cnCounter
            [ port ]
            [ OID_COUNTER_XMIT_ERROR ]+=1;

#ifndef NO_STATS
        if ( ( wStatus & TX_STAT_MAX_COL ) )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_XMIT_MORE_COLLISIONS ]+=1;

        if ( ( wStatus & TX_STAT_LATE_COL ) )
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_XMIT_LATE_COLLISION ]+=1;
#endif

        if ( ( wStatus & TX_STAT_UNDERRUN ) )
        {
            pHardware->m_cnCounter
                [ port ]
                [ OID_COUNTER_XMIT_UNDERRUN ]+=1;
            DBG_PRINT( "tx underrun!"NEWLINE );
        }

        /* Restart the transmit. */
        HardwareStartTransmit( pHardware );

#ifdef DEBUG_COUNTER
        pHardware->m_nBad[ COUNT_BAD_SEND ]+=1;
#endif
        return FALSE;
    }


#endif /* #ifdef KS_ISA_BUS */

    return TRUE;
}  /* HardwareTransmitDone */

/* -------------------------------------------------------------------------- */

/*
    HardwareSetAddress

    Description:
        This routine programs the MAC address of the hardware when the address
        is overrided.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareSetAddress_PCI
#else
void HardwareSetAddress_ISA
#endif
(
    PHARDWARE pHardware )
{
    UCHAR i;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_ADDR_0_BANK );
#endif
    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {
#ifdef SH_16BIT_WRITE
        if ( ( i & 1 ) )
        {
            HW_WRITE_WORD( pHardware, (( REG_ADDR_0_OFFSET + i ) & ~1 ),
                ( pHardware->m_bOverrideAddress[ MAC_ADDR_ORDER( i )] << 8 ) |
                pHardware->m_bOverrideAddress[ MAC_ADDR_ORDER( i - 1 )]);
        }

#else
        HW_WRITE_BYTE( pHardware, ( ULONG )( REG_ADDR_0_OFFSET + i ),
            ( UCHAR ) pHardware->m_bOverrideAddress[ MAC_ADDR_ORDER( i )]);
#endif
    }

#ifdef DBG
    DBG_PRINT( "set addr: " );
    PrintMacAddress( pHardware->m_bOverrideAddress );
    DBG_PRINT( NEWLINE );
#endif

    SwitchSetAddress( pHardware, pHardware->m_bOverrideAddress );
}  /* HardwareSetAddress */


/*
    HardwareReadAddress

    Description:
        This function retrieves the MAC address of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE to indicate successful.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareReadAddress_PCI
#else
BOOLEAN HardwareReadAddress_ISA
#endif
(
    PHARDWARE pHardware )
{
    UCHAR i;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_ADDR_0_BANK );
#endif
    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {
        HW_READ_BYTE( pHardware, ( ULONG )( REG_ADDR_0_OFFSET + i ),
            ( PUCHAR ) &pHardware->m_bPermanentAddress[ MAC_ADDR_ORDER( i )]);
    }

#ifdef DBG
    DBG_PRINT( "get addr: " );
    PrintMacAddress( pHardware->m_bPermanentAddress );
    DBG_PRINT( NEWLINE );
#endif
#if 0
    if ( !pHardware->m_bMacOverrideAddr )
    {
        MOVE_MEM( pHardware->m_bOverrideAddress,
            pHardware->m_bPermanentAddress, MAC_ADDRESS_LENGTH );
#ifdef DEVELOP_
memset( pHardware->m_bOverrideAddress, 0, MAC_ADDRESS_LENGTH );
#endif
        if ( 0 == pHardware->m_bOverrideAddress[ 0 ]  &&
                0 == pHardware->m_bOverrideAddress[ 1 ]  &&
                0 == pHardware->m_bOverrideAddress[ 2 ]  &&
                0 == pHardware->m_bOverrideAddress[ 3 ]  &&
                0 == pHardware->m_bOverrideAddress[ 4 ]  &&
                0 == pHardware->m_bOverrideAddress[ 5 ] )
        {
            MOVE_MEM( pHardware->m_bPermanentAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            MOVE_MEM( pHardware->m_bOverrideAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            HardwareSetAddress( pHardware );
        }
        else
            SwitchSetAddress( pHardware, pHardware->m_bOverrideAddress );
    }
    else
    {
        HardwareSetAddress( pHardware );
    }
#endif
            MOVE_MEM( pHardware->m_bPermanentAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            MOVE_MEM( pHardware->m_bOverrideAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            HardwareSetAddress( pHardware );

    return TRUE;
}  /* HardwareReadAddress */


#ifdef KS_PCI_BUS
/*
 * HardwareAddrFilterClear
 *  This routine clears all the MAC Additional Station Address filter.
 *
 * Argument(s)
 *  pHardware          Pointer to hardware instance.
 *
 * Return(s)
 *  NONE.
 */
void HardwareAddrFilterClear
(
    PHARDWARE  pHardware
)
{
    ULONG   macAddrLow, macAddrHigh;
    int i;


    /* Clear MAC address filter */
    macAddrLow  = REG_ADD_ADDR_0_LO;
    macAddrHigh = REG_ADD_ADDR_0_HI;
    for (i=0; i<16;i++)
    {
        HW_WRITE_DWORD(pHardware, macAddrLow, 0);
        HW_WRITE_DWORD(pHardware, macAddrHigh, 0);
        macAddrLow  += 0x08;
        macAddrHigh += 0x08;
    }
} /* HardwareAddrFilterClear */


/*
 * HardwareAddrFilterAdd
 *  This function adds MAC address to the switch's MAC address filter.
 *
 * Argument(s)
 *  pHardware    Pointer to hardware instance.
 *  pMacAddress  pointer to a byte array to hold MAC address (at least 6 bytes long)
 *
 * Return(s)
 *  NONE.
 */
int  HardwareAddrFilterAdd
(
    PHARDWARE  pHardware,
    UCHAR     *pMacAddress
)
{
    ULONG   uLow, uHigh;
    ULONG   macAddrLow, macAddrHigh;
    UCHAR   macFilterIndex;
    ULONG   uData;

    uLow = ((ULONG)pMacAddress[2] << 24);
    uLow += ((ULONG)pMacAddress[3] << 16);
    uLow += ((ULONG)pMacAddress[4] << 8);
    uLow += pMacAddress[5];
    uHigh = ((ULONG)pMacAddress[0] << 8) + pMacAddress[1];

    /* Search for avaiable entry from the MAC Additional Station Address */

    for ( macFilterIndex=0; macFilterIndex < 16; macFilterIndex++)
    {
        macAddrHigh = ( REG_ADD_ADDR_0_HI +  (macFilterIndex * 0x08) );

        /* check if this entry is avaiable */
        HW_READ_DWORD(pHardware, macAddrHigh, &uData);
        if ( (uData & MAC_ADDR_ENABLE) != MAC_ADDR_ENABLE)
            break;
    }

    /* this entry is not avaiable */
    if  (macFilterIndex >= 16)
        return (FALSE);

    /* this entry is not avaiable */

    macAddrLow  = ( REG_ADD_ADDR_0_LO +  (macFilterIndex * 0x08) );
    macAddrHigh = ( REG_ADD_ADDR_0_HI +  (macFilterIndex * 0x08) );

    HW_WRITE_DWORD(pHardware, macAddrLow,  uLow);
    HW_WRITE_DWORD(pHardware, macAddrHigh, (uHigh | MAC_ADDR_ENABLE));

    return (TRUE);

} /* HardwareAddrFilterAdd */

/*
 * HardwareAddrFilterDel
 *  This function deletes MAC address from the switch's MAC address filter.
 *
 * Argument(s)
 *  pHardware    Pointer to hardware instance.
 *  pMacAddress  pointer to a byte array to hold MAC address (at least 6 bytes long)
 *
 * Return(s)
 *  NONE.
 */
int  HardwareAddrFilterDel
(
    PHARDWARE  pHardware,
    UCHAR     *pMacAddress
)
{
    ULONG   uLow, uHigh;
    ULONG   macAddrLow, macAddrHigh;
    UCHAR   macFilterIndex;
    ULONG   uDataLow, uDataHigh;

    uLow = ((ULONG)pMacAddress[2] << 24);
    uLow += ((ULONG)pMacAddress[3] << 16);
    uLow += ((ULONG)pMacAddress[4] << 8);
    uLow += pMacAddress[5];
    uHigh = ((ULONG)pMacAddress[0] << 8) + pMacAddress[1];

    /* Search for the entry from the MAC Additional Station Address
       that match MAC address to be deleted */

    for ( macFilterIndex=0; macFilterIndex < 16; macFilterIndex++)
    {
        macAddrLow  = ( REG_ADD_ADDR_0_LO +  (macFilterIndex * 0x08) );
        macAddrHigh = ( REG_ADD_ADDR_0_HI +  (macFilterIndex * 0x08) );

        /* check if this entry's MAC match */
        HW_READ_DWORD(pHardware, macAddrLow,  &uDataLow);
        HW_READ_DWORD(pHardware, macAddrHigh, &uDataHigh);

        uDataHigh &= ~MAC_ADDR_ENABLE;

        if ( (uDataLow ==uLow) && (uDataHigh == uHigh) )
            break;
    }

    /* not found */
    if  (macFilterIndex >= 16)
        return (FALSE);

    /* found the entry that match MAC address to be deleted */

    macAddrLow  = ( REG_ADD_ADDR_0_LO +  (macFilterIndex * 0x08) );
    macAddrHigh = ( REG_ADD_ADDR_0_HI +  (macFilterIndex * 0x08) );

    HW_WRITE_DWORD(pHardware, macAddrLow,  0);
    HW_WRITE_DWORD(pHardware, macAddrHigh, 0);

    return (TRUE);

}  /* HardwareAddrFilterDel */

#endif /* #ifdef KS_PCI_BUS */

/*
    HardwareClearMulticast

    Description:
        This routine removes all multicast addresses set in the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareClearMulticast_PCI
#else
void HardwareClearMulticast_ISA
#endif
(
    PHARDWARE pHardware )
{
    UCHAR i;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_MULTICAST_BANK );
#endif
    for ( i = 0; i < HW_MULTICAST_SIZE; i++ )
    {
        pHardware->m_bMulticastBits[ i ] = 0;

#ifdef SH_16BIT_WRITE
        if ( ( i & 1 ) )
        {
            HW_WRITE_WORD( pHardware, ( ULONG )(( REG_MULTICAST_0_OFFSET + i )
                & ~1 ), 0 );
        }
#else
        HW_WRITE_BYTE( pHardware, ( ULONG )( REG_MULTICAST_0_OFFSET + i ), 0 );
#endif
    }
}  /* HardwareClearMulticast */


#if 0
/* The little-endian AUTODIN II ethernet CRC calculation.
   N.B. Do not use for bulk data, use a table-based routine instead.
   This is common code and should be moved to net/core/crc.c */
static unsigned const ethernet_polynomial_le = 0xedb88320U;
static unsigned ether_crc_le (
    int            length,
    unsigned char *data )
{
    unsigned int crc = 0xffffffff;  /* Initial value. */
    while ( --length >= 0 ) {
        unsigned char current_octet = *data++;
        int bit;

        for ( bit = 8; --bit >= 0; current_octet >>= 1 ) {
            if ( ( crc ^ current_octet ) & 1 ) {
                crc >>= 1;
                crc ^= ethernet_polynomial_le;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}  /* ether_crc_le */
#endif


/*
    ether_crc

    Description:
        This function generates a CRC-32 from the data block.
        It is used to calculate values for the hardware multicast filter hash table,
        or wake up frame CRC value.

    Parameters:
        int             length,
            the length of the block data

        unsigned char * data
            Pointer to the block data.

    Return:
        The computed CRC of the data.
*/
static unsigned long const ethernet_polynomial = 0x04c11db7U;
unsigned long ether_crc
(
    int  length,
    unsigned char *data
)
{
    long crc = -1;
    while ( --length >= 0 )
    {
        unsigned char current_octet = *data++;
        int bit;

        for ( bit = 0; bit < 8; bit++, current_octet >>= 1 )
        {
            crc = ( crc << 1 ) ^
                (( crc < 0 ) ^ ( current_octet & 1 ) ?
                ethernet_polynomial : 0 );
        }
    }
    return crc;
}  /* ether_crc */


/*
    HardwareSetGroupAddress

    Description:
        This function programs multicast addresses for the hardware to accept
        those addresses.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareSetGroupAddress_PCI
#else
BOOLEAN HardwareSetGroupAddress_ISA
#endif
(
    PHARDWARE pHardware )
{
    UCHAR i;
    int   index;
    int   position;
    int   value;

    memset( pHardware->m_bMulticastBits, 0,
        sizeof( UCHAR ) * HW_MULTICAST_SIZE );

#ifdef DEVELOP
    DBG_PRINT( "set multicast:"NEWLINE );
    {
        UCHAR spanning_tree[] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x00 };

        position = ( ether_crc( 6, spanning_tree ) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
    }
#endif
    for ( i = 0; i < pHardware->m_bMulticastListSize; i++ )
    {

#ifdef DBG
        PrintMacAddress( pHardware->m_bMulticastList[ i ]);
        DBG_PRINT( NEWLINE );
#endif
        position = ( ether_crc( 6, pHardware->m_bMulticastList[ i ]) >> 26 ) &
            0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
    }

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_MULTICAST_BANK );
#endif
    for ( i = 0; i < HW_MULTICAST_SIZE; i++ )
    {

#ifdef SH_16BIT_WRITE
        if ( ( i & 1 ) )
        {
            HW_WRITE_WORD( pHardware, ( ULONG )(( REG_MULTICAST_0_OFFSET + i )
                & ~1 ),
                ( pHardware->m_bMulticastBits[ i ] << 8 ) |
                pHardware->m_bMulticastBits[ i - 1 ]);
        }
#else
        HW_WRITE_BYTE( pHardware, ( ULONG )( REG_MULTICAST_0_OFFSET + i ),
            pHardware->m_bMulticastBits[ i ]);
#endif
    }
    return TRUE;
}  /* HardwareSetGroupAddress */


/*
    HardwareSetMulticast

    Description:
        This function enables/disables the hardware to accept all multicast
        packets.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bMulticast
            To turn on or off the all multicast feature.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareSetMulticast_PCI
#else
BOOLEAN HardwareSetMulticast_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bMulticast )
{
    pHardware->m_bAllMulticast = bMulticast;

    HardwareStopReceive( pHardware );  /* Stop receiving for reconfiguration */
#ifdef KS_PCI_BUS
    if ( bMulticast )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ALL_MULTICAST;
    else
        pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_ALL_MULTICAST;

#else
    if ( bMulticast )
        pHardware->m_wReceiveConfig |= RX_CTRL_ALL_MULTICAST;
    else
        pHardware->m_wReceiveConfig &= ~RX_CTRL_ALL_MULTICAST;
#endif

    if ( pHardware->m_bEnabled )
        HardwareStartReceive( pHardware );
    return TRUE;
}  /* HardwareSetMulticast */


/*
    HardwareSetPromiscuous

    Description:
        This function enables/disables the hardware to accept all packets.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bMulticast
            To turn on or off the promiscuous feature.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

#ifdef KS_PCI_BUS
BOOLEAN HardwareSetPromiscuous_PCI
#else
BOOLEAN HardwareSetPromiscuous_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPromiscuous )
{
    pHardware->m_bPromiscuous = bPromiscuous;

    HardwareStopReceive( pHardware );  /* Stop receiving for reconfiguration */

#ifdef KS_PCI_BUS
    if ( bPromiscuous )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_PROMISCUOUS;
    else
        pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_PROMISCUOUS;

#else   /* ISA_BUS */
//    if ( bPromiscuous ) //modifyed by zhuzhenhua
        pHardware->m_wReceiveConfig |= RX_CTRL_PROMISCUOUS;
//    else
//        pHardware->m_wReceiveConfig &= ~RX_CTRL_PROMISCUOUS;

#endif

    if ( pHardware->m_bEnabled )
        HardwareStartReceive( pHardware );
    return TRUE;
}  /* HardwareSetPromiscuous */

/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
/*
    HardwareGenerateInterrupt

    Description:
        This routine is used to generate an interrupt so that the actual
        interrupt number can be detected by the operating system.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareGenerateInterrupt (
    PHARDWARE pHardware )
{
    USHORT wInterruptMask;

    HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
        &wInterruptMask );

    /* Turn on receive stopped interrupt. */
    HardwareSetInterrupt( pHardware, INT_RX_STOPPED );

    /* Turn off receive to generate interrupts. */
    HardwareWriteRegWord( pHardware, REG_RX_CTRL_BANK, REG_RX_CTRL_OFFSET,
        0 );

    /* Wait for interrupt to be registered. */
    DelayMillisec( 10 );

    /* Restore original interrupt mask. */
    HardwareSetInterrupt( pHardware, wInterruptMask );
}  /* HardwareGenerateInterrupt */


/*
    HardwareServiceInterrupt

    Description:
        This routine is called after HardwareGenerateInterrupt() to service the
        pending interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareServiceInterrupt (
    PHARDWARE pHardware )
{
    USHORT  wStatus;

    HardwareReadInterrupt( pHardware, &wStatus );

    /* Clear the receive stopped interrupt status. */
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_STOPPED );
}  /* HardwareServiceInterrupt */
#endif

/* -------------------------------------------------------------------------- */

enum {
    CABLE_UNKNOWN,
    CABLE_GOOD,
    CABLE_CROSSED,
    CABLE_REVERSED,
    CABLE_CROSSED_REVERSED,
    CABLE_OPEN,
    CABLE_SHORT
};


#define STATUS_FULL_DUPLEX  0x01
#define STATUS_CROSSOVER    0x02
#define STATUS_REVERSED     0x04

#define LINK_10MBPS_FULL    0x00000001
#define LINK_10MBPS_HALF    0x00000002
#define LINK_100MBPS_FULL   0x00000004
#define LINK_100MBPS_HALF   0x00000008
#define LINK_1GBPS_FULL     0x00000010
#define LINK_1GBPS_HALF     0x00000020
#define LINK_10GBPS_FULL    0x00000040
#define LINK_10GBPS_HALF    0x00000080
#define LINK_SYM_PAUSE      0x00000100
#define LINK_ASYM_PAUSE     0x00000200

#define LINK_AUTO_MDIX      0x00010000
#define LINK_MDIX           0x00020000
#define LINK_AUTO_POLARITY  0x00040000


#define CABLE_LEN_MAXIMUM     15000
#define CABLE_LEN_MULTIPLIER  41


/*
    HardwareGetCableStatus

    Description:
        This routine is used to get the cable status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        void* pBuffer
            Buffer to store the cable status.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareGetCableStatus_PCI
#else
void HardwareGetCableStatus_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer )
{
    PULONG pulData = ( PULONG ) pBuffer;

    USHORT Control;
    USHORT Crossover;
    USHORT Data;
    USHORT LowSpeed;
    ULONG  ulStatus;
    ULONG  ulLength[ 5 ];
    UCHAR  bStatus[ 5 ];
    int    i;
    int    cnTimeOut;
    int    phy;

    phy = bPhy;

#ifdef KS_PCI_BUS
    phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
        PHY_BANK_INTERVAL ));
#endif
    HW_READ_PHY_LINK_STATUS( pHardware, phy, Data );
    bStatus[ 4 ] = CABLE_UNKNOWN;
    if ( ( Data & PHY_LINK_STATUS ) )
    {
        bStatus[ 4 ] = CABLE_GOOD;
        ulLength[ 4 ] = 1;
        bStatus[ 0 ] = CABLE_GOOD;
        ulLength[ 0 ] = 1;
        bStatus[ 1 ] = CABLE_GOOD;
        ulLength[ 1 ] = 1;

#ifdef KS_PCI_BUS
        phy = REG_PHY_SPECIAL_OFFSET + bPhy * PHY_SPECIAL_INTERVAL;

#else
        HardwareSelectBank( pHardware, REG_PHY_SPECIAL_BANK );
#endif
        HW_READ_PHY_POLARITY( pHardware, phy, Data );
        ulStatus = 0;
        if ( ( Data & PHY_STAT_MDIX ) )
            ulStatus |= STATUS_CROSSOVER;
        if ( ( Data & PHY_STAT_REVERSED_POLARITY ) )
            ulStatus |= STATUS_REVERSED;
        if ( ( ulStatus & ( STATUS_CROSSOVER | STATUS_REVERSED )) ==
                ( STATUS_CROSSOVER | STATUS_REVERSED ) )
            bStatus[ 4 ] = CABLE_CROSSED_REVERSED;
        else if ( ( ulStatus & STATUS_CROSSOVER ) == STATUS_CROSSOVER )
            bStatus[ 4 ] = CABLE_CROSSED;
        else if ( ( ulStatus & STATUS_CROSSOVER ) == STATUS_REVERSED )
            bStatus[ 4 ] = CABLE_REVERSED;
        goto GetCableStatusDone;
    }

    /* Put in 10 Mbps mode. */

    HW_READ_PHY_CTRL( pHardware, phy, Control );
    Data = Control;
    Data &= ~( PHY_AUTO_NEG_ENABLE | PHY_SPEED_100MBIT | PHY_FULL_DUPLEX );
    LowSpeed = Data;
    HW_WRITE_PHY_CTRL( pHardware, phy, Data );

    Crossover = Data;

    for ( i = 0; i < 2; i++ )
    {

#ifdef KS_PCI_BUS
        phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
        HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
            PHY_BANK_INTERVAL ));
#endif
        Data = Crossover;

        /* Disable auto MDIX. */
        Data |= PHY_AUTO_MDIX_DISABLE;
        if ( 0 == i )
            Data &= ~PHY_FORCE_MDIX;
        else
            Data |= PHY_FORCE_MDIX;
        HW_WRITE_PHY_CROSSOVER( pHardware, phy, Data );

        /* Disable transmitter. */
        Data |= PHY_TRANSMIT_DISABLE;
        HW_WRITE_PHY_CTRL( pHardware, phy, Data );

        /* Wait at most 1 second.*/
        DelayMillisec( 100 );

        /* Enable transmitter. */
        Data &= ~PHY_TRANSMIT_DISABLE;
        HW_WRITE_PHY_CTRL( pHardware, phy, Data );

        /* Start cable diagnostic test. */
#ifdef KS_PCI_BUS
        phy = REG_PHY_SPECIAL_OFFSET + bPhy * PHY_SPECIAL_INTERVAL;

#else
        HardwareSelectBank( pHardware, REG_PHY_SPECIAL_BANK );
#endif
        HW_READ_PHY_LINK_MD( pHardware, phy, Data );
        Data |= PHY_START_CABLE_DIAG;
        HW_WRITE_PHY_LINK_MD( pHardware, phy, Data );
        cnTimeOut = PHY_RESET_TIMEOUT;
        do
        {
            if ( !--cnTimeOut )
            {
                break;
            }
            DelayMillisec( 10 );
            HW_READ_PHY_LINK_MD( pHardware, phy, Data );
        } while ( ( Data & PHY_START_CABLE_DIAG ) );

        ulLength[ i ] = 0;
        bStatus[ i ] = CABLE_UNKNOWN;

        if ( !( Data & PHY_START_CABLE_DIAG ) )
        {
            ulLength[ i ] = ( Data & PHY_CABLE_FAULT_COUNTER ) *
                CABLE_LEN_MULTIPLIER;
            Data &= PHY_CABLE_DIAG_RESULT;
            switch ( Data )
            {
                case PHY_CABLE_STAT_NORMAL:
                    bStatus[ i ] = CABLE_GOOD;
                    break;
                case PHY_CABLE_STAT_OPEN:
                    bStatus[ i ] = CABLE_OPEN;
                    break;
                case PHY_CABLE_STAT_SHORT:
                    bStatus[ i ] = CABLE_SHORT;
                    break;
            }
        }
        if ( CABLE_GOOD == bStatus[ i ] )
        {
            ulLength[ i ] = 1;
        }
    }

#ifdef KS_PCI_BUS
    phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
        PHY_BANK_INTERVAL ));
#endif
    HW_WRITE_PHY_CROSSOVER( pHardware, phy, Crossover );
    HW_WRITE_PHY_CTRL( pHardware, phy, Control );
    Control |= PHY_AUTO_NEG_RESTART;
    HW_WRITE_PHY_CTRL( pHardware, phy, Control );

    ulLength[ 4 ] = ulLength[ 0 ];
    bStatus[ 4 ] = bStatus[ 0 ];
    for ( i = 1; i < 2; i++ )
    {
        if ( CABLE_GOOD == bStatus[ 4 ] )
        {
            if ( bStatus[ i ] != CABLE_GOOD )
            {
                bStatus[ 4 ] = bStatus[ i ];
                ulLength[ 4 ] = ulLength[ i ];
                break;
            }
        }
    }

GetCableStatusDone:
    /* Overall status */
    *pulData++ = ulLength[ 4 ];
    *pulData++ = bStatus[ 4 ];

    /* Pair 1-2 */
    *pulData++ = ulLength[ 0 ];
    *pulData++ = bStatus[ 0 ];

    /* Pair 3-6 */
    *pulData++ = ulLength[ 1 ];
    *pulData++ = bStatus[ 1 ];

    /* Pair 4-5 */
    *pulData++ = 0;
    *pulData++ = CABLE_UNKNOWN;

    /* Pair 7-8 */
    *pulData++ = 0;
    *pulData++ = CABLE_UNKNOWN;

}  /* HardwareGetCableStatus */


/*
    HardwareGetLinkStatus

    Description:
        This routine is used to get the link status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        void* pBuffer
            Buffer to store the link status.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareGetLinkStatus_PCI
#else
void HardwareGetLinkStatus_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPhy,
    void*     pBuffer )
{
    PULONG pulData = ( PULONG ) pBuffer;
    ULONG  ulStatus;
    USHORT Capable;
    USHORT Crossover;
    USHORT Data;
    USHORT Link;
    USHORT Polarity;
    USHORT Status;
    int    phy;

    phy = bPhy;

#ifdef KS_PCI_BUS
    phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
        PHY_BANK_INTERVAL ));
#endif
    HW_READ_PHY_LINK_STATUS( pHardware, phy, Capable );
    ulStatus = 0;
    if ( ( Capable & PHY_LINK_STATUS ) )
        ulStatus = 1;

    /* Link status */
    *pulData++ = ulStatus;

#ifdef KS_PCI_BUS
    {
        ULONG ulAddr;

        PORT_CTRL_ADDR( bPhy, ulAddr );
        ulAddr += REG_PORT_STATUS_OFFSET;
        HW_READ_WORD( pHardware, ulAddr, &Link );
    }

#else
    {
        UCHAR bBank;

        bBank = REG_PORT_LINK_STATUS_BANK + phy * PORT_BANK_INTERVAL;
        HardwareSelectBank( pHardware, bBank );
        HW_READ_WORD( pHardware, REG_PORT_STATUS_OFFSET, &Link );
        HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
            PHY_BANK_INTERVAL ));
    }
#endif
    HW_READ_PHY_AUTO_NEG( pHardware, phy, Data );
    HW_READ_PHY_REM_CAP( pHardware, phy, Status );

    HW_READ_PHY_CROSSOVER( pHardware, phy, Crossover );

#ifdef KS_PCI_BUS
    phy = REG_PHY_SPECIAL_OFFSET + bPhy * PHY_SPECIAL_INTERVAL;

#else
    HardwareSelectBank( pHardware, REG_PHY_SPECIAL_BANK );
#endif
    HW_READ_PHY_POLARITY( pHardware, phy, Polarity );

    ulStatus = 100000;

    if ( ( Link & PHY_STAT_SPEED_100MBIT ) )
        ulStatus = 1000000;

    /* Link speed */
    *pulData++ = ulStatus;

    ulStatus = 0;

    if ( ( Link & PHY_STAT_FULL_DUPLEX ) )
        ulStatus |= STATUS_FULL_DUPLEX;

    if ( ( Polarity & PHY_STAT_MDIX ) )
        ulStatus |= STATUS_CROSSOVER;

    if ( ( Polarity & PHY_STAT_REVERSED_POLARITY ) )
        ulStatus |= STATUS_REVERSED;

    /* Duplex mode with crossover and reversed polarity */
    *pulData++ = ulStatus;

    ulStatus = 0;
    if ( ( Capable & PHY_100BTX_FD_CAPABLE ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Capable & PHY_100BTX_CAPABLE ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Capable & PHY_10BT_FD_CAPABLE ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Capable & PHY_10BT_CAPABLE ) )
        ulStatus |= LINK_10MBPS_HALF;

    ulStatus |= LINK_SYM_PAUSE;

    /* Capability */
    *pulData++ = ulStatus;

    ulStatus = 0;
#if 0
    if ( ( Data & PHY_AUTO_NEG_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
#endif
    if ( ( Data & PHY_AUTO_NEG_SYM_PAUSE ) )
        ulStatus |= LINK_SYM_PAUSE;
    if ( ( Data & PHY_AUTO_NEG_100BTX_FD ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Data & PHY_AUTO_NEG_100BTX ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Data & PHY_AUTO_NEG_10BT_FD ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Data & PHY_AUTO_NEG_10BT ) )
        ulStatus |= LINK_10MBPS_HALF;

    if ( !( Crossover & PHY_AUTO_MDIX_DISABLE ) )
        ulStatus |= LINK_AUTO_MDIX;
    else if ( ( Crossover & PHY_FORCE_MDIX ) )
        ulStatus |= LINK_MDIX;
    ulStatus |= LINK_AUTO_POLARITY;

    /* Auto-Negotiation advertisement */
    *pulData++ = ulStatus;

    ulStatus = 0;
#if 0
    if ( ( Status & PHY_REMOTE_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
#endif
    if ( ( Status & PHY_REMOTE_SYM_PAUSE ) )
        ulStatus |= LINK_SYM_PAUSE;
    if ( ( Status & PHY_REMOTE_100BTX_FD ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Status & PHY_REMOTE_100BTX ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Status & PHY_REMOTE_10BT_FD ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Status & PHY_REMOTE_10BT ) )
        ulStatus |= LINK_10MBPS_HALF;

    /* Link parnter capabilities */
    *pulData++ = ulStatus;
}  /* HardwareGetLinkStatus */


/*
    HardwareSetCapabilities

    Description:
        This routine is used to set the link capabilities.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        ULONG ulCapabilities
            A set of flags indicating different capabilities.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareSetCapabilities_PCI
#else
void HardwareSetCapabilities_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPhy,
    ULONG     ulCapabilities )
{
#ifdef KS_PCI_BUS
    ULONG  InterruptMask;

#else
    USHORT InterruptMask;
#endif
    USHORT Data;
    int    phy;
    int    cnTimeOut = PHY_RESET_TIMEOUT;

ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );
    HardwareDisable( pHardware );

    phy = bPhy;

#ifdef KS_PCI_BUS
    phy = REG_PHY_1_CTRL_OFFSET + bPhy * PHY_CTRL_INTERVAL;

#else
    HardwareSelectBank( pHardware, ( UCHAR )( REG_PHY_1_CTRL_BANK + phy *
        PHY_BANK_INTERVAL ));
#endif
    HW_READ_PHY_AUTO_NEG( pHardware, phy, Data );

#if 0
    if ( ( ulCapabilities & LINK_ASYM_PAUSE ) )
        Data |= PHY_AUTO_NEG_ASYM_PAUSE;
    else
        Data &= ~PHY_AUTO_NEG_ASYM_PAUSE;
#endif
    if ( ( ulCapabilities & LINK_SYM_PAUSE ) )
        Data |= PHY_AUTO_NEG_SYM_PAUSE;
    else
        Data &= ~PHY_AUTO_NEG_SYM_PAUSE;
    if ( ( ulCapabilities & LINK_100MBPS_FULL ) )
        Data |= PHY_AUTO_NEG_100BTX_FD;
    else
        Data &= ~PHY_AUTO_NEG_100BTX_FD;
    if ( ( ulCapabilities & LINK_100MBPS_HALF ) )
        Data |= PHY_AUTO_NEG_100BTX;
    else
        Data &= ~PHY_AUTO_NEG_100BTX;
    if ( ( ulCapabilities & LINK_10MBPS_FULL ) )
        Data |= PHY_AUTO_NEG_10BT_FD;
    else
        Data &= ~PHY_AUTO_NEG_10BT_FD;
    if ( ( ulCapabilities & LINK_10MBPS_HALF ) )
        Data |= PHY_AUTO_NEG_10BT;
    else
        Data &= ~PHY_AUTO_NEG_10BT;
    HW_WRITE_PHY_AUTO_NEG( pHardware, phy, Data );

    HW_READ_PHY_CROSSOVER( pHardware, phy, Data );
    if ( ( ulCapabilities & LINK_AUTO_MDIX ) )
        Data &= ~( PHY_AUTO_MDIX_DISABLE | PHY_FORCE_MDIX );
    else
    {
        Data |= PHY_AUTO_MDIX_DISABLE;
        if ( ( ulCapabilities & LINK_MDIX ) )
            Data |= PHY_FORCE_MDIX;
        else
            Data &= ~PHY_FORCE_MDIX;
    }
    HW_WRITE_PHY_CROSSOVER( pHardware, phy, Data );


    HW_READ_PHY_CTRL( pHardware, phy, Data );
    Data |= PHY_AUTO_NEG_RESTART;
    HW_WRITE_PHY_CTRL( pHardware, phy, Data );

    /* Wait for auto negotiation to complete. */
    DelayMillisec( 1500 );
    HW_READ_PHY_LINK_STATUS( pHardware, phy, Data );
    while ( !( Data & PHY_LINK_STATUS ) )
    {
        if ( !--cnTimeOut )
        {
            break;
        }
        DelayMillisec( 100 );
        HW_READ_PHY_LINK_STATUS( pHardware, phy, Data );
    }

#ifdef DEBUG_TIMEOUT
    cnTimeOut = PHY_RESET_TIMEOUT - cnTimeOut;
    if ( pHardware->m_nWaitDelay[ WAIT_DELAY_AUTO_NEG ] < cnTimeOut )
    {
        DBG_PRINT( "phy auto neg: %d = %x"NEWLINE, cnTimeOut, ( int ) Data );
        pHardware->m_nWaitDelay[ WAIT_DELAY_AUTO_NEG ] = cnTimeOut;
    }
#endif

    /* Check for disconnect. */
    HardwareCheckLink( pHardware );

    /* Check for connect. */
    HardwareCheckLink( pHardware );

    HardwareEnable( pHardware );
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* HardwareSetCapabilities */


/* -------------------------------------------------------------------------- */

/* #ifdef DEF_KS8841 */

/******************************************************************************
 * void ks_byteNumberToBytelist
 *
 * DESCRIPTION:
 *     Convert byte number into a ByteList bit map.
 *
 *     Byte numbers are zero-based up to 63
 *     The ByteList specifies a set of eight bytes, with the first octect specifying
 *     byte 0 through 7, the second octet specifying byte 8 though 15, and so on.
 *     The MSB of the octet represents the lowest numbered byte and LSB represents
 *     the hightest numbered byte.
 *
 *     eg. if byte number 0 are configured (byte 0 is first byte),
 *         the  byteNumber value is 0 ( byte 0 ), after convert,
 *         the *byteList value will be 80:00:00:00 (b31 is byte 0)
 *
 * PARAMETERS:
 *      DWORD  byteNumber
 *          byte number (0 - byte 0, ... 63 - byte 63).
 *
 *      BYTE   *byteList
 *          pointer to a byte array.
 *
 * RETURNS: STATUS.
 *
 */
static
void ks_byteNumberToBytelist
(
    PHARDWARE pHardware,
    BYTE   byteNumber,
    BYTE  *byteList
)
{
/* swap the MSW with the LSW of a 32 bit integer */
#define LLSB(x)     ((x) & 0xff)
#define LNLSB(x)    (((x) >> 8) & 0xff)
#define LNMSB(x)    (((x) >> 16) & 0xff)
#define LMSB(x)     (((x) >> 24) & 0xff)
#define SWAPLONG(x) ((LLSB(x) << 24) |  \
                    (LNLSB(x) << 16) |  \
                    (LNMSB(x) << 8)  |  \
                    (LMSB(x)))

    BYTE   * dList;
    BYTE     byteIndex, byteIndexInByte, i;
    BYTE     bitMasks[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    UINT32 * pdwData = (UINT32 *)byteList;


    for (byteIndex = 0; byteIndex < 32; byteIndex++)
    {
         /* get an octet */
         i = byteIndex / 8;
         dList = &byteList[i];

         if ( byteNumber == byteIndex )
         {
            byteIndexInByte = byteIndex % 8;

            /* set bit */
            *dList |= bitMasks[8 - (byteIndexInByte + 1)];
         }
    }

    /* host is big endian system */
    if ( pHardware->m_boardBusEndianMode & 1 )
    {
        *pdwData = SWAPLONG(*pdwData);
    }

}  /* ks_byteNumberToBytelist */


/*
    HardwareEnableWolMagicPacket

    Description:
        This routine is used to enable the Wake-on-LAN that wake-up signal is caused by
        receipting of a Magic Packet.
        KS8841 device can support D1, D2, or D3 power state by EEPROM setting.
        By default, device supports D3 power state without EEPROM setting.
        The example here is by default D3 power state.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareEnableWolMagicPacket
(
    PHARDWARE pHardware
)
{
    USHORT RegData=0;



    /* Set power management capabilities */

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_POWER_CNTL_BANK );
    HW_READ_WORD( pHardware, REG_POWER_CNTL_OFFSET, &RegData );
#else  /* PCI bus */
    HW_PCI_READ_WORD( pHardware, (CPMC), &RegData );
#endif

    RegData &= POWER_STATE_MASK;
    RegData |= ( POWER_PME_ENABLE | POWER_STATE_D3 );

#ifdef KS_ISA_BUS
    HW_WRITE_WORD( pHardware, REG_POWER_CNTL_OFFSET, RegData );
#else  /* PCI bus */
    HW_PCI_WRITE_WORD( pHardware, (CPMC), RegData );
#endif


    /* Enables the magic packet pattern detection */

    RegData = 0;
#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_WOL_CTRL_BANK );
#endif
    HW_READ_WORD( pHardware, REG_WOL_CTRL_OFFSET, &RegData );

    RegData |= ( WOL_MAGIC_ENABLE );

    HW_WRITE_WORD( pHardware, REG_WOL_CTRL_OFFSET, RegData );

}

/*
    HardwareEnableWolFrame

    Description:
        This routine is used to enable the Wake-on-LAN that wake-up signal is caused by
        receipting of a 'wake-up' packet. The device can support four different
        'wake-up' frames.
        KS8841 device can support D1, D2, or D3 power state by EEPROM setting.
        By default, device supports D3 power state without EEPROM setting.
        The example here is by default D3 power state.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32    dwFrame
            whose wake up frame to be enabled (0 - Frame 0, 1 - Frame 1, 2 - Frame 2, 3 - Frame 3).

    Return (None):
*/

void HardwareEnableWolFrame
(
    PHARDWARE pHardware,
    UINT32    dwFrame
)
{
    USHORT RegData;



    /* Set power management capabilities */

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_POWER_CNTL_BANK );
    HW_READ_WORD( pHardware, REG_POWER_CNTL_OFFSET, &RegData );
#else  /* PCI bus */
    HW_PCI_READ_WORD( pHardware, (CPMC), &RegData );
#endif

    RegData &= POWER_STATE_MASK;
    RegData |= ( POWER_PME_ENABLE | POWER_STATE_D3 );

#ifdef KS_ISA_BUS
    HW_WRITE_WORD( pHardware, REG_POWER_CNTL_OFFSET, RegData );
#else  /* PCI bus */
    HW_PCI_WRITE_WORD( pHardware, (CPMC), RegData );
#endif

    /* Enables the wake up frame pattern detection */

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_WOL_CTRL_BANK );
#endif
    HW_READ_WORD( pHardware, REG_WOL_CTRL_OFFSET, &RegData );

    switch (dwFrame)
    {
        case 0:
            RegData |= ( WOL_FRAME0_ENABLE );
            break;
        case 1:
            RegData |= ( WOL_FRAME1_ENABLE );
            break;
        case 2:
            RegData |= ( WOL_FRAME2_ENABLE );
            break;
        case 3:
            RegData |= ( WOL_FRAME3_ENABLE );
            break;
    }

    HW_WRITE_WORD( pHardware, REG_WOL_CTRL_OFFSET, RegData );
}

/*
    HardwareSetWolFrameCRC

    Description:
        This routine is used to set the expected 32-bit CRC value of the 'Wake up'
        frame pattern.
        The device can support four different 'wake-up' frames.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32    dwFrame
            whose wake up frame CRC to be set (0 - Frame 0, 1 - Frame 1, 2 - Frame 2, 3 - Frame 3).

        UINT32    dwCRC
            expected 32bit CRC value.

    Return (None):
*/

void HardwareSetWolFrameCRC
(
    PHARDWARE pHardware,
    UINT32    dwFrame,
    UINT32    dwCRC
)
{
#ifdef KS_ISA_BUS
    UCHAR bWolBank;
#endif

    if ( dwFrame > 3 )
        return;

    /*  Set expected 32bit CRC value Frame # register */

#ifdef KS_ISA_BUS
    bWolBank = ( REG_WOL_FRAME_0_BANK + dwFrame );
    HardwareSelectBank( pHardware, bWolBank );
#endif

    HW_WRITE_DWORD( pHardware, WOL_FRAME_CRC_OFFSET, dwCRC );
}

/*
    HardwareSetWolFrameByteMask

    Description:
        This routine is used to set the byte mask within 64 byte of the 'Wake up'
        frame pattern to calculate CRC value.
        The device can support four different 'wake-up' frames.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32    dwFrame
            whose wake up frame byte mask to be set (0 - Frame 0, 1 - Frame 1, 2 - Frame 2, 3 - Frame 3).

        UINT8     bByteMask
            byte number to mask to calculate CRC value (0 - byte 0, 63 - byte 63).

    Return (None):
*/

void HardwareSetWolFrameByteMask
(
    PHARDWARE pHardware,
    UINT32    dwFrame,
    UINT8     bByteMask
)
{
    UINT32    RegData=0;
    UINT32  * pdwData;
    UCHAR     byteList[4];
#ifdef KS_ISA_BUS
    UCHAR     bWolBank;
    UCHAR     bWolFrameOffset;
#else
    USHORT    bWolFrameOffset;
#endif


    if ( dwFrame > 3 )
        return;
    if ( bByteMask > 63 )
        return;

    if ( bByteMask > 31 )
    {
        bWolFrameOffset = WOL_FRAME_BYTE2_OFFSET;
        bByteMask -= 32;
    }
    else
    {
        bWolFrameOffset = WOL_FRAME_BYTE0_OFFSET;
    }

    pdwData = (UINT32 *)byteList;
    memset ( &byteList[0], 0, 4);
    ks_byteNumberToBytelist( pHardware, bByteMask, byteList );

    /*  Set byte mask for the device to calculate 32bit CRC value from the Frame pattern */

#ifdef KS_ISA_BUS
    bWolBank = ( REG_WOL_FRAME_0_BANK + dwFrame );
    HardwareSelectBank( pHardware, bWolBank );
#endif

    HW_READ_DWORD( pHardware, bWolFrameOffset, &RegData );

    RegData |= *pdwData;

    HW_WRITE_DWORD( pHardware, bWolFrameOffset, RegData );
}


/*
    HardwareCheckWolPMEStatus

    Description:
        This function is used to check PMEN pin is asserted.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if PMEN pin is asserted; otherwise, FALSE.
*/

BOOLEAN HardwareCheckWolPMEStatus
(
    PHARDWARE pHardware
)
{
    USHORT RegData = 0;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_POWER_CNTL_BANK );
    HW_READ_WORD( pHardware, REG_POWER_CNTL_OFFSET, &RegData );

#else  /* PCI bus */
    HW_PCI_READ_WORD( pHardware, (CPMC), &RegData );
#endif

    return(( RegData & POWER_PME_STATUS ) == POWER_PME_STATUS );
}  /* HardwareCheckWolPMEStatus */


/*
    HardwareClearWolPMEStatus

    Description:
        This routine is used to clear PME_Status to deassert PMEN pin.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareClearWolPMEStatus
(
    PHARDWARE pHardware
)
{
   USHORT  RegData=0;


    /* Clear PME_Status to deassert PMEN pin */

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_POWER_CNTL_BANK );
    HW_READ_WORD( pHardware, REG_POWER_CNTL_OFFSET, &RegData );
#else  /* PCI bus */
    HW_PCI_READ_WORD( pHardware, (CPMC), &RegData );
#endif

    RegData |= ( POWER_PME_STATUS );

#ifdef KS_ISA_BUS
    HW_WRITE_WORD( pHardware, REG_POWER_CNTL_OFFSET, RegData );
#else  /* PCI bus */
    HW_PCI_WRITE_WORD( pHardware, (CPMC), RegData );
#endif

}


/* -------------------------------------------------------------------------- */

UCHAR TestPacket[] =
{
/*  0 */    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* ff:ff:ff:ff:ff:ff (DA) */
/*  6 */    0x08, 0x00, 0x70, 0x22, 0x44, 0x55,     /* 08:00:70:22:44:55 (SA) */
/* 12 */    0x08, 0x06,                             /* ARP */
/* 14 */    0x00, 0x01,                             /* Ethernet */
/* 16 */    0x08, 0x00,                             /* IP */
/* 18 */    0x06, 0x04,
/* 20 */    0x00, 0x01,                             /* Request */
/* 22 */    0x08, 0x00, 0x70, 0x22, 0x44, 0x55,     /* 08:00:70:22:44:55 (SA) */
/* 28 */    0xC0, 0xA8, 0x01, 0x01,                 /* 192.168.1.1  (Source IP) */
/* 32 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 00:00:00:00:00:00 (DA) */
/* 38 */    0xC0, 0xA8, 0x01, 0x1E,                 /* 192.168.1.30 (Dest IP) */
/* 42 */    0x5A, 0xA5,                             /* Data */
};

/*
    TestLoopback

    Description:
        This function is used to test the loopback function.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if loopback is working; otherwise, FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN TestLoopback_PCI
#else
BOOLEAN TestLoopback_ISA
#endif
(
    PHARDWARE pHardware )
{
#ifdef KS_PCI_BUS
    ULONG      IntEnable;
    ULONG      IntStatus;
    ULONG      IntMask = INT_TX | INT_RX;
#if 0
    PTDescInfo pRx = &pHardware->m_RxDescInfo;
#endif
    PTDescInfo pTx = &pHardware->m_TxDescInfo;

#else
    USHORT     IntEnable;
    USHORT     IntStatus;
    USHORT     IntMask = INT_TX | INT_RX;
#endif
    int        nLength = 42;
    int        cnTimeOut = 100;
    BOOLEAN    rc = TRUE;
    USHORT     i;

#ifdef KS_PCI_BUS
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_LOOPBACK;

#else
    pHardware->m_wTransmitConfig |= TX_CTRL_MAC_LOOPBACK;

#endif
    HardwareEnable( pHardware );

#ifdef KS_PCI_BUS
    if ( HardwareAllocPacket( pHardware, nLength, 1 ) )

#else
    if ( HardwareAllocPacket( pHardware, nLength ) )
#endif
    {

#ifdef KS_PCI_BUS
        HardwareSetTransmitBuffer( pHardware, 0 );
        HardwareSetTransmitLength( pHardware, nLength );

#else
        HardwareSetTransmitLength( pHardware, nLength );

        HW_WRITE_BUFFER( pHardware, REG_DATA_OFFSET, TestPacket, nLength );

#endif
        HardwareSendPacket( pHardware );

        do {
            HardwareReadInterrupt( pHardware, &IntStatus );
            IntEnable = IntStatus & IntMask;
        } while ( !IntEnable  &&  cnTimeOut-- );
        if ( ( IntEnable & INT_TX ) )
        {
            HardwareTransmitDone( pHardware );

            /* Acknowledge the interrupt. */
            HardwareAcknowledgeTransmit( pHardware );
            IntEnable &= ~INT_TX;
        }
        else
        {

#ifdef KS_PCI_BUS
            HardwareFreeTxPacket( pTx );
#endif
            rc = FALSE;
        }

        cnTimeOut = 100;
        while ( !IntEnable  &&  cnTimeOut-- )
        {
            HardwareReadInterrupt( pHardware, &IntStatus );
            IntEnable = IntStatus & IntMask;
        }
        if ( ( IntEnable & INT_RX ) )
        {
            /* Receive the packet. */
            HardwareReceive( pHardware );

            /* Acknowledge the interrupt. */
            HardwareAcknowledgeReceive( pHardware );

            for ( i = 0; i < nLength; i++ )
            {
                if ( pHardware->m_bLookahead[ i ] != TestPacket[ i ] )
                {
                    rc = FALSE;
                    break;
                }
            }
        }
        else
            rc = FALSE;
    }
    else
        rc = FALSE;

#ifdef KS_PCI_BUS
    pHardware->m_dwTransmitConfig &= ~DMA_TX_CTRL_LOOPBACK;

#else
    pHardware->m_wTransmitConfig &= ~TX_CTRL_MAC_LOOPBACK;
#endif
    HardwareDisable( pHardware );
    return( rc );
}  /* TestLoopback */


USHORT act_read_net_irq_mask(PHARDWARE pHardware)
{
	USHORT    rReadInterrupt;

	HardwareReadRegWord( pHardware, REG_INT_MASK_BANK, REG_INT_MASK_OFFSET,
		&rReadInterrupt );

	return rReadInterrupt;
}

USHORT act_read_net_irq_status(PHARDWARE pHardware)
{
	USHORT    rReadInterrupt;

	HardwareReadRegWord( pHardware, REG_INT_STATUS_BANK, REG_INT_STATUS_OFFSET,
		&rReadInterrupt );

	return rReadInterrupt;
}

UCHAR act_read_net_bank(PHARDWARE pHardware )
{
	UCHAR bBank;

	HW_READ_BYTE( pHardware, REG_BANK_SEL_OFFSET, &bBank);

	return bBank;
}  /* act_read_net_bank */


void mem_dump_long(void *buf, int len)
{
	int i;
	unsigned int *p;
	
	for(i=0, p=(unsigned int *)buf; i<len; ) {
		if( i % 16 == 0 )
			printk("%08x:  ", p );
		printk("%08x ", *p);
		p++;
		i += 4;
		if( i % 16 == 0 )
			printk("\n");
	}
	if( i % 16 != 0 )
		printk("\n");
	printk("\n");
}
