/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_stp.c - KS884X switch Spanning Tree Protocol support functions.

    Author      Date        Description
    THa         02/13/04    Created file.
    THa         09/28/04    Updated for PCI version.
    THa         10/08/04    Updated with latest specs.
    THa         02/28/06    Do not use HW_WRITE_BYTE because of limitation of
                            some hardware platforms.
    THa         04/03/06    Move STP state definitions to ks_config.h.
   ---------------------------------------------------------------------------
*/


#ifdef DEF_KS8842
#include "target.h"
#include "hardware.h"


/* -------------------------------------------------------------------------- */

#define STP_ENTRY  0

/* -------------------------------------------------------------------------- */

/*
    PortSet_STP_State

    Description:
        This routine configures the spanning tree state of the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        int nState
            The spanning tree state.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortSet_STP_State_PCI
#else
void PortSet_STP_State_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    int       nState )
{
#ifdef SH_16BIT_WRITE
    USHORT usData;
#endif
    UCHAR Data;

    ASSERT( pHardware->m_bAcquire );

#ifdef SH_16BIT_WRITE
    PortConfigReadWord( pHardware, bPort,
        REG_PORT_CTRL_BANK,
        REG_PORT_CTRL_2_OFFSET, &usData );
    Data = TO_LO_BYTE( usData );

#else
    PortConfigReadByte( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        REG_PORT_CTRL_2_HI_OFFSET, &Data );
#endif
    switch ( nState )
    {
        case STP_STATE_DISABLED:
            Data &= ~( PORT_TX_ENABLE | PORT_RX_ENABLE );
            Data |= PORT_LEARN_DISABLE;
            break;
        case STP_STATE_LISTENING:
            Data &= ~( PORT_TX_ENABLE | PORT_RX_ENABLE );
            Data |= PORT_LEARN_DISABLE;
            break;
        case STP_STATE_LEARNING:
            Data &= ~( PORT_TX_ENABLE | PORT_RX_ENABLE );
            Data &= ~PORT_LEARN_DISABLE;
            break;
        case STP_STATE_FORWARDING:
            Data |= ( PORT_TX_ENABLE | PORT_RX_ENABLE );
            Data &= ~PORT_LEARN_DISABLE;
            break;
        case STP_STATE_BLOCKED:
            Data &= ~( PORT_TX_ENABLE | PORT_RX_ENABLE );
            Data |= PORT_LEARN_DISABLE;
            break;
    }

#ifdef SH_16BIT_WRITE
    usData &= 0x00FF;
    usData |= TO_HI_BYTE( Data );
    PortConfigWriteWord( pHardware, bPort,
        REG_PORT_CTRL_BANK,
        REG_PORT_CTRL_2_OFFSET, usData );

#else
    PortConfigWriteByte( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        REG_PORT_CTRL_2_HI_OFFSET, Data );
#endif
    pHardware->m_PortInfo[ bPort ].nSTP_State = nState;
}  /* PortSet_STP_State */


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    HardwareInit_STP

    Description:
        This routine initializes the spanning tree support of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareInit_STP (
    PHARDWARE pHardware )
{
    PMAC_TABLE pMacTableEntry;
    UCHAR      bPort;

    pMacTableEntry = &pHardware->m_MacTable[ STP_ENTRY ];
    pMacTableEntry->MacAddr[ 0 ] = 0x01;
    pMacTableEntry->MacAddr[ 1 ] = 0x80;
    pMacTableEntry->MacAddr[ 2 ] = 0xC2;
    pMacTableEntry->MacAddr[ 3 ] = 0x00;
    pMacTableEntry->MacAddr[ 4 ] = 0x00;
    pMacTableEntry->MacAddr[ 5 ] = 0x00;
    pMacTableEntry->bPorts = 0x4;
    pMacTableEntry->fOverride = TRUE;
    pMacTableEntry->fValid = TRUE;
    SwitchWriteStaticMacTable( pHardware, STP_ENTRY,
        pMacTableEntry->MacAddr, pMacTableEntry->bPorts,
        pMacTableEntry->fOverride, pMacTableEntry->fValid,
        pMacTableEntry->fUseFID, pMacTableEntry->bFID );
    for ( bPort = 0; bPort < SWITCH_PORT_NUM; bPort++ )
    {
        PortSet_STP_State( pHardware, bPort, STP_STATE_FORWARDING );
    }
}  /* HardwareInit_STP */
#endif
#endif
