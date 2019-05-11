/* ---------------------------------------------------------------------------
             Copyright (c) 2004 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_mirror.c - KS884X switch mirror functions.

    Author      Date        Description
    THa         02/13/04    Created file.
    THa         09/28/04    Updated for PCI version.
    THa         10/08/04    Updated with latest specs.
   ---------------------------------------------------------------------------
*/


#ifdef DEF_KS8842
#include "target.h"
#include "hardware.h"


/* -------------------------------------------------------------------------- */

/*
    SwitchDisableMirrorSniffer

    Description:
        This routine disables the mirror sniffer port of the switch port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the sniffer.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableMirrorSniffer_PCI
#else
void SwitchDisableMirrorSniffer_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorSniffer( pHardware, bPort, FALSE );

}  /* SwitchDisableMirrorSniffer */

/*
    SwitchEnableMirrorSniffer

    Description:
        This routine enables the mirror sniffer port of the switch port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the sniffer.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableMirrorSniffer_PCI
#else
void SwitchEnableMirrorSniffer_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorSniffer( pHardware, bPort, TRUE );

}  /* SwitchEnableMirrorSniffer */


/*
    SwitchDisableMirrorReceive

    Description:
        This routine disables the "receive only" mirror on a port of the switch port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the monitoring.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableMirrorReceive_PCI
#else
void SwitchDisableMirrorReceive_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorReceive( pHardware, bPort, FALSE );

}  /* SwitchDisableMirrorReceive */

/*
    SwitchEnableMirrorReceive

    Description:
        This routine disables the "receive only" mirror on a port of the switch port.
        All the packets received on this port are mirrored to the sniffer port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the monitoring.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableMirrorReceive_PCI
#else
void SwitchEnableMirrorReceive_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorReceive( pHardware, bPort, TRUE );

}  /* SwitchEnableMirrorReceive */


/*
    SwitchDisableMirrorTransmit

    Description:
        This routine disables the "transmit only" mirror on a port of the switch port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the monitoring.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableMirrorTransmit_PCI
#else
void SwitchDisableMirrorTransmit_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorTransmit( pHardware, bPort, FALSE );

}  /* SwitchDisableMirrorTransmit */

/*
    SwitchEnableMirrorTransmit

    Description:
        This routine disables the "transmit only" mirror on a port of the switch port.
        All the packets transmiited on this port are mirrored on the sniffer port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port to be the monitoring.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableMirrorTransmit_PCI
#else
void SwitchEnableMirrorTransmit_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPort )
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigMirrorTransmit( pHardware, bPort, TRUE );

}  /* SwitchEnableMirrorTransmit */


/*
    SwitchDisableMirrorRxAndTx

    Description:
        This routine disables the "receive AND transmit" mirror of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableMirrorRxAndTx_PCI
#else
void SwitchDisableMirrorRxAndTx_ISA
#endif
(
    PHARDWARE pHardware
)
{

    ASSERT( pHardware->m_bAcquire );

    SwitchConfigMirrorRxAndTx( pHardware, FALSE );

}  /* SwitchDisableMirrorRxAndTx */

/*
    SwitchEnableMirrorRxAndTx

    Description:
        This routine enables the "receive AND transmit" mirror of the switch. All the packets 
        received on port A AND transmiited on port B are mirrored to the sniffer port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableMirrorRxAndTx_PCI
#else
void SwitchEnableMirrorRxAndTx_ISA
#endif
(
    PHARDWARE pHardware
)
{

    ASSERT( pHardware->m_bAcquire );

    SwitchConfigMirrorRxAndTx( pHardware, TRUE );

}  /* SwitchEnableMirrorRxAndTx */


/*
    SwitchInitMirror

    Description:
        This routine initializes /disables the switch Mirroring functions.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchInitMirror (
    PHARDWARE pHardware )
{
    UCHAR bPort;


    /* Disable all Mirroring functions */
    for ( bPort=0; bPort < TOTAL_PORT_NUM; bPort++ )
    {
        SwitchDisableMirrorSniffer( pHardware, bPort );
        SwitchDisableMirrorReceive( pHardware, bPort );
        SwitchDisableMirrorTransmit( pHardware, bPort );
        SwitchDisableMirrorRxAndTx( pHardware );
    }

}  /* SwitchInitMirror */
#endif
