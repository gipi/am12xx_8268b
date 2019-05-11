/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_table.c - KS884X switch table functions.

    Author  Date      Version  Description
    THa     02/23/06           Removed KS884X_HW conditional.
    PCD     05/25/05  0.1.4    Modify the way to read\write table registers in KS_ISA_BUS interface.
    THa     09/29/04           Updated for PCI version.
    THa     02/13/04           Created file.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"

//#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);
#define ACT_DEBUG_

/* -------------------------------------------------------------------------- */


#define TABLE_READ       0x10
#define TABLE_SEL_SHIFT  2

/* -------------------------------------------------------------------------- */

/*
    SwitchReadTable

    Description:
        This routine reads 4 bytes of data from the table of the switch.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int nTable
            The table selector.

        USHORT wAddr
            The address of the table entry.

        PULONG pdwData
            Buffer to store the read data.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchReadTable_PCI
#else
void SwitchReadTable_ISA
#endif
(
    PHARDWARE pHardware,
    int       nTable,
    USHORT    wAddr,
    PULONG    pdwData )
{
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( nTable << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ) |
        TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          (USHORT)wCtrlAddr
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          (PULONG)pdwData
                        );

#else
    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, pdwData );
#endif

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* SwitchReadTable */


/*
    SwitchWriteTable

    Description:
        This routine writes 4 bytes of data to the table of the switch.
        Hardware interrupts are disabled to minimize corruption of written data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int nTable
            The table selector.

        USHORT wAddr
            The address of the table entry.

        ULONG dwData
            Data to be written.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchWriteTable_PCI
#else
void SwitchWriteTable_ISA
#endif
(
    PHARDWARE pHardware,
    int       nTable,
    USHORT    wAddr,
    ULONG     dwData )
{
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( nTable << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ));

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          dwData
                        );
    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );
#else
    HW_WRITE_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, dwData );
    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
#endif

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* SwitchWriteTable */


/*
    SwitchReadTableQword

    Description:
        This routine reads 8 bytes of data from the table of the switch.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int nTable
            The table selector.

        USHORT wAddr
            The address of the table entry.

        PULONG pdwDataHi
            Buffer to store the high part of read data (bit63 ~ bit32).

        PULONG pdwDataLo
            Buffer to store the low part of read data (bit31 ~ bit0).

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchReadTableQword_PCI
#else
void SwitchReadTableQword_ISA
#endif
(
    PHARDWARE pHardware,
    int       nTable,
    USHORT    wAddr,
    PULONG    pdwDataHi,
    PULONG    pdwDataLo )
{
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( nTable << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ) |
        TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                         (UCHAR)REG_IND_ACC_CTRL_BANK,
                         (UCHAR)REG_IACR_OFFSET,
                         (USHORT)wCtrlAddr
                        );

    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_4_OFFSET,
                          pdwDataHi
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          pdwDataLo
                        );
#else

    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );

    HW_READ_DWORD( pHardware, REG_ACC_DATA_4_OFFSET, pdwDataHi );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, pdwDataLo );

#endif

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* SwitchReadTableQword */


/*
    SwitchWriteTableQword

    Description:
        This routine writes 8 bytes of data to the table of the switch.
        Hardware interrupts are disabled to minimize corruption of written data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int nTable
            The table selector.

        USHORT wAddr
            The address of the table entry.

        ULONG dwDataHi
            The high part of data to be written (bit63 ~ bit32).

        ULONG dwDataLo
            The low part of data to be written (bit31 ~ bit0).

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchWriteTableQword_PCI
#else
void SwitchWriteTableQword_ISA
#endif
(
    PHARDWARE pHardware,
    int       nTable,
    USHORT    wAddr,
    ULONG     dwDataHi,
    ULONG     dwDataLo )
{
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( nTable << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ));

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_4_OFFSET,
                          dwDataHi
                        );
    HardwareWriteRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          dwDataLo
                        );

    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );
#else
    HW_WRITE_DWORD( pHardware, REG_ACC_DATA_4_OFFSET, dwDataHi );
    HW_WRITE_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, dwDataLo );

    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
#endif

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* SwitchWriteTableQword */


/*
    SwitchReadTableAllword

    Description:
        This routine reads 10 bytes of data from the table of the switch.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int nTable
            The table selector.

        USHORT wAddr
            The address of the table entry.

        USHORT pwDataHi
            Buffer to store the highest part of read data (bit79 ~ bit64).

        PULONG pdwDataHi
            Buffer to store the high part of read data (bit63 ~ bit32).

        PULONG pdwDataLo
            Buffer to store the low part of read data (bit31 ~ bit0).

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchReadTableAllword_PCI
#else
void SwitchReadTableAllword_ISA
#endif
(
    PHARDWARE pHardware,
    int       nTable,
    USHORT    wAddr,
    PUSHORT   pwDataHi,
    PULONG    pdwDataHi,
    PULONG    pdwDataLo )
{
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( nTable << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ) |
                      TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );

    HardwareReadRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_8_OFFSET,
                          pwDataHi
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_4_OFFSET,
                          pdwDataHi
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          pdwDataLo
                        );
#else

    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );

    HW_READ_WORD ( pHardware, REG_ACC_DATA_8_OFFSET, pwDataHi );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_4_OFFSET, pdwDataHi );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, pdwDataLo );
#endif

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );

}  /* SwitchReadTableAllword */



/* -------------------------------------------------------------------------- */

/*
#define STATIC_MAC_TABLE_ADDR        00-0000FFFF-FFFFFFFF
#define STATIC_MAC_TABLE_FWD_PORTS   00-00070000-00000000
#define STATIC_MAC_TABLE_VALID       00-00080000-00000000
#define STATIC_MAC_TABLE_OVERRIDE    00-00100000-00000000
#define STATIC_MAC_TABLE_USE_FID     00-00200000-00000000
#define STATIC_MAC_TABLE_FID         00-03C00000-00000000
*/

#define STATIC_MAC_TABLE_ADDR        0x0000FFFF
#define STATIC_MAC_TABLE_FWD_PORTS   0x00070000
#define STATIC_MAC_TABLE_VALID       0x00080000
#define STATIC_MAC_TABLE_OVERRIDE    0x00100000
#define STATIC_MAC_TABLE_USE_FID     0x00200000
#define STATIC_MAC_TABLE_FID         0x03C00000

#define STATIC_MAC_FWD_PORTS_SHIFT   16
#define STATIC_MAC_FID_SHIFT         22

/*
#define VLAN_TABLE_VID               00-00000000-00000FFF
#define VLAN_TABLE_FID               00-00000000-0000F000
#define VLAN_TABLE_MEMBERSHIP        00-00000000-00070000
#define VLAN_TABLE_VALID             00-00000000-00080000
*/

#define VLAN_TABLE_VID               0x00000FFF
#define VLAN_TABLE_FID               0x0000F000
#define VLAN_TABLE_MEMBERSHIP        0x00070000
#define VLAN_TABLE_VALID             0x00080000

#define VLAN_TABLE_FID_SHIFT         12
#define VLAN_TABLE_MEMBERSHIP_SHIFT  16

/*
#define DYNAMIC_MAC_TABLE_ADDR        00-0000FFFF-FFFFFFFF
#define DYNAMIC_MAC_TABLE_FID         00-000F0000-00000000
#define DYNAMIC_MAC_TABLE_SRC_PORT    00-00300000-00000000
#define DYNAMIC_MAC_TABLE_TIMESTAMP   00-00C00000-00000000
#define DYNAMIC_MAC_TABLE_ENTRIES     03-FF000000-00000000
#define DYNAMIC_MAC_TABLE_MAC_EMPTY   04-00000000-00000000
#define DYNAMIC_MAC_TABLE_RESERVED    78-00000000-00000000
#define DYNAMIC_MAC_TABLE_NOT_READY   80-00000000-00000000
*/

#define DYNAMIC_MAC_TABLE_ADDR       0x0000FFFF
#define DYNAMIC_MAC_TABLE_FID        0x000F0000
#define DYNAMIC_MAC_TABLE_SRC_PORT   0x00300000
#define DYNAMIC_MAC_TABLE_TIMESTAMP  0x00C00000
#define DYNAMIC_MAC_TABLE_ENTRIES    0xFF000000

#define DYNAMIC_MAC_TABLE_ENTRIES_H    0x03
#define DYNAMIC_MAC_TABLE_MAC_EMPTY    0x04
#define DYNAMIC_MAC_TABLE_RESERVED     0x78
#define DYNAMIC_MAC_TABLE_NOT_READY    0x80

#define DYNAMIC_MAC_FID_SHIFT        16
#define DYNAMIC_MAC_SRC_PORT_SHIFT   20
#define DYNAMIC_MAC_TIMESTAMP_SHIFT  22
#define DYNAMIC_MAC_ENTRIES_SHIFT    24
#define DYNAMIC_MAC_ENTRIES_H_SHIFT  16

/*
#define MIB_COUNTER_VALUE            00-00000000-3FFFFFFF
#define MIB_COUNTER_VALID            00-00000000-40000000
#define MIB_COUNTER_OVERFLOW         00-00000000-80000000
*/

#define MIB_COUNTER_VALUE            0x3FFFFFFF
#define MIB_COUNTER_VALID            0x40000000
#define MIB_COUNTER_VALID_SWAPPED    0x00000040
#define MIB_COUNTER_OVERFLOW         0x80000000

#define MIB_COUNTER_PACKET_DROPPED   0x0000FFFF

/* -------------------------------------------------------------------------- */

/*
    SwitchReadDynMacTable

    Description:
        This function reads an entry of the dynamic MAC table of the switch.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the entry.

        PUCHAR MacAddr
            Buffer to store the MAC address.

        PUCHAR pbFID
            Buffer to store the FID.

        PUCHAR pbSrcPort
            Buffer to store the source port number.

        PUCHAR pbTimestamp
            Buffer to store the timestamp.

        PUSHORT pwEntries
            Buffer to store the number of entries.  If this is zero, the table
            is empty and so this function should not be called again until
            later.

    Return (BOOLEAN):
        TRUE if the entry is successfully read; otherwise FALSE.
*/

#ifdef KS_PCI_BUS
BOOLEAN SwitchReadDynMacTable_PCI
#else
BOOLEAN SwitchReadDynMacTable_ISA
#endif
(
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    PUCHAR    pbFID,
    PUCHAR    pbSrcPort,
    PUCHAR    pbTimestamp,
    PUSHORT   pwEntries
)
{
    ULONG   dwDataHi = 0;
    ULONG   dwDataLo;
    UCHAR   bAddr;
    UCHAR   bCtrl;
    UCHAR   bData = 0;
    USHORT  wCtrlAddr;
    BOOLEAN rc = FALSE;

#ifdef KS_PCI_BUS
    ULONG   InterruptMask;

#else
    USHORT   InterruptMask;
#endif

    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( TABLE_DYNAMIC_MAC << TABLE_SEL_SHIFT ) | ( wAddr >> 8 )
                        | TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );

    HardwareReadRegByte( pHardware,
                         (UCHAR)REG_IND_ACC_CTRL_BANK,
                         (UCHAR)REG_ACC_DATA_8_OFFSET,
                         &bData
                       );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_4_OFFSET,
                          &dwDataHi
                        );
#else

    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
    HW_READ_BYTE( pHardware, REG_ACC_DATA_8_OFFSET, &bData );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_4_OFFSET, &dwDataHi );
#endif

    /* Entry is not ready for accessing */
    if ( ( bData & DYNAMIC_MAC_TABLE_NOT_READY ) )
    {
        *pwEntries = 0;
    }
    /* Entry is ready for accessing */
    else
    {
        /* There is no valid entry in the table */
        if ( ( bData & DYNAMIC_MAC_TABLE_MAC_EMPTY ) )
        {
            *pwEntries = 0;
        }
        /* At least one valid entry in the table */
        else
        {
            /* check out how many valid entry in the table */
            *pwEntries = ( USHORT )(
                         (((( ULONG ) bData & DYNAMIC_MAC_TABLE_ENTRIES_H ) <<
                              DYNAMIC_MAC_ENTRIES_H_SHIFT ) |
                          ((( dwDataHi & DYNAMIC_MAC_TABLE_ENTRIES ) >>
                              DYNAMIC_MAC_ENTRIES_SHIFT ))) + 1 );

            /* get the detail of dynamaic MAC table information */
            HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, &dwDataLo );
            MacAddr[ 5 ] = ( UCHAR ) dwDataLo;
            MacAddr[ 4 ] = ( UCHAR )( dwDataLo >> 8 );
            MacAddr[ 3 ] = ( UCHAR )( dwDataLo >> 16 );
            MacAddr[ 2 ] = ( UCHAR )( dwDataLo >> 24 );
            MacAddr[ 1 ] = ( UCHAR ) dwDataHi;
            MacAddr[ 0 ] = ( UCHAR )( dwDataHi >> 8 );
            *pbFID = ( UCHAR )(( dwDataHi & DYNAMIC_MAC_TABLE_FID ) >>
                                 DYNAMIC_MAC_FID_SHIFT );
            *pbSrcPort = ( UCHAR )(( dwDataHi & DYNAMIC_MAC_TABLE_SRC_PORT ) >>
                                     DYNAMIC_MAC_SRC_PORT_SHIFT );
            *pbTimestamp = ( UCHAR )(( dwDataHi & DYNAMIC_MAC_TABLE_TIMESTAMP ) >>
                                     DYNAMIC_MAC_TIMESTAMP_SHIFT );
            rc = TRUE;

        } /* if ( ( dwDataHi & DYNAMIC_MAC_TABLE_MAC_EMPTY ) ) */

    } /* if ( ( bData & DYNAMIC_MAC_TABLE_NOT_READY ) ) */

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );

    return( rc );
}  /* SwitchReadDynMacTable */

/* -------------------------------------------------------------------------- */

#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchReadStaticMacTable

    Description:
        This function reads an entry of the static MAC table of the switch.  It
        calls SwitchReadTableQword() to get the data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the entry.

        PUCHAR MacAddr
            Buffer to store the MAC address.

        PUCHAR pbPorts
            Buffer to store the port members.

        PBOOLEAN pfOverride
            Buffer to store the override flag.

        PBOOLEAN pfUserFID
            Buffer to store the use FID flag which indicates the FID is valid.

        PUCHAR pbFID
            Buffer to store the FID.

    Return (BOOLEAN):
        TRUE if the entry is valid; otherwise FALSE.
*/

BOOLEAN SwitchReadStaticMacTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    PUCHAR    pbPorts,
    PBOOLEAN  pfOverride,
    PBOOLEAN  pfUseFID,
    PUCHAR    pbFID )
{
    ULONG dwDataHi;
    ULONG dwDataLo;

    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        /* DBG_PRINT ("SwitchReadStaticMacTable(ISA)\n"); */

        SwitchReadTableQword_ISA( pHardware, TABLE_STATIC_MAC, wAddr,
            &dwDataHi, &dwDataLo );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        /*DBG_PRINT ("SwitchReadStaticMacTable(PCI)\n");*/

        SwitchReadTableQword_PCI( pHardware, TABLE_STATIC_MAC, wAddr,
            &dwDataHi, &dwDataLo );
    }
#endif
    if ( ( dwDataHi & STATIC_MAC_TABLE_VALID ) )
    {

#ifdef KS_ISA_BUS
#ifdef KS_PCI
        if ( !pHardware->m_fPCI )
#endif
        {
            MacAddr[ 5 ] = ( UCHAR ) dwDataLo;
            MacAddr[ 4 ] = ( UCHAR )( dwDataLo >> 8 );
            MacAddr[ 3 ] = ( UCHAR )( dwDataLo >> 16 );
            MacAddr[ 2 ] = ( UCHAR )( dwDataLo >> 24 );
        }

#ifdef KS_PCI
        else
#endif
#endif

#ifdef KS_PCI
        {
            MacAddr[ 5 ] = ( UCHAR ) dwDataLo;
            MacAddr[ 4 ] = ( UCHAR )( dwDataLo >> 8 );
            MacAddr[ 3 ] = ( UCHAR )( dwDataLo >> 16 );
            MacAddr[ 2 ] = ( UCHAR )( dwDataLo >> 24 );
        }
#endif
        MacAddr[ 1 ] = ( UCHAR ) dwDataHi;
        MacAddr[ 0 ] = ( UCHAR )( dwDataHi >> 8 );
        *pbPorts = ( UCHAR )(( dwDataHi & STATIC_MAC_TABLE_FWD_PORTS ) >>
            STATIC_MAC_FWD_PORTS_SHIFT );
        *pfOverride = ( dwDataHi & STATIC_MAC_TABLE_OVERRIDE ) ? TRUE : FALSE;
        *pfUseFID = ( dwDataHi & STATIC_MAC_TABLE_USE_FID ) ? TRUE : FALSE;
        *pbFID = ( UCHAR )(( dwDataHi & STATIC_MAC_TABLE_FID ) >>
            STATIC_MAC_FID_SHIFT );
        return TRUE;
    }
    return FALSE;
}  /* SwitchReadStaticMacTable */


/*
    SwitchWriteStaticMacTable

    Description:
        This routine writes an entry of the static MAC table of the switch.  It
        calls SwitchWriteTableQword() to write the data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the entry.

        PUCHAR MacAddr
            The MAC address.

        UCHAR bPorts
            The port members.

        BOOLEAN fOverride
            The override flag to override the port receive/transmit settings.

        BOOLEAN fValid
            The valid flag to indicate entry is valid.

        BOOLEAN fUserFID
            The use FID flag to indicate the FID is valid.

        UCHAR bFID
            The FID value.

    Return (None):
*/

void SwitchWriteStaticMacTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    UCHAR     bPorts,
    BOOLEAN   fOverride,
    BOOLEAN   fValid,
    BOOLEAN   fUseFID,
    UCHAR     bFID )
{
    ULONG dwDataHi;
    ULONG dwDataLo;

    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        dwDataLo = *(( PULONG ) &MacAddr[ 2 ]);
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        dwDataLo = (( ULONG ) MacAddr[ 2 ] << 24 ) |
            (( ULONG ) MacAddr[ 3 ] << 16 ) |
            (( ULONG ) MacAddr[ 4 ] << 8 ) | MacAddr[ 5 ];
    }
#endif
    dwDataHi = (( ULONG ) MacAddr[ 0 ] << 8 ) | MacAddr[ 1 ];
    dwDataHi |= ( ULONG ) bPorts << STATIC_MAC_FWD_PORTS_SHIFT;
    if ( fOverride )
    {
        dwDataHi |= STATIC_MAC_TABLE_OVERRIDE;
    }
    if ( fUseFID )
    {
        dwDataHi |= STATIC_MAC_TABLE_USE_FID;
        dwDataHi |= ( ULONG ) bFID << STATIC_MAC_FID_SHIFT;
    }
    if ( fValid )
    {
        dwDataHi |= STATIC_MAC_TABLE_VALID;
    }

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        SwitchWriteTableQword_ISA( pHardware, TABLE_STATIC_MAC, wAddr,
            dwDataHi, dwDataLo );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        SwitchWriteTableQword_PCI( pHardware, TABLE_STATIC_MAC, wAddr,
            dwDataHi, dwDataLo );
    }
#endif
}  /* SwitchWriteStaticMacTable */

/* -------------------------------------------------------------------------- */

/*
    SwitchReadVlanTable

    Description:
        This function reads an entry of the VLAN table of the switch.  It calls
        SwitchReadTable() to get the data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the entry.

        PUSHORT pwVID
            Buffer to store the VID.

        PUCHAR pbFID
            Buffer to store the VID.

        PUCHAR pbMember
            Buffer to store the port membership.

    Return (BOOLEAN):
        TRUE if the entry is valid; otherwise FALSE.
*/

BOOLEAN SwitchReadVlanTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUSHORT   pwVID,
    PUCHAR    pbFID,
    PUCHAR    pbMember )
{
    ULONG dwData;

    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        SwitchReadTable_ISA( pHardware, TABLE_VLAN, wAddr, &dwData );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        SwitchReadTable_PCI( pHardware, TABLE_VLAN, wAddr, &dwData );
    }
#endif
    if ( ( dwData & VLAN_TABLE_VALID ) )
    {
        *pwVID = ( USHORT )( dwData & VLAN_TABLE_VID );
        *pbFID = ( UCHAR )(( dwData & VLAN_TABLE_FID ) >>
            VLAN_TABLE_FID_SHIFT );
        *pbMember = ( UCHAR )(( dwData & VLAN_TABLE_MEMBERSHIP ) >>
            VLAN_TABLE_MEMBERSHIP_SHIFT );
        return TRUE;
    }
    return FALSE;
}  /* SwitchReadVlanTable */


/*
    SwitchWriteVlanTable

    Description:
        This routine writes an entry of the VLAN table of the switch.  It calls
        SwitchWriteTable() to write the data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the entry.

        USHORT wVID
            The VID value.

        UCHAR bFID
            The FID value.

        UCHAR bMember
            The port membership.

        BOOLEAN fValid
            The valid flag to indicate entry is valid.

    Return (None):
*/

void SwitchWriteVlanTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    USHORT    wVID,
    UCHAR     bFID,
    UCHAR     bMember,
    BOOLEAN   fValid )
{
    ULONG dwData;

    ASSERT( pHardware->m_bAcquire );

    dwData = wVID;
    dwData |= ( ULONG ) bFID << VLAN_TABLE_FID_SHIFT;
    dwData |= ( ULONG ) bMember << VLAN_TABLE_MEMBERSHIP_SHIFT;
    if ( fValid )
        dwData |= VLAN_TABLE_VALID;

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        dwData = SwapBytes( dwData );
        SwitchWriteTable_ISA( pHardware, TABLE_VLAN, wAddr, dwData );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        SwitchWriteTable_PCI( pHardware, TABLE_VLAN, wAddr, dwData );
    }
#endif
}  /* SwitchWriteVlanTable */
#endif

/* -------------------------------------------------------------------------- */

#define MIB_COUNTER_PACKET_DROPPED_TX_0  0x100
#define MIB_COUNTER_PACKET_DROPPED_TX_1  0x101
#define MIB_COUNTER_PACKET_DROPPED_TX_2  0x102
#define MIB_COUNTER_PACKET_DROPPED_RX_0  0x103
#define MIB_COUNTER_PACKET_DROPPED_RX_1  0x104
#define MIB_COUNTER_PACKET_DROPPED_RX_2  0x105


/*
    PortReadMIBCounter

    Description:
        This routine reads a MIB counter of the port.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        USHORT wAddr
            The address of the counter.

        PULONGLONG pqData
            Buffer to store the counter.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortReadMIBCounter_PCI
#else
void PortReadMIBCounter_ISA
#endif
(
    PHARDWARE  pHardware,
    UCHAR      bPort,
    USHORT     wAddr,
    PULONGLONG pqData )
{
    ULONG      dwData;
    UCHAR      bAddr;
    UCHAR      bCtrl;
    USHORT     wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG InterruptMask;

#else
    USHORT InterruptMask;
#endif
    int   cnTimeOut;

    wAddr += PORT_COUNTER_NUM * bPort;
    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( TABLE_MIB << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ) |
        TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
//ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                                  (UCHAR)REG_IND_ACC_CTRL_BANK,
                                  (UCHAR)REG_IACR_OFFSET,
                                  wCtrlAddr
                                );
#else
    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
#endif

    for ( cnTimeOut = 100; cnTimeOut > 0; cnTimeOut-- )
    {

#ifdef KS_ISA_BUS
        HardwareSelectBank( pHardware, REG_IND_ACC_CTRL_BANK );
        HardwareReadRegDWord( pHardware,
                                 (UCHAR)REG_IND_ACC_CTRL_BANK,
                                 (UCHAR)REG_ACC_DATA_0_OFFSET,
                                 (PULONG)&dwData
                               );
#else
        HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, &dwData );
#endif

        if ( ( dwData & MIB_COUNTER_VALID ) )
        {
            if ( ( dwData & MIB_COUNTER_OVERFLOW ) )
            {
                *pqData += MIB_COUNTER_VALUE + 1;
            }
            *pqData += dwData & MIB_COUNTER_VALUE;
            break;
        }
    }

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* PortReadMIBCounter */


/*
    PortReadMIBPacket

    Description:
        This routine reads the dropped packet counts of the port.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        PULONGLONG pqData
            Buffer to store the receive and transmit dropped packet counts.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortReadMIBPacket_PCI
#else
void PortReadMIBPacket_ISA
#endif
(
    PHARDWARE  pHardware,
    UCHAR      bPort,
    PULONG     pdwCurrent,
    PULONGLONG pqData )
{
    ULONG     dwCurrent;
    ULONG     dwData = 0;
    USHORT    wAddr;
    UCHAR     bAddr;
    UCHAR     bCtrl;
    USHORT    wCtrlAddr;

#ifdef KS_PCI_BUS
    ULONG     InterruptMask;

#else
    USHORT    InterruptMask;
#endif


    wAddr = MIB_COUNTER_PACKET_DROPPED_RX_0 + bPort;
    bAddr = ( UCHAR ) wAddr;
    bCtrl = ( UCHAR )(( TABLE_MIB << TABLE_SEL_SHIFT ) | ( wAddr >> 8 ) |
        TABLE_READ );

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

    /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
    InterruptMask = HardwareBlockInterrupt( pHardware );

#ifdef KS_ISA_BUS
    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          &dwData
                        );
#else
    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, &dwData );

#endif

    dwData &= MIB_COUNTER_PACKET_DROPPED;
    dwCurrent = *pdwCurrent;
    if ( dwData != dwCurrent )
    {
        if ( dwData < dwCurrent )
        {
            dwData += MIB_COUNTER_PACKET_DROPPED + 1;
        }
        dwData -= dwCurrent;
        *pqData += dwData;
        *pdwCurrent = dwCurrent;
    }
    ++pdwCurrent;
    ++pqData;
    wAddr = MIB_COUNTER_PACKET_DROPPED_TX_0 + bPort;
    bAddr = ( UCHAR ) wAddr;

    wCtrlAddr = bCtrl;
    wCtrlAddr = (wCtrlAddr << 8) | bAddr ;

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_IND_ACC_CTRL_BANK );

    HardwareWriteRegWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_IACR_OFFSET,
                          wCtrlAddr
                        );
    HardwareReadRegDWord( pHardware,
                          (UCHAR)REG_IND_ACC_CTRL_BANK,
                          (UCHAR)REG_ACC_DATA_0_OFFSET,
                          &dwData
                        );
#else
    HW_WRITE_WORD( pHardware, REG_IACR_OFFSET, wCtrlAddr );
    HW_READ_DWORD( pHardware, REG_ACC_DATA_0_OFFSET, &dwData );
#endif

    dwData &= MIB_COUNTER_PACKET_DROPPED;
    dwCurrent = ( ULONG )( *pqData & MIB_COUNTER_PACKET_DROPPED );
    if ( dwData != dwCurrent )
    {
        if ( dwData < dwCurrent )
        {
            dwData += MIB_COUNTER_PACKET_DROPPED + 1;
        }
        dwData -= dwCurrent;
        *pqData += dwData;
    }

    /* Restore the interrupt mask. */
    HardwareSetInterrupt( pHardware, InterruptMask );
}  /* PortReadMIBPacket */


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )

/*
    PortReadCounters

    Description:
        This routine is used to read the counters of the port periodically to
        avoid counter overflow.  The hardware should be acquired first before
        calling this routine.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (int):
        non-zero when not all counters not read.
*/

int PortReadCounters (
    PHARDWARE pHardware,
    UCHAR     bPort )
{
    PPORT_CONFIG pPort = &pHardware->m_Port[ bPort ];

#ifdef KS_PCI_BUS
    ULONG        IntEnable;
    ULONG        IntMask = pHardware->m_ulInterruptMask;

#else
    USHORT       IntEnable;
    USHORT       IntMask = pHardware->m_wInterruptMask;
#endif

    ASSERT( pHardware->m_bAcquire );

ACT_DEBUG_
    do
    {
        PortReadMIBCounter( pHardware, bPort, pPort->bCurrentCounter,
            &pPort->cnCounter[ pPort->bCurrentCounter ]);
        ++pPort->bCurrentCounter;

        /* Do table read in stages to allow interrupt processing. */
        HardwareReadInterrupt( pHardware, &IntEnable );
        if ( (( IntMask & IntEnable ) & INT_RX ) )
            return( pPort->bCurrentCounter );

#if (0)
        switch ( pPort->bCurrentCounter )
        {
            case MIB_COUNTER_RX_UNDERSIZE:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_RX_SYMBOL_ERR:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_RX_BROADCAST:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_RX_OCTET_64:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_TX_LO_PRIORITY:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_TX_BROADCAST:
                return( pPort->bCurrentCounter );
            case MIB_COUNTER_TX_TOTAL_COLLISION:
                return( pPort->bCurrentCounter );
        }
#endif
    } while ( pPort->bCurrentCounter < PORT_COUNTER_NUM );
    if ( bPort < HOST_PORT )
        PortReadMIBPacket( pHardware, bPort, pPort->cnDropped,
            &pPort->cnCounter[ pPort->bCurrentCounter ]);
    pPort->bCurrentCounter = 0;
    return 0;
}  /* PortReadCounters */


/*
    PortInitCounters

    Description:
        This routine is used to initialize all counters to zero if the hardware
        cannot do it after reset.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

void PortInitCounters (
    PHARDWARE pHardware,
    UCHAR     bPort )
{
    PPORT_CONFIG pPort = &pHardware->m_Port[ bPort ];


    ASSERT( pHardware->m_bAcquire );

    pPort->bCurrentCounter = 0;
ACT_DEBUG_
    do
    {
        PortReadMIBCounter( pHardware, bPort, pPort->bCurrentCounter,
            &pPort->cnCounter[ pPort->bCurrentCounter ]);
        ++pPort->bCurrentCounter;
    } while ( pPort->bCurrentCounter < PORT_COUNTER_NUM );

    PortReadMIBPacket( pHardware, bPort, pPort->cnDropped,
        &pPort->cnCounter[ pPort->bCurrentCounter ]);
    memset(( void* ) pPort->cnCounter, 0,
        sizeof( ULONGLONG ) * TOTAL_PORT_COUNTER_NUM );
    pPort->bCurrentCounter = 0;
}  /* PortInitCounters */


#endif
