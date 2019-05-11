/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_qos.c - KS884X switch QoS priority functions.

    Author      Date        Description
    THa         02/13/04    Created file.
    THa         09/29/04    Updated for PCI version.
    THa         10/11/04    Updated with latest specs.
    THa         02/28/06    Do not use HW_WRITE_BYTE because of limitation of
                            some hardware platforms.
   ---------------------------------------------------------------------------
*/


#ifdef DEF_KS8842
#include "target.h"
#include "hardware.h"


/* -------------------------------------------------------------------------- */


#define MAX_TOS_IN_FIRST_TOS_BANK         48

/*
    SwitchSet_TOS_Priority

    Description:
        This routine programs the TOS priority into the switch registers.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bTosValue
            ToS value from 6-bit (bit7 ~ bit2) of ToS field, range from 0 to 63.

        USHORT wPriority
            Priority to be assigned.

    Return (None):
*/

static
void SwitchSet_TOS_Priority
(
    PHARDWARE pHardware,
    UCHAR     bTosValue,
    USHORT    wPriority
)
{
    USHORT    bOffset;
    USHORT    bShift;
    USHORT    bIndex;
    USHORT    Priority;
    USHORT    wRegData;
    USHORT    wMask = 0x00000003;

    if (bTosValue < MAX_TOS_IN_FIRST_TOS_BANK)
    {
        bOffset = REG_TOS_1_OFFSET;
        bIndex = bTosValue / 8;
    }
    else
    {
        bOffset = REG_TOS_7_OFFSET;
        bIndex = (bTosValue - MAX_TOS_IN_FIRST_TOS_BANK) / 8;
    }

    if (bIndex > 0)
        bOffset += (bIndex << 1);

    bShift = (bTosValue % 8) << 1;
    Priority = wPriority << bShift;
    wMask <<= bShift;

#if (0)
    DBG_PRINT ("bTosValue=%d, wPriority=%04x, bIndex=%d, bShift=%d, bOffset=%d, Priority=%04x"NEWLINE,
                bTosValue, wPriority, bIndex, bShift, bOffset, Priority );
#endif

    if (bTosValue < MAX_TOS_IN_FIRST_TOS_BANK)
    {
#ifdef KS_ISA_BUS
        HardwareSelectBank( pHardware, REG_TOS_PRIORITY_BANK );
#endif
        HW_READ_WORD ( pHardware, bOffset, &wRegData );
        wRegData &= ~wMask;
        wRegData |= Priority;
        HW_WRITE_WORD( pHardware, bOffset, wRegData );
    }
    else
    {
#ifdef KS_ISA_BUS
        HardwareSelectBank( pHardware, REG_TOS_PRIORITY_2_BANK );
#endif
        HW_READ_WORD ( pHardware, bOffset, &wRegData );
        wRegData &= ~wMask;
        wRegData |= Priority;
        HW_WRITE_WORD( pHardware, bOffset, wRegData );
    }

}  /* SwitchSet_TOS_Priority */


/*
    SwitchDisableDiffServ

    Description:
        This routine disables the DiffServ priority function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableDiffServ_PCI
#else
void SwitchDisableDiffServ_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigDiffServ( pHardware, bPort, FALSE );

}  /* SwitchEnableDiffServ */


/*
    SwitchEnableDiffServ

    Description:
        This routine enables the DiffServ priority function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableDiffServ_PCI
#else
void SwitchEnableDiffServ_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{
    UCHAR bTosValue;

    ASSERT( pHardware->m_bAcquire );

    for ( bTosValue=0; bTosValue < 64; bTosValue++ )
        SwitchSet_TOS_Priority( pHardware, bTosValue,
            pHardware->m_wDiffServ[ bTosValue ] );

    PortConfigDiffServ( pHardware, bPort, TRUE );

}  /* SwitchEnableDiffServ */


/*
    HardwareConfig_TOS_Priority

    Description:
        This routine configures the TOS priority in the hardware.
        DiffServ Value 0~63 is mapping to Priority Queue Number 0 ~ 3.
        It is called by user functions. The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR  bTosValue
            ToS value from 6-bit (bit7 ~ bit2) of ToS field, range from 0 to 63.

        USHORT bPriority
            Priority to be assigned.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareConfig_TOS_Priority_PCI
#else
void HardwareConfig_TOS_Priority_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bTosValue,
    USHORT    wPriority
)
{

    if ( (wPriority > 63) ||
         (wPriority > 3) )
         return;

    ASSERT( pHardware->m_bAcquire );

    pHardware->m_wDiffServ[ bTosValue ] = wPriority;
    SwitchSet_TOS_Priority( pHardware, bTosValue, wPriority );

}  /* HardwareConfig_TOS_Priority */


/* -------------------------------------------------------------------------- */


/*
    SwitchSet802_1P_Priority

    Description:
        This routine programs the 802.1p priority into the switch register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR  bTagPriorityValue
            The 802.1p tag priority value, range from 0 to 7.

        USHORT bPriority
            Priority to be assigned.


    Return (None):
*/

static
void SwitchSet802_1P_Priority
(
    PHARDWARE pHardware,
    UCHAR     bTagPriorityValue,
    USHORT    wPriority
)
{
    UCHAR     bShift;
    USHORT    Priority;
    USHORT    wRegData;
    USHORT    wMask = 0x00000003;

    bShift = bTagPriorityValue << 1;
    Priority = wPriority << bShift;
    wMask <<= bShift;

#if (0)
    DBG_PRINT ("bTagPriorityValue=%d, wPriority=%04x, bShift=%d, Priority=%04x"NEWLINE,
                bTagPriorityValue, wPriority, bShift, Priority );
#endif

#ifdef KS_ISA_BUS
    HardwareSelectBank( pHardware, REG_SWITCH_802_1P_BANK );
#endif
    HW_READ_WORD( pHardware, REG_SWITCH_CTRL_6_OFFSET, &wRegData );
    wRegData &= ~wMask;
    wRegData |= Priority;
    HW_WRITE_WORD( pHardware, REG_SWITCH_CTRL_6_OFFSET, wRegData );

}  /* SwitchSet802_1P_Priority */


/*
    SwitchDisable802_1P

    Description:
        This routine disables the 802.1p priority function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisable802_1P_PCI
#else
void SwitchDisable802_1P_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfig802_1P( pHardware, bPort, FALSE );

}  /* SwitchDisable802_1P */


/*
    SwitchEnable802_1P

    Description:
        This routine enables the 802.1p priority function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnable802_1P_PCI
#else
void SwitchEnable802_1P_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{
    UCHAR bTagPriorityValue;

    ASSERT( pHardware->m_bAcquire );

    for ( bTagPriorityValue=0; bTagPriorityValue < 8; bTagPriorityValue++ )
        SwitchSet802_1P_Priority( pHardware, bTagPriorityValue,
            pHardware->m_b802_1P_Priority[ bTagPriorityValue ] );

    PortConfig802_1P( pHardware, bPort, TRUE );

}  /* SwitchEnable802_1P */


/*
    HardwareConfig802_1P_Priority

    Description:
        This routine configures the 802.1p priority in the hardware.
        802.1p Tag priority value 0~7 is mapping to Priority Queue Number 0 ~ 3.
        It is called by user functions.  The hardware should be acquired first.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR  bTagPriorityValue
            The 802.1p tag priority value, range from 0 to 7.

        USHORT bPriority
            Priority to be assigned.

    Return (None):
*/

#ifdef KS_PCI_BUS
void HardwareConfig802_1P_Priorit_PCI
#else
void HardwareConfig802_1P_Priorit_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bTagPriorityValue,
    USHORT    wPriority
)
{
    if ( (bTagPriorityValue > SWITCH_802_1P_BASE) ||
         (wPriority > 3) )
         return;

    ASSERT( pHardware->m_bAcquire );

    pHardware->m_b802_1P_Priority[ bTagPriorityValue ] = wPriority;
    SwitchSet802_1P_Priority( pHardware, bTagPriorityValue, wPriority );

}  /* HardwareConfig802_1P_Priority */


/*
    SwitchDisableDot1pRemapping

    Description:
        This routine disables the 802.1p priority re-mapping function of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableDot1pRemapping_PCI
#else
void SwitchDisableDot1pRemapping_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfig802_1P_Remapping( pHardware, bPort, FALSE );

}  /* SwitchDisableDot1pRemapping */


/*
    SwitchEnableDot1pRemapping

    Description:
        This routine enables the 802.1p priority re-mapping function of the switch.
        That allows 802.1p priority field is replaced with the Port's default tag's
        priority value if the ingress packet's 802.1p priority has a higher priority than
        Port's default tag's priority.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableDot1pRemapping_PCI
#else
void SwitchEnableDot1pRemapping_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfig802_1P_Remapping( pHardware, bPort, TRUE );

}  /* SwitchEnableDot1pRemapping */


/* -------------------------------------------------------------------------- */

/*
    SwitchConfigPortBased

    Description:
        This routine configures the port based priority of the switch.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPriority
            The priority to set.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchConfigPortBased_PCI
#else
void SwitchConfigPortBased_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR     bPriority,
    UCHAR     bPort
)
{
    USHORT usData;

    ASSERT( pHardware->m_bAcquire );

    if ( bPriority > PORT_BASED_PRIORITY_BASE )
        bPriority = PORT_BASED_PRIORITY_BASE;

    pHardware->m_Port[ bPort ].bPortPriority = bPriority;

    PortConfigReadWord( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        REG_PORT_CTRL_1_OFFSET, &usData );
    usData &= ~PORT_BASED_PRIORITY_MASK;
    usData |= bPriority << PORT_BASED_PRIORITY_SHIFT;

    PortConfigWriteWord( pHardware, bPort,

#ifdef KS_ISA_BUS
        REG_PORT_CTRL_BANK,
#endif
        REG_PORT_CTRL_1_OFFSET, usData );
}  /* SwitchConfigPortBased */

/* -------------------------------------------------------------------------- */

/*
    SwitchDisableMultiQueue

    Description:
        This routine disables the transmit multiple queues selection of the switch port.
        Only single transmit queue on the port.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchDisableMultiQueue_PCI
#else
void SwitchDisableMultiQueue_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigPriority( pHardware, bPort, FALSE );

}  /* SwitchDisableMultiQueue */


/*
    SwitchEnableMultiQueue

    Description:
        This routine enables the transmit multiple queues selection of the switch port.
        The port transmit queue is split into four priority queues.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bPort
            The port index.

    Return (None):
*/

#ifdef KS_PCI_BUS
void SwitchEnableMultiQueue_PCI
#else
void SwitchEnableMultiQueue_ISA
#endif
(
    PHARDWARE pHardware,
    UCHAR bPort
)
{

    ASSERT( pHardware->m_bAcquire );

    PortConfigPriority( pHardware, bPort, TRUE );

}  /* SwitchEnableMultiQueue */


/* -------------------------------------------------------------------------- */

#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
/*
    SwitchInitPriority

    Description:
        This routine initializes the switch QoS priority functions.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void SwitchInitPriority (
    PHARDWARE pHardware )
{
    UCHAR bTosValue;
#if 0
    UCHAR bTagPriorityValue;
#endif
    UCHAR bPort;

    pHardware->m_w802_1P_Mapping = 0;

    /* init all the 802.1p Tag priority value assign to different priority queue */
    pHardware->m_b802_1P_Priority[0] = 0;
    pHardware->m_b802_1P_Priority[1] = 0;
    pHardware->m_b802_1P_Priority[2] = 1;
    pHardware->m_b802_1P_Priority[3] = 1;
    pHardware->m_b802_1P_Priority[4] = 2;
    pHardware->m_b802_1P_Priority[5] = 2;
    pHardware->m_b802_1P_Priority[6] = 3;
    pHardware->m_b802_1P_Priority[7] = 3;

    /* init all the DiffServ priority value assign to priority queue 0 */
    for ( bTosValue=0; bTosValue < 64; bTosValue++ )
        pHardware->m_wDiffServ[ bTosValue ] = 0;

    /* Init All QoS function disabled */
    for ( bPort=0; bPort < TOTAL_PORT_NUM; bPort++ )
    {
        SwitchDisableMultiQueue( pHardware, bPort );
        SwitchDisableDiffServ( pHardware, bPort );
        SwitchDisable802_1P( pHardware, bPort );
        SwitchDisableDot1pRemapping( pHardware, bPort );

        pHardware->m_Port[ bPort ].bPortPriority = 0;
        SwitchConfigPortBased ( pHardware,
            pHardware->m_Port[ bPort ].bPortPriority, bPort );
    }

}  /* SwitchInitPriority */
#endif
#endif
