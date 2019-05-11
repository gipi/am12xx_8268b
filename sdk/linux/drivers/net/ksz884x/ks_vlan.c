/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_vlan.c - KS884X switch VLAN functions.

    Author  Date      Version  Description
    THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                               some hardware platforms.
    PCD     08/10/05  0.1.7    Fix incorrect register offset in configuration of VLAN.
    THa     10/11/04           Updated with latest specs.
    THa     09/28/04           Updated for PCI version.
    THa     02/13/04           Created file.
   ---------------------------------------------------------------------------
*/


#ifdef DEF_KS8842
#include "target.h"
#include "hardware.h"


/* -------------------------------------------------------------------------- */

/*
    PortConfigDefaultVID

    Description:
        This routine configures the default VID of the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        USHORT wVID
            The VID value.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortConfigDefaultVID_PCI
#else
void PortConfigDefaultVID_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    USHORT    wVID )
{
#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, ( UCHAR )
        ( REG_PORT_CTRL_BANK + bPort * PORT_BANK_INTERVAL ));

    HW_WRITE_WORD( pHardware, REG_PORT_CTRL_VID_OFFSET, wVID );

#else  /* PCI BUS */
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += REG_PORT_CTRL_VID_OFFSET;

    HW_WRITE_WORD( pHardware, ulAddr, wVID );
#endif

}  /* PortConfigDefaultVID */


/*
    PortGetDefaultVID

    Description:
        This routine retrieves the default VID of the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        PUSHORT pwVID
            Buffer to store the VID.

    Return (None):
*/

#ifdef KS_PCI_BUS
void PortGetDefaultVID_PCI
#else
void PortGetDefaultVID_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    PUSHORT   pwVID )
{
#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, ( UCHAR )
        ( REG_PORT_CTRL_BANK + bPort * PORT_BANK_INTERVAL ));

    HW_READ_WORD( pHardware, REG_PORT_CTRL_VID_OFFSET, pwVID );

#else  /* PCI BUS */
    ULONG ulAddr;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += REG_PORT_CTRL_VID_OFFSET;

    HW_READ_WORD( pHardware, ulAddr, pwVID );
#endif
}  /* PortGetDefaultVID */


#ifdef KS_PCI_BUS
#define PortConfigDefaultVID  PortConfigDefaultVID_PCI
#else
#define PortConfigDefaultVID  PortConfigDefaultVID_ISA
#endif


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchDisableVlan

    Description:
        This routine disables the VLAN function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchDisableVlan (
    PHARDWARE pHardware )
{
    ASSERT( pHardware->m_bAcquire );

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        SwitchConfigSet_ISA( pHardware, REG_SWITCH_CTRL_2_HI_OFFSET,
            SWITCH_VLAN_ENABLE, FALSE );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        SwitchConfigSet_PCI( pHardware, REG_SWITCH_CTRL_2_HI_OFFSET,
            SWITCH_VLAN_ENABLE, FALSE );
    }
#endif
}  /* SwitchDisableVlan */
#endif


/*
    SwitchEnableVlan

    Description:
        This routine enables the VLAN function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableVlan_PCI
#else
void SwitchEnableVlan_ISA
#endif
(
    PHARDWARE pHardware
)
{
    UCHAR bEntry;


    ASSERT( pHardware->m_bAcquire );

    /* Create 16 VLAN entries in the VLAN table */
    for ( bEntry = 0; bEntry < VLAN_TABLE_ENTRIES; bEntry++ )
    {
        if ( pHardware->m_VlanTable[ bEntry ].wVID )
        {
            SwitchWriteVlanTable( pHardware, bEntry,
                pHardware->m_VlanTable[ bEntry ].wVID,
                pHardware->m_VlanTable[ bEntry ].bFID,
                pHardware->m_VlanTable[ bEntry ].bMember,
                TRUE );
        }
        else
        {
            SwitchWriteVlanTable( pHardware, bEntry, 0, 0, 0, FALSE );
        }
    }

    /* Enable 802.1q VLAN mode */
    SwitchConfigSet( pHardware, REG_SWITCH_CTRL_2_OFFSET,
                     UNICAST_VLAN_BOUNDARY, TRUE );
    SwitchConfigSet( pHardware, REG_SWITCH_CTRL_2_HI_OFFSET,
                     SWITCH_VLAN_ENABLE, TRUE );

}  /* SwitchEnableVlan */


#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchInitVlan

    Description:
        This routine initializes the VLAN of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchInitVlan (
    PHARDWARE pHardware )
{
    UCHAR bPort;

    pHardware->m_VlanTable[ 0 ].wVID = 1;
    pHardware->m_VlanTable[ 0 ].bFID = 0;
    pHardware->m_VlanTable[ 0 ].bMember = 7;
    for ( bPort = 0; bPort < TOTAL_PORT_NUM; bPort++ )
    {
        pHardware->m_Port[ bPort ].wVID = 1;
        pHardware->m_Port[ bPort ].bMember = 7;
    }
}  /* SwitchInitVlan */


/*
    HardwareConfigDefaultVID

    Description:
        This routine configures the default VID of the port.  It is called from
        user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        USHORT wVID
            The VID value.

    Return (None):
*/

void HardwareConfigDefaultVID (
    PHARDWARE pHardware,
    UCHAR     bPort,
    USHORT    wVID )
{
    pHardware->m_Port[ bPort ].wVID = wVID;

#ifdef KS_ISA_BUS
#ifdef KS_PCI
    if ( !pHardware->m_fPCI )
#endif
    {
        PortConfigDefaultVID_ISA( pHardware, bPort, wVID );
    }

#ifdef KS_PCI
    else
#endif
#endif

#ifdef KS_PCI
    {
        PortConfigDefaultVID_PCI( pHardware, bPort, wVID );
    }
#endif
}  /* HardwareConfigDefaultVID */


/*
    HardwareConfigPortBaseVlan

    Description:
        This routine configures the port-base VLAN membership of the port.
        It is called from user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        UCHAR bMember
            The port-base VLAN membership.

    Return (None):
*/

void HardwareConfigPortBaseVlan
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bMember
)
{
#ifdef KS_ISA_BUS
    USHORT usData;

    HardwareSelectBank( pHardware, ( UCHAR )
        ( REG_PORT_CTRL_BANK + bPort * PORT_BANK_INTERVAL ));

    HW_READ_WORD( pHardware, REG_PORT_CTRL_2_OFFSET, &usData );
    usData &= ~PORT_VLAN_MEMBERSHIP;
    usData |= ( bMember & PORT_VLAN_MEMBERSHIP );
    HW_WRITE_WORD( pHardware, REG_PORT_CTRL_2_OFFSET, usData );

#else  /* PCI BUS */
    ULONG ulAddr;
    UCHAR bData;

    PORT_CTRL_ADDR( bPort, ulAddr );
    ulAddr += REG_PORT_CTRL_2_OFFSET;

    HW_READ_BYTE( pHardware, ulAddr, &bData );
    bData &= ~PORT_VLAN_MEMBERSHIP;
    bData |= ( bMember & PORT_VLAN_MEMBERSHIP );
    HW_WRITE_BYTE( pHardware, ulAddr, bData );
#endif

    pHardware->m_Port[ bPort ].bMember = bMember;

}  /* HardwareConfigPortBaseVlan */


/*
    SwitchVlanConfigDiscardNonVID

    Description:
        This routine configures Discard Non PVID packets of the switch port.
        If enabled, the device will discard packets whose VLAN id does not match
        ingress port-base default VLAN id.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        BOOLEAN  fSet
            TRUE, enable; FALSE, disable.

    Return (None):
*/

void SwitchVlanConfigDiscardNonVID
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigDiscardNonVID( pHardware, bPort, fSet );

}  /* SwitchVlanConfigDiscardNonVID */

/*
    SwitchVlanConfigIngressFiltering

    Description:
        This routine configures Ingress VLAN filtering of the switch port.
        If enabled, the device will discard packets whose VLAN id membership
        in the VLAN table bits [18:16] does not include the ingress port that
        received this packet.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        BOOLEAN  fSet
            TRUE, enable; FALSE, disable.

    Return (None):
*/

void SwitchVlanConfigIngressFiltering
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigIngressFiltering( pHardware, bPort, fSet );

}  /* SwitchVlanConfigIngressFiltering */

/*
    SwitchVlanConfigInsertTag

    Description:
        This routine configures 802.1q Tag insertion to the switch port.
        If enabled, the device will insert 802.1q tag to the transmit packet
        on this port if received packet is an untagged packet. The device
        will not insert 802.1q tag if received packet is tagged packet.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        BOOLEAN  fSet
            TRUE, enable; FALSE, disable.

    Return (None):
*/

void SwitchVlanConfigInsertTag
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigInsertTag( pHardware, bPort, fSet );

}  /* SwitchVlanConfigInsertTag */

/*
    SwitchVlanConfigRemoveTag

    Description:
        This routine configures 802.1q Tag removal to the switch port.
        If enabled, the device will removed 802.1q tag to the transmit packet
        on this port if received packet is a tagged packet. The device will
        not remove 802.1q tag if received packet is untagged packet.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

        BOOLEAN  fSet
            TRUE, enable; FALSE, disable.

    Return (None):
*/

void SwitchVlanConfigRemoveTag
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigRemoveTag( pHardware, bPort, fSet );

}  /* SwitchVlanConfigRemoveTag */

#endif
#endif
