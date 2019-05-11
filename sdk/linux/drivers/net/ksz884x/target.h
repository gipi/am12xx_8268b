/* ---------------------------------------------------------------------------
          Copyright (c) 2003-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    target.h - Target platform functions header

    Author      Date        Description
    THa         12/10/03    Created file.
    THa         02/23/04    Use 32-bit buffer read/write to improve
                            performance.
    THa         09/27/04    Updated for PCI version.
    THa         08/15/05    Added PCI configuration I/O.
    THa         01/13/06    Transfer dword-aligned data for performance.
    THa         02/28/06    Do not use HW_WRITE_BYTE because of limitation of
                            some hardware platforms.
    THa         06/02/06    Define HardwareDisableInterruptSync.
   ---------------------------------------------------------------------------
*/


#ifndef __TARGET_H
#define __TARGET_H

#include <asm/mach-am7x/actions_io.h>
#include <asm/mach-am7x/actions_regs.h>

static void inline act_net_debug(void)
{
}


#ifdef KS_ISA_BUS

/* Only support generic bus for now. */
#if 1
#define SH_BUS
#endif

/* SuperH needs to be aligned at 32-bit for memory i/o. */
#if 1
#define SH_32BIT_ALIGNED
#endif

/* Some hardware platforms can only do 32-bit access. */
/* Temporary workaround.  Need to rewrite driver for better performance. */
#if 0
#define SH_32BIT_ACCESS_ONLY
#endif

/* Some hardware platforms cannot do byte write successfully. */
#define SH_16BIT_WRITE

#endif /* #ifdef KS_ISA_BUS */


/* No auto-negotiation for link speed/duplex mode in KSZ8861/2 with 100FX fiber mode.
   when user define "FIBER_100FX", the driver set KS8861/2 port 1 to 100BT/full duplex */
#if 0
#define FIBER_100FX
#endif

/* To configure two separated Network interface, set Port1 and Port3 in
   one port-base vlan, and Port2 and Port3 in another port-base vlan.
 */
#if 0
#define TWO_NETWORK_INTERFACE
#endif


#define HardwareDisableInterruptSync( pHardware )  \
    HardwareDisableInterrupt( pHardware )


#if defined(_WIN32) || defined(DEF_LINUX)


/* -------------------------------------------------------------------------- *
 *                               WIN32 OS                                     *
 * -------------------------------------------------------------------------- */

#ifdef _WIN32

#define FAR
#ifdef UNDER_CE
#define NEWLINE    "\r\n"
#else
#define NEWLINE    "\n"
#endif

typedef unsigned char   BYTE;
typedef unsigned char   UINT8;

#ifdef NDIS_MINIPORT_DRIVER
#include <ndis.h>


#undef HardwareDisableInterruptSync
void HardwareDisableInterruptSync ( void* pHardware );


#define DBG_PRINT  DbgPrint


#define CPU_TO_LE32( x )  ( x )
#define LE32_TO_CPU( x )  ( x )


#ifdef KS_ISA_BUS
#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    NdisRawReadPortBufferUlong(( phwi )->m_ulVIoAddr + addr, data,          \
        (( len ) + 3 ) >> 2 )

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    NdisRawWritePortBufferUlong(( phwi )->m_ulVIoAddr + addr, data,         \
        (( len ) + 3 ) >> 2 );                                              \
}

#define HW_READ_BUFFER_BYTE( phwi, addr, data, len )                        \
    NdisRawReadPortBufferUchar(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_WRITE_BUFFER_BYTE( phwi, addr, data, len )                       \
    NdisRawWritePortBufferUchar(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_READ_BUFFER_WORD( phwi, addr, data, len )                        \
    NdisRawReadPortBufferUshort(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_WRITE_BUFFER_WORD( phwi, addr, data, len )                       \
    NdisRawWritePortBufferUshort(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_READ_BUFFER_DWORD( phwi, addr, data, len )                       \
    NdisRawReadPortBufferUlong(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_WRITE_BUFFER_DWORD( phwi, addr, data, len )                      \
    NdisRawWritePortBufferUlong(( phwi )->m_ulVIoAddr + addr, data, len )

#define HW_READ_BYTE( phwi, addr, data )                                    \
    NdisRawReadPortUchar(( phwi )->m_ulVIoAddr + addr, data )

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    NdisRawWritePortUchar(( phwi )->m_ulVIoAddr + addr, data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    NdisRawReadPortUshort(( phwi )->m_ulVIoAddr + addr, data )

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    NdisRawWritePortUshort(( phwi )->m_ulVIoAddr + addr, data )

#define HW_READ_DWORD( phwi, addr, data )                                   \
    NdisRawReadPortUlong(( phwi )->m_ulVIoAddr + addr, data )

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    NdisRawWritePortUlong(( phwi )->m_ulVIoAddr + addr, data )
#endif

#ifdef KS_PCI_BUS
#define HW_READ_BYTE( phwi, addr, data )                                    \
    *( data ) = *(( volatile UCHAR* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    *(( volatile UCHAR* )(( phwi )->m_pVirtualMemory + addr )) = ( UCHAR )( data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    *( data ) = *(( volatile USHORT* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    *(( volatile USHORT* )(( phwi )->m_pVirtualMemory + addr )) = ( USHORT )( data )

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *( data ) = *(( volatile ULONG* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    *(( volatile ULONG* )(( phwi )->m_pVirtualMemory + addr )) = ( ULONG )( data )

BOOLEAN PciReadConfig ( PVOID, int, PULONG );
BOOLEAN PciReadConfigByte ( PVOID, int, PUCHAR );
BOOLEAN PciReadConfigWord ( PVOID, int, PUSHORT );
BOOLEAN PciWriteConfig ( PVOID, int, ULONG );
BOOLEAN PciWriteConfigByte ( PVOID, int, UCHAR );
BOOLEAN PciWriteConfigWord ( PVOID, int, USHORT );

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    PciReadConfigByte(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    PciReadConfigWord(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    PciReadConfig(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    PciWriteConfigByte(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    PciWriteConfigWord(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    PciWriteConfig(( phwi )->m_pPciCfg, addr, data )

#endif

#define MOVE_MEM( dest, src, len )                                          \
    NdisMoveMemory( dest, src, len )

#else  /* #ifdef NDIS_MINIPORT_DRIVER */
#include <conio.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char   UCHAR;
typedef unsigned char*  PUCHAR;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned long   ULONG;
typedef unsigned long*  PULONG;
typedef unsigned long   ULONGLONG;
typedef unsigned long*  PULONGLONG;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;


#define FALSE  0
#define TRUE   1


#define DBG_PRINT  printf


#define CPU_TO_LE32( x )  ( x )
#define LE32_TO_CPU( x )  ( x )


#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    HardwareReadBuffer( phwi, addr, ( PULONG ) data, ( len )  )
#if (0)
    HardwareReadBuffer( phwi, addr, ( PUSHORT ) data, ( len ) >> 1 )
#endif

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
    HardwareWriteBuffer( phwi, addr, ( PULONG ) data, ( len ) )
#if (0)
    HardwareWriteBuffer( phwi, addr, ( PUSHORT ) data, ( len ) >> 1 )
#endif

#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = _inp(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    _outp(( USHORT )(( phwi )->m_ulVIoAddr + addr ), data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = _inpw(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    _outpw(( USHORT )(( phwi )->m_ulVIoAddr + addr ), ( USHORT )( data ))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = _inpd(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    _outpd(( USHORT )(( phwi )->m_ulVIoAddr + addr ), data )

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )

#endif /* #ifdef NDIS_MINIPORT_DRIVER */
#endif /* #ifdef _WIN32 */

#define MEM_READ_BYTE( phwi, addr, data )                                   \
    *data = *(( PUCHAR )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_BYTE( phwi, addr, data )                                  \
    *(( PUCHAR )(( phwi )->m_pVirtualMemory + addr )) = ( UCHAR )( data )

#define MEM_READ_WORD( phwi, addr, data )                                   \
    *data = *(( PUSHORT )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_WORD( phwi, addr, data )                                  \
    *(( PUSHORT )(( phwi )->m_pVirtualMemory + addr )) = ( USHORT )( data )

#define MEM_READ_DWORD( phwi, addr, data )                                  \
    *data = *(( PULONG )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_DWORD( phwi, addr, data )                                 \
    *(( PULONG )(( phwi )->m_pVirtualMemory + addr )) = ( ULONG )( data )


/* -------------------------------------------------------------------------- *
 *                             LINUX OS                                       *
 * -------------------------------------------------------------------------- */

#ifdef DEF_LINUX
#include <asm/io.h>
#include <linux/mm.h>
#include <linux/pci.h>


typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef unsigned char*  PUCHAR;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef unsigned long   UINT32;
typedef unsigned long*  PULONG;
typedef void*           PVOID;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;

#if 0
typedef unsigned long long  ULONGLONG;
typedef unsigned long long* PULONGLONG;
#else
typedef unsigned long  ULONGLONG;
typedef unsigned long* PULONGLONG;
#endif


#define MIO_DWORD( x )  *(( volatile unsigned long* )( x ))
#define MIO_WORD( x )   *(( volatile unsigned short* )( x ))
#define MIO_BYTE( x )   *(( volatile unsigned char* )( x ))

#define FAR

#define FALSE  0
#define TRUE   1


#if 0
#undef HardwareDisableInterruptSync
void HardwareDisableInterruptSync ( void* pHardware );
#endif


#define NEWLINE    "\n"
#define DBG_PRINT  printk


#define CPU_TO_LE32( x )  cpu_to_le32( x )
#define LE32_TO_CPU( x )  le32_to_cpu( x )


#ifdef KS_ISA_BUS

/* Special generic bus */
#ifdef SH_BUS

/* 32-bit access */
#ifdef SH_32BIT_ALIGNED
#define HW_READ_BUFFER( phwi, addr, data, len )                             \
{                                                                           \
    PULONG pData = ( PULONG )( data );                                      \
    int    nRead = (( len ) + 3 ) >> 2;                                     \
    while ( nRead-- )                                                       \
        *pData++ = MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr ));             \
}

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    PULONG pData = ( PULONG )( data );                                      \
    int    nWrite = (( len ) + 3 ) >> 2;                                    \
    while ( nWrite-- )                                                      \
        MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr )) = *pData++;             \
}

/* 16-bit access */
#else
#define HW_READ_BUFFER( phwi, addr, data, len )                             \
{                                                                           \
    PUSHORT pData = ( PUSHORT )( data );                                    \
    int     nRead = (( len ) + 3 ) >> 2;                                    \
    while ( nRead-- ) {                                                     \
        *pData++ = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ));              \
        *pData++ = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ) + 2 );         \
    }                                                                       \
}

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    PUSHORT pData = ( PUSHORT )( data );                                    \
    int     nWrite = (( len ) + 3 ) >> 2;                                   \
    while ( nWrite-- ) {                                                    \
        MIO_WORD(( phwi )->m_ulVIoAddr + ( addr )) = *pData++;              \
        MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ) + 2 ) = *pData++;         \
    }                                                                       \
}
#endif

#define HW_READ_BUFFER_WORD( phwi, addr, data, len )                        \
{                                                                           \
    PUSHORT pData = ( PUSHORT )( data );                                    \
    int     nRead = (( len ) + 3 ) >> 2;                                    \
    while ( nRead-- ) {                                                     \
        *pData++ = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ));              \
        *pData++ = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ) + 2 );         \
    }                                                                       \
}

#define HW_WRITE_BUFFER_WORD( phwi, addr, data, len )                       \
{                                                                           \
    PUSHORT pData = ( PUSHORT )( data );                                    \
    int     nWrite = (( len ) + 3 ) >> 2;                                   \
    while ( nWrite-- ) {                                                    \
        MIO_WORD(( phwi )->m_ulVIoAddr + ( addr )) = *pData++;              \
        MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ) + 2 ) = *pData++;         \
    }                                                                       \
}

#define HW_READ_BUFFER_DWORD( phwi, addr, data, len )                       \
{                                                                           \
    PULONG pData = ( PULONG )( data );                                      \
    int    nRead = (( len ) + 3 ) >> 2;                                     \
    while ( nRead-- )                                                       \
        *pData++ = MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr ));             \
}

#define HW_WRITE_BUFFER_DWORD( phwi, addr, data, len )                      \
{                                                                           \
    PULONG pData = ( PULONG )( data );                                      \
    int    nWrite = (( len ) + 3 ) >> 2;                                    \
    while ( nWrite-- )                                                      \
        MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr )) = *pData++;             \
}

#ifndef SH_32BIT_ACCESS_ONLY
/*
 *  Access ks884x register by BYTE\WORD\DWORD
 */

#if 0	/* hjk debug */

#define HW_READ_BYTE( phwi, addr, data )                                 {act_net_debug();    \
    *( data ) = MIO_BYTE(( phwi )->m_ulVIoAddr + ( addr )); \
}

#define HW_WRITE_BYTE( phwi, addr, data )                                {act_net_debug();   \
    MIO_BYTE(( phwi )->m_ulVIoAddr + ( addr )) = ( UCHAR )( data ); \
}

#define HW_READ_WORD( phwi, addr, data )                                    {act_net_debug();\
    *( data ) = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr )); \
}

#define HW_WRITE_WORD( phwi, addr, data )                                   {act_net_debug();\
    MIO_WORD(( phwi )->m_ulVIoAddr + ( addr )) = ( USHORT )( data ); \
}

#else	/* default define */
#define HW_READ_BYTE( phwi, addr, data )	\
    *( data ) = MIO_BYTE(( phwi )->m_ulVIoAddr + ( addr ))

#define HW_WRITE_BYTE( phwi, addr, data )	\
    MIO_BYTE(( phwi )->m_ulVIoAddr + ( addr )) = ( UCHAR )( data )

#define HW_READ_WORD( phwi, addr, data )	\
    *( data ) = MIO_WORD(( phwi )->m_ulVIoAddr + ( addr ))

#define HW_WRITE_WORD( phwi, addr, data )	\
    MIO_WORD(( phwi )->m_ulVIoAddr + ( addr )) = ( USHORT )( data )

#endif

#else
/*
 *  Access ks884x register by DWORD only
 */

#define HW_READ_BYTE( phwi, addr, data )                                    \
{                                                                           \
    UCHAR   shiftBit;                                                       \
    ULONG   dwDataRead;                                                     \
                                                                            \
    switch ( (addr) & 0x03 )                                                \
    {                                                                       \
        case 0:                                                             \
            *data = inb(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )));    \
            break;                                                          \
        default:                                                            \
            shiftBit=((addr) & 0x03) << 3;                                  \
            HW_READ_DWORD( phwi, ((addr) & 0x0C), &dwDataRead ) ;           \
            *data = (UCHAR)(dwDataRead >>= shiftBit);                       \
            break;                                                          \
    }                                                                       \
}

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
{                                                                           \
    UCHAR   addrByDwordAligned=(addr) & 0x0C;                               \
    UCHAR   shiftBit=((addr) & 0x03) << 3;                                  \
    ULONG   dwDataMask=0xFF;                                                \
    ULONG   dwDataRead;                                                     \
    ULONG   dwDataWrite=0;                                                  \
                                                                            \
    dwDataMask <<= shiftBit;                                                \
    HW_READ_DWORD( phwi, addrByDwordAligned, &dwDataRead ) ;                \
    dwDataRead &= ~dwDataMask ;                                             \
    dwDataWrite = ((( data ) << shiftBit ) | dwDataRead );                  \
    HW_WRITE_DWORD( phwi, addrByDwordAligned, dwDataWrite ) ;               \
}

#define HW_READ_WORD( phwi, addr, data )                                    \
{                                                                           \
    UCHAR   shiftBit;                                                       \
    ULONG   dwDataRead;                                                     \
                                                                            \
    switch ( (addr) & 0x03 )                                                \
    {                                                                       \
        case 0:                                                             \
            *data = inw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )));    \
            break;                                                          \
        case 2:                                                             \
            shiftBit=((addr) & 0x03) << 3;                                  \
            HW_READ_DWORD( phwi, ((addr) & 0x0C), &dwDataRead ) ;           \
            *data = (USHORT)(dwDataRead >>= shiftBit);                      \
            break;                                                          \
    }                                                                       \
}

#define HW_WRITE_WORD( phwi, addr, data )                                   \
{                                                                           \
    UCHAR   addrByDwordAligned=(addr) & 0x0C;                               \
    UCHAR   shiftBit=((addr) & 0x03) << 3;                                  \
    ULONG   dwDataMask=0xFFFF;                                              \
    ULONG   dwDataRead;                                                     \
    ULONG   dwDataWrite=0;                                                  \
                                                                            \
    dwDataMask <<= shiftBit;                                                \
    HW_READ_DWORD( phwi, addrByDwordAligned, &dwDataRead ) ;                \
    dwDataRead &= ~dwDataMask ;                                             \
    dwDataWrite = ((( data ) << shiftBit ) | dwDataRead );                  \
    HW_WRITE_DWORD( phwi, addrByDwordAligned, dwDataWrite ) ;               \
}
#endif

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *( data ) = MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    MIO_DWORD(( phwi )->m_ulVIoAddr + ( addr )) = ( ULONG )( data )

#else
#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    insl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data,              \
        (( len ) + 3 ) >> 2 )

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    outsl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data,             \
        (( len ) + 3 ) >> 2 );                                              \
}

#define HW_READ_BUFFER_WORD( phwi, addr, data, len )                        \
    insw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_WRITE_BUFFER_WORD( phwi, addr, data, len )                       \
    outsw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_READ_BUFFER_DWORD( phwi, addr, data, len )                       \
    insl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_WRITE_BUFFER_DWORD( phwi, addr, data, len )                      \
    outsl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = inb(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    outb( data, ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = inw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    outw(( USHORT )( data ), ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = inl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    outl( data, ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))
#endif
#endif

#ifdef KS_PCI_BUS

/* Renesas 7751R Solution Engine kernel code uses functions to access I/O. */
#ifdef CONFIG_SH_7751_SOLUTION_ENGINE
#define HW_READ_BYTE( phwi, addr, data )                                    \
    *( data ) = MIO_BYTE(( phwi )->m_pVirtualMemory + ( addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    MIO_BYTE(( phwi )->m_pVirtualMemory + ( addr )) = ( UCHAR )( data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    *( data ) = MIO_WORD(( phwi )->m_pVirtualMemory + ( addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    MIO_WORD(( phwi )->m_pVirtualMemory + ( addr )) = ( USHORT )( data )

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *( data ) = MIO_DWORD(( phwi )->m_pVirtualMemory + ( addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
{                                                                           \
    MIO_WORD(( phwi )->m_pVirtualMemory + ( addr )) = ( USHORT )( data );   \
    MIO_WORD(( phwi )->m_pVirtualMemory + ( addr ) + 2 ) =                  \
        ( USHORT )(( data ) >> 16 );                                        \
}

#else
#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = readb(( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    writeb( data, ( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = readw(( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    writew(( USHORT )( data ), ( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = readl(( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    writel( data, ( void* )(( phwi )->m_pVirtualMemory + ( addr )))
#endif

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    pci_read_config_byte(( struct pci_dev* )( phwi )->m_pPciCfg,            \
        addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    pci_read_config_word(( struct pci_dev* )( phwi )->m_pPciCfg,            \
        addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    pci_read_config_dword(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    pci_write_config_byte(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    pci_write_config_word(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    pci_write_config_dword(( struct pci_dev* )( phwi )->m_pPciCfg,          \
        addr, data )

#endif

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )

#endif /* #ifdef DEF_LINUX */

#endif /* #if defined(_WIN32) || defined(DEF_LINUX) */


/* -------------------------------------------------------------------------- *
 *                             QC TEST                                        *
 * -------------------------------------------------------------------------- */

#ifdef KS_ARM

#define cpu_to_le16(x)  ((unsigned short)(x))
#define cpu_to_le32(x)  ((unsigned long)(x))
#define le16_to_cpu(x)  ((unsigned short)(x))
#define le32_to_cpu(x)  ((unsigned long)(x))


/* asm/io.h */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int  *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int  *)(a) = (v))


#define __raw_writeb(v,a)		__arch_putb(v,a)
#define __raw_writew(v,a)		__arch_putw(v,a)
#define __raw_writel(v,a)		__arch_putl(v,a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)


/* asm/arch/io.h */
#define __io(a)				(a)
#define __mem_pci(a)			((unsigned long)(a))


#ifdef __io
#define outb(v,p)			__raw_writeb(v,__io(p))
#define outw(v,p)			__raw_writew(cpu_to_le16(v),__io(p))
#define outl(v,p)			__raw_writel(cpu_to_le32(v),__io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p,d,l)			__raw_writesb(__io(p),d,l)
#define outsw(p,d,l)			__raw_writesw(__io(p),d,l)
#define outsl(p,d,l)			__raw_writesl(__io(p),d,l)

#define insb(p,d,l)			__raw_readsb(__io(p),d,l)
#define insw(p,d,l)			__raw_readsw(__io(p),d,l)
#define insl(p,d,l)			__raw_readsl(__io(p),d,l)
#endif


/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(c) ({ unsigned int __v = __raw_readb(__mem_pci(c)); __v; })
#define readw(c) ({ unsigned int __v = le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })
#define readl(c) ({ unsigned int __v = le32_to_cpu(__raw_readl(__mem_pci(c))); __v; })

#define writeb(v,c)		__raw_writeb(v,__mem_pci(c))
#define writew(v,c)		__raw_writew(cpu_to_le16(v),__mem_pci(c))
#define writel(v,c)		__raw_writel(cpu_to_le32(v),__mem_pci(c))

#endif	/* __mem_pci */


#define HZ  100


extern volatile unsigned long jiffies;


#include "platform.h"


typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef unsigned char*  PUCHAR;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef unsigned long   UINT32;
typedef unsigned long*  PULONG;
typedef void*           PVOID;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;

typedef unsigned long long  ULONGLONG;
typedef unsigned long long* PULONGLONG;


#define FAR

#ifndef NULL
#define NULL  (( void* ) 0 )
#endif

#define FALSE  0
#define TRUE   1


#define NEWLINE    "\r\n"
#define DBG_PRINT  printf

void flush ( void );


#define CPU_TO_LE32( x )  cpu_to_le32( x )
#define LE32_TO_CPU( x )  le32_to_cpu( x )


void pci_read_config_byte ( PCIDevice_t* pdev, int offset, UCHAR* pbData );
void pci_read_config_word ( PCIDevice_t* pdev, int offset, USHORT* pwData );
void pci_read_config_dword ( PCIDevice_t* pdev, int offset, ULONG* pdwData );
void pci_write_config_byte ( PCIDevice_t* pdev, int offset, UCHAR bData );
void pci_write_config_word ( PCIDevice_t* pdev, int offset, USHORT wData );
void pci_write_config_dword ( PCIDevice_t* pdev, int offset, ULONG dwData );


#ifdef KS_ISA_BUS
#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    insl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data,              \
        (( len ) + 3 ) >> 2 )

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    outsl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data,             \
        (( len ) + 3 ) >> 2 );                                              \
}

#define HW_READ_BUFFER_BYTE( phwi, addr, data, len )                        \
    insb(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_WRITE_BUFFER_BYTE( phwi, addr, data, len )                       \
    outsb(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_READ_BUFFER_WORD( phwi, addr, data, len )                        \
    insw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_WRITE_BUFFER_WORD( phwi, addr, data, len )                       \
    outsw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_READ_BUFFER_DWORD( phwi, addr, data, len )                       \
    insl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_WRITE_BUFFER_DWORD( phwi, addr, data, len )                      \
    outsl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )), data, len )

#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = inb(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    outb( data, ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = inw(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    outw(( USHORT )( data ), ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = inl(( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    outl( data, ( unsigned )(( phwi )->m_ulVIoAddr + ( addr )))
#endif

#ifdef KS_PCI_BUS
#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = readb(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    writeb( data, ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = readw(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    writew(( USHORT )( data ), ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = readl(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    writel( data, ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    pci_read_config_byte(( PCIDevice_t* )( phwi )->m_pPciCfg,               \
        addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    pci_read_config_word(( PCIDevice_t* )( phwi )->m_pPciCfg,               \
        addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    pci_read_config_dword(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    pci_write_config_byte(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    pci_write_config_word(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    pci_write_config_dword(( PCIDevice_t* )( phwi )->m_pPciCfg,             \
        addr, data )

#endif

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )


#include "util.h"
#endif


/* -------------------------------------------------------------------------- *
 *               Renesas SH7751R MS7751Rse BSP  (32-bit PCI bus\Generic bus)  *
 *                   VxWorks OS  ( Tornado 2.2.1 \vxWorks 5.5.1)              *
 * -------------------------------------------------------------------------- */

#ifdef DEF_VXWORKS
#include "vxWorks.h"                    /* define STATUS */
#include "cacheLib.h"
#include "stdio.h"
#include "string.h"

typedef unsigned char   BYTE;
typedef unsigned char * PUCHAR;
typedef unsigned char * PBYTE;
typedef unsigned short* PUSHORT;
typedef unsigned long * PULONG;
typedef unsigned long   ULONGLONG;
typedef unsigned long * PULONGLONG;
typedef int             BOOLEAN;
typedef	int		      * PBOOLEAN;
typedef void*           PVOID;


#define FAR

#define NEWLINE    "\n"
#define DBG_PRINT  printf

#define CPU_TO_LE32( x )  ( x )
#define LE32_TO_CPU( x )  ( x )


#undef  DBG
#define DBG
#undef  DEBUG_HARDWARE_SETUP
#undef  DEBUG_SPEED_SETUP



/*
 * Hardware access macros
 */

#ifdef KS_PCI_BUS

/*
 *  To access device registers through the PCI bus
 *
 *  Note: CSR is mapping to SH7751R PCI memory space by memory mapping.
 *        The driver will locate device registers as offsets from this base address.
 *
 *        When we access to registers, we need to software byte swap the data and then put it
 *        on the PCI bus, if big-endian mode is selected for SH7751R PCI controller
 *        (which is default setting).
 */

#define KS884X_ENDIAN_BIG       1          /* endian mode to access PCI CSR reg, 1:big-endian, 0:little-endian */


#define SWAP_LONG(x)        LONGSWAP(x)
#define SWAP_WORD(x)        (MSB(x) | LSB(x) << 8)


/*
 * Read\Write KS884x Registers
 */
#define HW_READ_BYTE( phwi, addr, data )                                    \
      *(PUCHAR)data = *(volatile PUCHAR)(phwi->m_ulVIoAddr + addr )

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
      *(volatile PUCHAR)(phwi->m_ulVIoAddr + addr ) = (UCHAR) data

#define HW_READ_WORD( phwi, addr, data )                                    \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
    {                                                                       \
      USHORT wDatap;                                                        \
      wDatap = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr );            \
      *(PUSHORT)data = SWAP_WORD ( wDatap );                                \
    }                                                                       \
    else                                                                    \
      *(PUSHORT)data = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr );

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
      *(volatile PUSHORT)(phwi->m_ulVIoAddr + addr ) = (USHORT)( SWAP_WORD (data) ); \
    else                                                                    \
      *(volatile PUSHORT)(phwi->m_ulVIoAddr + addr ) = data;

#define HW_READ_DWORD( phwi, addr, data )                                   \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
    {                                                                       \
      ULONG  dwDatap=0;                                                     \
      dwDatap = *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) ;             \
      *(PULONG)data = SWAP_LONG ( dwDatap );                                \
    }                                                                       \
    else                                                                    \
      *(PULONG)data = *(volatile PULONG)(phwi->m_ulVIoAddr + addr );

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
      *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) = (ULONG)( SWAP_LONG (data) ); \
    else                                                                    \
      *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) = data;

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )


/*
 * Read\Write KS884x PCI Configuration Space
 */
#define HW_PCI_READ_BYTE( phwi, addr, data )                                   \
    hw_pci_read_byte( phwi, addr, data );

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                                  \
    hw_pci_write_byte( phwi, addr, data );

#define HW_PCI_READ_WORD( phwi, addr, data )                                   \
    hw_pci_read_word( phwi, addr, data );

#define HW_PCI_WRITE_WORD( phwi, addr, data )                                  \
    hw_pci_write_word( phwi, addr, data );

#define HW_PCI_READ_DWORD( phwi, addr, data )                                   \
    hw_pci_read_dword( phwi, addr, data );

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                                  \
    hw_pci_write_dword( phwi, addr, data );

#endif  /* #ifdef KS_PCI_BUS */


#ifdef  KS_ISA_BUS

/*
 *  To access device registers through the generic bus
 *
 *  Note: Device registers are mapping to SH7751R CS4 memory space.
 *        The driver will locate device registers as offsets from this base address.
 *
 *        When we access to registers, we need to software byte swap the data,
 *        if big-endian mode is selected for SH7751R bus controller.
 */

#define KS884X_ENDIAN_BIG       1          /* endian mode to access PCI CSR reg, 1:big-endian, 0:little-endian */

#undef  SH_32BIT_ACCESS_ONLY               /* KS8841/2M can be accessed by 32bit data width only */


#define SWAP_LONG(x)        LONGSWAP(x)
#define SWAP_WORD(x)        (MSB(x) | LSB(x) << 8)



#ifndef SH_32BIT_ACCESS_ONLY

/*
 * SH7751R Area4 'Byte Control mode' is enabled,
 * SH7751R WE0/WE1/WE2/WE3 are connected to KS8841/2M BE0/BE1/BE2/BE3.
 * Area4 can access KS8841/2M by BYTE\WORD\DWORD.
 */

#define HW_READ_BYTE( phwi, addr, data )                                    \
      *(PUCHAR)data = *(volatile PUCHAR)(phwi->m_ulVIoAddr + addr );

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
      *(volatile PUCHAR)(phwi->m_ulVIoAddr + addr ) = (UCHAR) data

#define HW_READ_WORD( phwi, addr, data )                                    \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
    {                                                                       \
      USHORT wDatag;                                                        \
      wDatag = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr ) ;           \
      *(PUSHORT)data = SWAP_WORD (wDatag );                                 \
    }                                                                       \
    else                                                                    \
      *(PUSHORT)data = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr )

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
      *(volatile PUSHORT)(phwi->m_ulVIoAddr + addr ) = (USHORT)( SWAP_WORD (data) ); \
    else                                                                    \
      *(volatile PUSHORT)(phwi->m_ulVIoAddr + addr ) = data


#else /* ifndef SH_32BIT_ACCESS_ONLY */

/*
 * SH7751R Area4 'Byte Control mode' is disabled,
 * SH7751R WE0/WE1/WE2/WE3 are not connected to KS8841/2M.
 * KS8841/2M BE0/BE1/BE2/BE3 are connected to low.
 * Area4 can access KS8841/2M by DWORD only.
 * Here, we convert read/write by BYTE/WORD to by DWORD.
 */

#define HW_READ_BYTE( phwi, addr, data )                                    \
{                                                                           \
    UCHAR   shiftBit;                                                       \
    ULONG   dwDataRead;                                                     \
	                                                                        \
	switch ( addr & 0x03 )													\
	{                                                                       \
       case 0:                                                              \
            *(PUCHAR)data = *(volatile PUCHAR)(phwi->m_ulVIoAddr + addr );  \
			break;                                                          \
       default:                                                             \
            shiftBit=(addr & 0x03) << 3;                                    \
	        HW_READ_DWORD( phwi, (addr & 0x0C), &dwDataRead ) ;             \
            *data = (UCHAR)(dwDataRead >>= shiftBit);                       \
			break;                                                          \
	}                                                                       \
}

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
{                                                                           \
    UCHAR   addrByDwordAligned=addr & 0x0C;                                 \
    UCHAR   shiftBit=(addr & 0x03) << 3;                                    \
    ULONG   dwDataMask=0xFF;                                                \
    ULONG   dwDataRead;                                                     \
    ULONG   dwDataWrite=0;                                                  \
                                                                            \
    dwDataMask <<= shiftBit;                                                \
	HW_READ_DWORD( phwi, addrByDwordAligned, &dwDataRead ) ;                \
    dwDataRead &= ~dwDataMask ;                                             \
    dwDataWrite = ( ( data << shiftBit ) | dwDataRead );                    \
    HW_WRITE_DWORD( phwi, addrByDwordAligned, dwDataWrite ) ;               \
}

#define HW_READ_WORD( phwi, addr, data )                                    \
{                                                                           \
    UCHAR   shiftBit;                                                       \
    ULONG   dwDataRead;                                                     \
	                                                                        \
	switch ( addr & 0x03 )													\
	{                                                                       \
       case 0:                                                             \
            if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)             \
            {                                                               \
               USHORT wDatag;                                               \
               wDatag = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr ) ;  \
               *(PUSHORT)data = SWAP_WORD (wDatag );                        \
            }                                                               \
            else                                                            \
               *(PUSHORT)data = *(volatile PUSHORT)((phwi)->m_ulVIoAddr + addr ); \
			break;                                                          \
       case 2:                                                             \
            shiftBit=(addr & 0x03) << 3;                                    \
	        HW_READ_DWORD( phwi, (addr & 0x0C), &dwDataRead ) ;             \
            *data = (USHORT)(dwDataRead >>= shiftBit);                      \
			break;                                                          \
	}                                                                       \
}

#define HW_WRITE_WORD( phwi, addr, data )                                   \
{                                                                           \
    UCHAR   addrByDwordAligned=addr & 0x0C;                                 \
    UCHAR   shiftBit=(addr & 0x03) << 3;                                    \
    ULONG   dwDataMask=0xFFFF;                                              \
    ULONG   dwDataRead;                                                     \
    ULONG   dwDataWrite=0;                                                  \
                                                                            \
    dwDataMask <<= shiftBit;                                                \
	HW_READ_DWORD( phwi, addrByDwordAligned, &dwDataRead ) ;                \
    dwDataRead &= ~dwDataMask ;                                             \
    dwDataWrite = ( ( data << shiftBit ) | dwDataRead );                    \
    HW_WRITE_DWORD( phwi, addrByDwordAligned, dwDataWrite ) ;               \
}

#endif /* ifndef SH_32BIT_ACCESS_ONLY */


#define HW_READ_DWORD( phwi, addr, data )                                   \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                     \
    {                                                                       \
      ULONG  dwDatag;                                                       \
      dwDatag = *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) ;             \
      *(PULONG)data = SWAP_LONG ( dwDatag );                                \
    }                                                                       \
    else                                                                    \
      *(PULONG)data = *(volatile PULONG)(phwi->m_ulVIoAddr + addr )

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    if (phwi->m_boardBusEndianMode & KS884X_ENDIAN_BIG)                             \
      *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) = (ULONG)( SWAP_LONG (data) ); \
    else                                                                     \
      *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) = data


#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )

#define HW_READ_BUFFER( phwi, addr, data, len )                             \
{                                                                           \
    int lengthInDWord = ((len + 3) >> 2) ;                                  \
    PULONG    pdwData = (PULONG)data;                                       \
                                                                            \
    HardwareSelectBank( phwi, REG_DATA_BANK );                              \
    while ( lengthInDWord--)                                                \
    *(PULONG)pdwData++ = *(volatile PULONG)(phwi->m_ulVIoAddr + addr );     \
}

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
{                                                                           \
    int lengthInDWord = ((len + 3) >> 2) ;                                  \
    PULONG    pdwData = (PULONG)data;                                       \
                                                                            \
    HardwareSelectBank( phwi, REG_DATA_BANK );                              \
    while ( lengthInDWord-- )                                               \
      *(volatile PULONG)(phwi->m_ulVIoAddr + addr ) = *pdwData++;           \
}

#endif /* #ifdef  KS_ISA_BUS */

#endif  /* #ifdef DEF_VXWORKS */


#ifndef ASSERT
#ifdef DBG
#define ASSERT( f )  \
    if ( !( f ) ) {  \
        DBG_PRINT( "assert %s at %d in %s\n", #f, __LINE__, __FILE__ );  \
    }

#else
#define ASSERT( f )
#endif
#endif


/* -------------------------------------------------------------------------- *
 *               Renesas M16C/62P      (16-bit Generic bus)                   *
 *                      MTOOL/OpenTCP  (version 1.04)                         *
 * -------------------------------------------------------------------------- */

#ifdef M16C_62P
#include <inet/datatypes.h>
#include <inet/system.h>
#include <string.h>
#include "stdio.h"
#include "skp_bsp.h"	      /* include SKP board support package */



typedef unsigned char    UCHAR;
typedef unsigned char  * PUCHAR;
typedef unsigned char  * PBYTE;
typedef unsigned short   USHORT;
typedef unsigned short * PUSHORT;
typedef unsigned long    ULONG;
typedef unsigned long  * PULONG;
typedef unsigned long    ULONGLONG;
typedef unsigned long  * PULONGLONG;
typedef int              BOOLEAN;
typedef	int		       * PBOOLEAN;

#define FAR _far

#ifndef NULL
#define NULL  0
#endif

#define NEWLINE    "\r\n"
#define DBG_PRINT  uart_print

#undef  DBG
#undef  DEBUG_TX
#undef  DEBUG_RX

/* to test Early Transmit\Receive */
#undef EARLY_RECEIVE
#undef EARLY_TRANSMIT

/*
 * Hardware access macros
 */

#ifdef KS_ISA_BUS

#define HW_READ_BYTE( phwi, addr, data )                                   \
    *data = *((volatile FAR UINT8 *)(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                  \
    *((volatile FAR UINT8 *)(( phwi )->m_ulVIoAddr + addr )) = ( UINT8 )( data )

#ifdef KS_ISA_8BIT_BUS

/* for 8-bit data bus */
#define HW_READ_WORD( phwi, addr, data )                                   \
{                                                                          \
    UCHAR   bData, bDumy;                                                  \
    USHORT  wData, wDumy;                                                         \
    HW_READ_BYTE( phwi, (UCHAR)(addr), (UCHAR *)&bData );                  \
    HW_READ_BYTE( phwi, (UCHAR)(addr+1), (UCHAR *)&wData );                \
    wData <<= 8;                                                           \
    wData |= (USHORT)bData;                                                 \
    *(USHORT *)data = wData;                                               \
}

#define HW_WRITE_WORD( phwi, addr, data )                                  \
{                                                                          \
    UCHAR   bData;                                                         \
    bData = (UCHAR)(data & 0x00ff);                                        \
    HW_WRITE_BYTE( phwi, (UCHAR)(addr), (UCHAR)bData );                    \
    bData = (UCHAR)( (data & 0xff00) >> 8 );                               \
    HW_WRITE_BYTE( phwi, (UCHAR)(addr+1), (UCHAR)bData );                  \
}

#define HW_READ_DWORD( phwi, addr, data )                                   \
    hw_read_dword( phwi, addr, data );

/*
#define HW_READ_DWORD( phwi, addr, data )                                   \
{                                                                           \
    USHORT  wData, wDumy;                                                   \
    ULONG   dwData;                                                 \
    HW_READ_WORD( phwi, (UCHAR)(addr), (USHORT *)&wData );                  \
    wDumy = wData;                                                          \
    HW_READ_WORD( phwi, (UCHAR)(addr+2), (USHORT *)&dwData );               \
    dwData <<= 16;                                                          \
    dwData |= (ULONG)wData;                                                 \
    *(ULONG *)data = dwData;                                                \
}
*/

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    hw_write_dword( phwi, addr, data );

/*
#define HW_WRITE_DWORD( phwi, addr, data )                                  \
{                                                                           \
    USHORT  wData;                                                          \
    wData = (USHORT)(data & 0x0000ffff);                                    \
    HW_WRITE_WORD( phwi, (UCHAR)addr, (USHORT)wData );                      \
    wData = (USHORT)( (data & 0xffff0000) >> 16 );                          \
    HW_WRITE_WORD( phwi, (UCHAR)(addr+2), (USHORT)wData );                  \
}
*/


#else

/* for 16-bit data bus */
#define HW_READ_WORD( phwi, addr, data )                                   \
    *data = *((volatile FAR UINT16 * )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                  \
    *((volatile FAR UINT16 * )(( phwi )->m_ulVIoAddr + addr )) = ( UINT16 )( data )

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = *((volatile FAR UINT32 * )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    *((volatile FAR UINT32 * )(( phwi )->m_ulVIoAddr + addr )) = ( UINT32 )( data )

#endif /* #ifdef KS_ISA_8BIT_BUS */

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )


#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    HardwareReadBuffer( phwi, addr, ( PULONG ) data, ( len )  );

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
    HardwareWriteBuffer( phwi, addr, ( PULONG ) data, ( len )  );


#endif  /* #ifdef KS_ISA_BUS */

#endif /* #ifdef M16C_62P */



/* -------------------------------------------------------------------------- *
 *     ZiLog eZ80L92 (eZ80 Webserver-i E-NET Module)  (8-bit Generic bus)     *
 *                   ZTP 1.3.2 / EMAC DDK 1.3.2                               *
 * -------------------------------------------------------------------------- */

#ifdef _EZ80L92
#include <kernel.h>
#include <network.h>


typedef unsigned char    UCHAR;
typedef unsigned char  * PUCHAR;
typedef unsigned char  * PBYTE;
typedef unsigned short   USHORT;
typedef unsigned short * PUSHORT;
typedef unsigned long    ULONG;
typedef unsigned long  * PULONG;
typedef unsigned long    ULONGLONG;
typedef unsigned long  * PULONGLONG;
typedef int              BOOLEAN;
typedef	int		       * PBOOLEAN;

typedef volatile DWORD __EXTIO *	IORegExt32;


#define FAR

#ifndef NULL
#define NULL  0
#endif

#define NEWLINE    "\n"
#define DBG_PRINT  kprintf

#undef  DBG
#undef  DEBUG_TX
#undef  DEBUG_RX

#define  DEBUG_DUMP_TX
#define  DEBUG_DUMP_RX


/* ERROR CODES */
#define KS884X_NOTFOUND     -1
#define KS_MAC_NOT_FOUND    -2
#define ILLEGAL_CALLBACKS   -3
#define CALLOC_HW_MEM_FAIL  -4
#define KS_TX_NOT_READY     -5
#define KS_RX_ERROR         -6

/*
 * Hardware access macros
 */

#ifdef KS_ISA_BUS

#define HW_READ_BYTE( phwi, addr, data )                                   \
    *data = *((PUCHAR)( (phwi) ->m_ulVIoAddr + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                  \
    *((PUCHAR)(( phwi )->m_ulVIoAddr + addr )) = ( UINT8 )( data )

#define HW_READ_WORD( phwi, addr, data )                                   \
    *data = *((PUSHORT)( (phwi) ->m_ulVIoAddr + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                  \
    *((PUSHORT)(( phwi )->m_ulVIoAddr + addr )) = ( UINT16 )( data )


#define HW_READ_DWORD( phwi, addr, data )                                  \
    *data = *((PULONG)(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                 \
    *((PULONG)(( phwi )->m_ulVIoAddr + addr )) = ( UINT32 )( data )

#define MOVE_MEM( dest, src, len )                                         \
    memcpy( dest, src, len )

#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    HardwareReadBuffer( phwi, addr, ( PULONG ) data, ( len )  );

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
    HardwareWriteBuffer( phwi, addr, ( PULONG ) data, ( len )  );

#endif  /* #ifdef KS_ISA_BUS */

char *strtok_p
(
    char *s1,
    const char *s2,
    char **ppLast
);


#endif /* #ifdef _EZ80L92 */

void DelayMicrosec (
    ULONG microsec );

void DelayMillisec (
    ULONG millisec );

void PrintMacAddress (
    PUCHAR bAddr );

void PrintIpAddress (
    ULONG IpAddr );

void PrintPacketData (
    UCHAR         *data,
    int            len,
    ULONG         port,
    ULONG         flag );


#define  PF()   printk("%s:%d\n",__func__,__LINE__)

#endif
