/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_config.h - KS884X switch configuration functions header.

    Author  Date      Version  Description
    THa     04/03/06           Move STP state definitions from ks_stp.c.
    THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                               some hardware platforms.
    PCD     08/10/05  0.1.7    Fix incorrect register offset in configuration of VLAN.
    THa     10/14/04           Updated with latest specs.
    THa     09/30/04           Updated for PCI version.
    THa     02/13/04           Created file.
   ---------------------------------------------------------------------------
*/


#ifndef __KS_CONFIG_H
#define __KS_CONFIG_H


/* -------------------------------------------------------------------------- */

#ifdef KS_ISA_BUS
ULONG SwapBytes (
    ULONG dwData );

BOOLEAN PortConfigGet_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bffset,
    UCHAR     bBits );

void PortConfigSet_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bOffset,

#ifdef SH_16BIT_WRITE
    USHORT    bBits,

#else
    UCHAR     bBits,
#endif
    BOOLEAN   fSet );
#endif

#ifdef KS_PCI_BUS
BOOLEAN PortConfigGetShift (
    PHARDWARE pHardware,
    UCHAR     bPort,
    ULONG     ulOffset,
    UCHAR     bShift );

void PortConfigSetShift (
    PHARDWARE pHardware,
    UCHAR     bPort,
    ULONG     ulOffset,
    UCHAR     bShift,
    BOOLEAN   fSet );
#endif

#ifdef KS_ISA_BUS
void PortConfigReadByte_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUCHAR    pbData );

void PortConfigWriteByte_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bOffset,
    UCHAR     bData );

void PortConfigReadWord_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bOffset,
    PUSHORT   pwData );

void PortConfigWriteWord_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bBank,
    UCHAR     bOffset,
    USHORT    usData );

BOOLEAN SwitchConfigGet_ISA (
    PHARDWARE pHardware,
    int       Offset,
    UCHAR     bBits );

void SwitchConfigSet_ISA (
    PHARDWARE pHardware,
    int       Offset,

#ifdef SH_16BIT_WRITE
    USHORT    bBits,

#else
    UCHAR     bBits,
#endif
    BOOLEAN   fSet );
#endif

#ifdef KS_PCI_BUS
BOOLEAN PortConfigGet_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    UCHAR     bBits );

void PortConfigSet_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    UCHAR     bBits,
    BOOLEAN   fSet );

void PortConfigReadByte_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    PUCHAR    pbData );

void PortConfigWriteByte_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    UCHAR     bData );

void PortConfigReadWord_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    PUSHORT   pwData );

void PortConfigWriteWord_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bOffset,
    USHORT    usData );

BOOLEAN SwitchConfigGet_PCI (
    PHARDWARE pHardware,
    int       Offset,
    UCHAR     bBits );

void SwitchConfigSet_PCI (
    PHARDWARE pHardware,
    int       Offset,
    UCHAR     bBits,
    BOOLEAN   fSet );
#endif

#ifdef KS_PCI_BUS
#define SwitchConfigReadByte( phwi, offset, data )                          \
    HW_READ_BYTE( phwi, offset, data )

#define SwitchConfigWriteByte( phwi, offset, data )                         \
    HW_WRITE_BYTE( phwi, offset, data )

#define PortConfigGet  PortConfigGet_PCI
#define PortConfigSet  PortConfigSet_PCI
#define PortConfigReadByte  PortConfigReadByte_PCI
#define PortConfigWriteByte  PortConfigWriteByte_PCI
#define PortConfigReadWord  PortConfigReadWord_PCI
#define PortConfigWriteWord  PortConfigWriteWord_PCI
#define SwitchConfigGet  SwitchConfigGet_PCI
#define SwitchConfigSet  SwitchConfigSet_PCI

#else
#define SwitchConfigReadByte( phwi, offset, data )                          \
    HardwareReadRegByte( phwi, REG_SWITCH_CTRL_BANK, offset, data )

#define SwitchConfigWriteByte( phwi, offset, data )                         \
    HardwareWriteRegByte( phwi, REG_SWITCH_CTRL_BANK, offset, data )

#define PortConfigGet PortConfigGet_ISA
#define PortConfigSet  PortConfigSet_ISA
#define PortConfigReadByte  PortConfigReadByte_ISA
#define PortConfigWriteByte  PortConfigWriteByte_ISA
#define PortConfigReadWord  PortConfigReadWord_ISA
#define PortConfigWriteWord  PortConfigWriteWord_ISA
#define SwitchConfigGet  SwitchConfigGet_ISA
#define SwitchConfigSet  SwitchConfigSet_ISA
#endif

/* -------------------------------------------------------------------------- */

/* Bandwidth */

#ifdef KS_PCI_BUS
#define PortConfigBroadcastStorm( phwi, port, enable )                      \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_BROADCAST_STORM, enable )

#define PortGetBroadcastStorm( phwi, port )                                 \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_BROADCAST_STORM )

#else
#define PortConfigBroadcastStorm( phwi, port, enable )                      \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_BROADCAST_STORM, enable )

#define PortGetBroadcastStorm( phwi, port )                                 \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_BROADCAST_STORM )
#endif


/* Communication */

#ifdef KS_PCI_BUS
#define PortConfigBackPressure( phwi, port, enable )                        \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_BACK_PRESSURE, enable )

#define PortConfigForceFlowCtrl( phwi, port, enable )                       \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_FORCE_FLOW_CTRL, enable )

#define PortGetBackPressure( phwi, port )                                   \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_BACK_PRESSURE )

#define PortGetForceFlowCtrl( phwi, port )                                  \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_FORCE_FLOW_CTRL )

#else
#define PortConfigBackPressure( phwi, port, enable )                        \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_BACK_PRESSURE, enable )

#define PortConfigForceFlowCtrl( phwi, port, enable )                       \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_FORCE_FLOW_CTRL, enable )

#define PortGetBackPressure( phwi, port )                                   \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_BACK_PRESSURE )

#define PortGetForceFlowCtrl( phwi, port )                                  \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_FORCE_FLOW_CTRL )
#endif


/* Spanning Tree */

#ifdef KS_PCI_BUS
#define PortConfigDisableLearning( phwi, port, enable )                     \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_LEARN_DISABLE, enable )

#define PortConfigEnableReceive( phwi, port, enable )                       \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_RX_ENABLE, enable )

#define PortConfigEnableTransmit( phwi, port, enable )                      \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_TX_ENABLE, enable )

#else
#define PortConfigDisableLearning( phwi, port, enable )                     \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_LEARN_DISABLE, enable )

#define PortConfigEnableReceive( phwi, port, enable )                       \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_RX_ENABLE, enable )

#define PortConfigEnableTransmit( phwi, port, enable )                      \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_TX_ENABLE, enable )
#endif


/* VLAN */

#ifdef KS_PCI_BUS
#define PortConfigDiscardNonVID( phwi, port, enable )                       \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_DISCARD_NON_VID, enable )

#define PortConfigDoubleTag( phwi, port, enable )                           \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_DOUBLE_TAG, enable )

#define PortConfigIngressFiltering( phwi, port, enable )                    \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_INGRESS_FILTER, enable )

#define PortConfigInsertTag( phwi, port, insert )                           \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_INSERT_TAG, insert )

#define PortConfigRemoveTag( phwi, port, remove )                           \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_REMOVE_TAG, remove )

#define PortGetDiscardNonVID( phwi, port )                                  \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_DISCARD_NON_VID )

#define PortGetDoubleTag( phwi, port )                                      \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_DOUBLE_TAG )

#define PortGetIngressFiltering( phwi, port )                               \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_INGRESS_FILTER )

#define PortGetInsertTag( phwi, port )                                      \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_INSERT_TAG )

#define PortGetRemoveTag( phwi, port )                                      \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_REMOVE_TAG )

#else
#define PortConfigDiscardNonVID( phwi, port, enable )                       \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_DISCARD_NON_VID, enable )

#define PortConfigDoubleTag( phwi, port, enable )                           \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_DOUBLE_TAG, enable )

#define PortConfigIngressFiltering( phwi, port, enable )                    \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_INGRESS_FILTER, enable )

#define PortConfigInsertTag( phwi, port, insert )                           \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_INSERT_TAG, insert )

#define PortConfigRemoveTag( phwi, port, remove )                           \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_REMOVE_TAG, remove )

#define PortGetDiscardNonVID( phwi, port )                                  \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_DISCARD_NON_VID )

#define PortGetDoubleTag( phwi, port )                                      \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_DOUBLE_TAG )

#define PortGetIngressFiltering( phwi, port )                               \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_HI_OFFSET, PORT_INGRESS_FILTER )

#define PortGetInsertTag( phwi, port )                                      \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_INSERT_TAG )

#define PortGetRemoveTag( phwi, port )                                      \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_REMOVE_TAG )
#endif


/* Mirroring */

#ifdef KS_PCI_BUS
#define PortConfigMirrorSniffer( phwi, port, enable )                       \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_SNIFFER, enable )

#define PortConfigMirrorReceive( phwi, port, enable )                       \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_RX, enable )

#define PortConfigMirrorTransmit( phwi, port, enable )                      \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_TX, enable )

#define SwitchConfigMirrorRxAndTx( phwi, enable )                           \
    SwitchConfigSet_PCI( phwi,                                              \
        REG_SWITCH_CTRL_2_HI_OFFSET, SWITCH_MIRROR_RX_TX, enable )

#else
#define PortConfigMirrorSniffer( phwi, port, enable )                       \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_SNIFFER, enable )

#define PortConfigMirrorReceive( phwi, port, enable )                       \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_RX, enable )

#define PortConfigMirrorTransmit( phwi, port, enable )                      \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_MIRROR_TX, enable )

#define SwitchConfigMirrorRxAndTx( phwi, enable )                           \
    SwitchConfigSet_ISA( phwi,                                              \
        REG_SWITCH_CTRL_2_HI_OFFSET, SWITCH_MIRROR_RX_TX, enable )
#endif


/* Priority */

#ifdef KS_PCI_BUS
#define PortConfigDiffServ( phwi, port, enable )                            \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_DIFFSERV_ENABLE, enable )

#define PortConfig802_1P( phwi, port, enable )                              \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_802_1P_ENABLE, enable )

#define PortConfigPriority( phwi, port, enable )                            \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_PRIORITY_ENABLE, enable )

#define PortConfig802_1P_Remapping( phwi, port, enable )                    \
    PortConfigSet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_2_OFFSET, PORT_802_1P_REMAPPING, enable )

#define PortGetDiffServ( phwi, port )                                       \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_DIFFSERV_ENABLE )

#define PortGet802_1P( phwi, port )                                         \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_802_1P_ENABLE )

#define PortGetPriority( phwi, port )                                       \
    PortConfigGet_PCI( phwi, port,                                          \
        REG_PORT_CTRL_1_OFFSET, PORT_PRIORITY_ENABLE )

#else
#define PortConfigDiffServ( phwi, port, enable )                            \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_DIFFSERV_ENABLE, enable )

#define PortConfig802_1P( phwi, port, enable )                              \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_802_1P_ENABLE, enable )

#define PortConfigPriority( phwi, port, enable )                            \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_PRIORITY_ENABLE, enable )

#define PortConfig802_1P_Remapping( phwi, port, enable )                    \
    PortConfigSet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_2_OFFSET, PORT_802_1P_REMAPPING, enable )

#define PortGetDiffServ( phwi, port )                                       \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_DIFFSERV_ENABLE )

#define PortGet802_1P( phwi, port )                                         \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_802_1P_ENABLE )

#define PortGetPriority( phwi, port )                                       \
    PortConfigGet_ISA( phwi, port, REG_PORT_CTRL_BANK,                      \
        REG_PORT_CTRL_1_OFFSET, PORT_PRIORITY_ENABLE )
#endif

/* -------------------------------------------------------------------------- */

/* ks_config.c */

#ifdef KS_PCI_BUS
void SwitchGetAddress_PCI (
#else
void SwitchGetAddress_ISA (
#endif
    PHARDWARE pHardware,
    PUCHAR    MacAddr );

#ifdef KS_PCI_BUS
void SwitchSetAddress_PCI (
#else
void SwitchSetAddress_ISA (
#endif
    PHARDWARE pHardware,
    PUCHAR    MacAddr );

#ifdef KS_PCI_BUS
void SwitchGetLinkStatus_PCI (
#else
void SwitchGetLinkStatus_ISA (
#endif
    PHARDWARE pHardware );

#ifdef KS_PCI_BUS
void SwitchSetLinkSpeed_PCI (
#else
void SwitchSetLinkSpeed_ISA (
#endif
    PHARDWARE pHardware );

#ifdef KS_PCI_BUS
void SwitchSetGlobalControl_PCI (
#else
void SwitchSetGlobalControl_ISA (
#endif
    PHARDWARE pHardware );

#ifdef KS_PCI_BUS
void SwitchRestartAutoNego_PCI (
#else
void SwitchRestartAutoNego_ISA (
#endif
    PHARDWARE pHardware );

#ifdef KS_PCI_BUS
void SwitchEnable_PCI (
#else
void SwitchEnable_ISA (
#endif
    PHARDWARE, BOOLEAN );


#ifdef KS_PCI_BUS
#define SwitchGetAddress  SwitchGetAddress_PCI
#define SwitchSetAddress  SwitchSetAddress_PCI
#define SwitchGetLinkStatus  SwitchGetLinkStatus_PCI
#define SwitchSetLinkSpeed   SwitchSetLinkSpeed_PCI
#define SwitchSetGlobalControl  SwitchSetGlobalControl_PCI
#define SwitchRestartAutoNego   SwitchRestartAutoNego_PCI
#define SwitchEnable  SwitchEnable_PCI

#else
#define SwitchGetAddress  SwitchGetAddress_ISA
#define SwitchSetAddress  SwitchSetAddress_ISA
#define SwitchGetLinkStatus  SwitchGetLinkStatus_ISA
#define SwitchSetLinkSpeed   SwitchSetLinkSpeed_ISA
#define SwitchSetGlobalControl  SwitchSetGlobalControl_ISA
#define SwitchRestartAutoNego   SwitchRestartAutoNego_ISA
#define SwitchEnable  SwitchEnable_ISA
#endif

/* -------------------------------------------------------------------------- */

/* ks_mirror.c */

#ifdef KS_ISA
void SwitchDisableMirrorSniffer_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorSniffer_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorReceive_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorReceive_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorTransmit_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorTransmit_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorRxAndTx_ISA (
    PHARDWARE pHardware );

void SwitchEnableMirrorRxAndTx_ISA (
    PHARDWARE pHardware );

#endif

#ifdef KS_PCI
void SwitchDisableMirrorSniffer_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorSniffer_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorReceive_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorReceive_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorTransmit_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableMirrorTransmit_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchDisableMirrorRxAndTx_PCI (
    PHARDWARE pHardware );

void SwitchEnableMirrorRxAndTx_PCI (
    PHARDWARE pHardware );

#endif

void SwitchInitMirror (
    PHARDWARE pHardware );

#define SwitchDisableMirrorSniffer( phwi, port )                            \
    ( phwi )->m_hwfn->fnSwitchDisableMirrorSniffer( phwi, port )

#define SwitchEnableMirrorSniffer( phwi, port )                             \
    ( phwi )->m_hwfn->fnSwitchEnableMirrorSniffer( phwi, port )

#define SwitchDisableMirrorReceive( phwi, port )                            \
    ( phwi )->m_hwfn->fnSwitchDisableMirrorReceive( phwi, port )

#define SwitchEnableMirrorReceive( phwi, port )                             \
    ( phwi )->m_hwfn->fnSwitchEnableMirrorReceive( phwi, port )

#define SwitchDisableMirrorTransmit( phwi, port )                           \
    ( phwi )->m_hwfn->fnSwitchDisableMirrorTransmit( phwi, port )

#define SwitchEnableMirrorTransmit( phwi, port )                            \
    ( phwi )->m_hwfn->fnSwitchEnableMirrorTransmit( phwi, port )

#define SwitchDisableMirrorRxAndTx( phwi )                                  \
    ( phwi )->m_hwfn->fnSwitchDisableMirrorRxAndTx( phwi )

#define SwitchEnableMirrorRxAndTx( phwi )                                   \
    ( phwi )->m_hwfn->fnSwitchEnableMirrorRxAndTx( phwi )


/* -------------------------------------------------------------------------- */

/* ks_qos.c */

#ifdef KS_ISA
void HardwareConfig_TOS_Priority_ISA (
    PHARDWARE pHardware,
    UCHAR     bTosValue,
    USHORT    wPriority );

void SwitchDisableDiffServ_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnableDiffServ_ISA (
    PHARDWARE pHardware ,
    UCHAR bPort );


void HardwareConfig802_1P_Priorit_ISA (
    PHARDWARE pHardware,
    UCHAR     bTagPriorityValue,
    USHORT    wPriority );

void SwitchDisable802_1P_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnable802_1P_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchDisableDot1pRemapping_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnableDot1pRemapping_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );


void SwitchConfigPortBased_ISA (
    PHARDWARE pHardware,
    UCHAR     bPriority,
    UCHAR     bPort );


void SwitchDisableMultiQueue_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnableMultiQueue_ISA (
    PHARDWARE pHardware,
    UCHAR bPort );

#endif

#ifdef KS_PCI
void HardwareConfig_TOS_Priority_PCI (
    PHARDWARE pHardware,
    UCHAR     bTosValue,
    USHORT    wPriority );

void SwitchDisableDiffServ_PCI (
    PHARDWARE pHardware ,
    UCHAR bPort );

void SwitchEnableDiffServ_PCI (
    PHARDWARE pHardware ,
    UCHAR bPort );


void HardwareConfig802_1P_Priorit_PCI (
    PHARDWARE pHardware,
    UCHAR     bTagPriorityValue,
    USHORT    wPriority );

void SwitchDisable802_1P_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnable802_1P_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchDisableDot1pRemapping_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnableDot1pRemapping_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );


void SwitchConfigPortBased_PCI (
    PHARDWARE pHardware,
    UCHAR     bPriority,
    UCHAR     bPort );


void SwitchDisableMultiQueue_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );

void SwitchEnableMultiQueue_PCI (
    PHARDWARE pHardware,
    UCHAR bPort );

#endif


#define HardwareConfig_TOS_Priority( phwi, p0, p1 )                         \
    ( phwi )->m_hwfn->fnHardwareConfig_TOS_Priority( phwi, p0, p1 )

#define SwitchDisableDiffServ( phwi, p0 )                                   \
    ( phwi )->m_hwfn->fnSwitchDisableDiffServ( phwi, p0 )

#define SwitchEnableDiffServ( phwi, p0 )                                    \
    ( phwi )->m_hwfn->fnSwitchEnableDiffServ( phwi, p0 )

#define HardwareConfig802_1P_Priority( phwi, p0, p1 )                       \
    ( phwi )->m_hwfn->fnHardwareConfig802_1P_Priority( phwi, p0, p1 )

#define SwitchDisable802_1P( phwi, p0 )                                     \
    ( phwi )->m_hwfn->fnSwitchDisable802_1P( phwi, p0 )

#define SwitchEnable802_1P( phwi, p0 )                                      \
    ( phwi )->m_hwfn->fnSwitchEnable802_1P( phwi, p0 )

#define SwitchDisableDot1pRemapping( phwi, p0 )                             \
    ( phwi )->m_hwfn->fnSwitchDisableDot1pRemapping( phwi, p0 )

#define SwitchEnableDot1pRemapping( phwi, p0 )                              \
    ( phwi )->m_hwfn->fnSwitchEnableDot1pRemapping( phwi, p0 )

#define SwitchConfigPortBased( phwi, p0, p1 )                               \
    ( phwi )->m_hwfn->fnSwitchConfigPortBased( phwi, p0, p1 )

void SwitchInitPriority (
    PHARDWARE pHardware );

#define SwitchDisableMultiQueue( phwi, p0 )                                 \
    ( phwi )->m_hwfn->fnSwitchDisableMultiQueue( phwi, p0 )

#define SwitchEnableMultiQueue( phwi, p0 )                                  \
    ( phwi )->m_hwfn->fnSwitchEnableMultiQueue( phwi, p0 )

/* -------------------------------------------------------------------------- */

/* ks_rate.c */

#ifdef KS_ISA
void SwitchDisableBroadcastStorm_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableBroadcastStorm_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );
#endif

#ifdef KS_PCI
void SwitchDisableBroadcastStorm_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnableBroadcastStorm_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );
#endif

void SwitchInitBroadcastStorm (
    PHARDWARE pHardware );

#ifdef KS_ISA
void HardwareConfigBroadcastStorm_ISA (
    PHARDWARE pHardware,
    int       percent );


void SwitchDisablePriorityRate_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnablePriorityRate_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort );
#endif

#ifdef KS_PCI
void HardwareConfigBroadcastStorm_PCI (
    PHARDWARE pHardware,
    int       percent );


void SwitchDisablePriorityRate_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );

void SwitchEnablePriorityRate_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort );
#endif

void SwitchInitPriorityRate (
    PHARDWARE pHardware );

#ifdef KS_ISA
void HardwareConfigRxPriorityRate_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwBytes );

void HardwareConfigTxPriorityRate_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwBytes );
#endif

#ifdef KS_PCI
void HardwareConfigRxPriorityRate_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwBytes );

void HardwareConfigTxPriorityRate_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bPriority,
    ULONG     dwBytes );
#endif

#define SwitchDisableBroadcastStorm( phwi, port )                           \
    ( phwi )->m_hwfn->fnSwitchDisableBroadcastStorm( phwi, port )

#define SwitchEnableBroadcastStorm( phwi, port )                            \
    ( phwi )->m_hwfn->fnSwitchEnableBroadcastStorm( phwi, port )

#define HardwareConfigBroadcastStorm( phwi, percent )                       \
    ( phwi )->m_hwfn->fnHardwareConfigBroadcastStorm( phwi, percent )


#define SwitchDisablePriorityRate( phwi, port )                             \
    ( phwi )->m_hwfn->fnSwitchDisablePriorityRate( phwi, port )

#define SwitchEnablePriorityRate( phwi, port )                              \
    ( phwi )->m_hwfn->fnSwitchEnablePriorityRate( phwi, port )

#define HardwareConfigRxPriorityRate( phwi, port, priority, bytes )         \
    ( phwi )->m_hwfn->fnHardwareConfigRxPriorityRate( phwi, port,           \
        priority, bytes )

#define HardwareConfigTxPriorityRate( phwi, port, priority, bytes )         \
    ( phwi )->m_hwfn->fnHardwareConfigTxPriorityRate( phwi, port,           \
        priority, bytes )

/* -------------------------------------------------------------------------- */

/* ks_stp.c */

enum {
    STP_STATE_DISABLED = 0,
    STP_STATE_LISTENING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_BLOCKED
};


#ifdef KS_ISA
void PortSet_STP_State_ISA (
    PHARDWARE pHardware,
    UCHAR     bPort,
    int       nState );
#endif

#ifdef KS_PCI
void PortSet_STP_State_PCI (
    PHARDWARE pHardware,
    UCHAR     bPort,
    int       nState );
#endif

#define PortSet_STP_State( phwi, port, state )                              \
    ( phwi )->m_hwfn->fnPortSet_STP_State( phwi, port, state )

void HardwareInit_STP (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/* ks_table.c */

#ifdef KS_ISA
BOOLEAN SwitchReadDynMacTable_ISA (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    PUCHAR    pbFID,
    PUCHAR    pbSrcPort,
    PUCHAR    pbTimestamp,
    PUSHORT   pwEntries );

#define SwitchReadDynMacTable     SwitchReadDynMacTable_ISA

#endif

#ifdef KS_PCI
BOOLEAN SwitchReadDynMacTable_PCI (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    PUCHAR    pbFID,
    PUCHAR    pbSrcPort,
    PUCHAR    pbTimestamp,
    PUSHORT   pwEntries );

#define SwitchReadDynMacTable     SwitchReadDynMacTable_PCI

#endif

BOOLEAN SwitchReadStaticMacTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    PUCHAR    pbPorts,
    PBOOLEAN  pfOverride,
    PBOOLEAN  pfUseFID,
    PUCHAR    pbFID );

void SwitchWriteStaticMacTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUCHAR    MacAddr,
    UCHAR     bPorts,
    BOOLEAN   fOverride,
    BOOLEAN   fValid,
    BOOLEAN   fUseFID,
    UCHAR     bFID );

BOOLEAN SwitchReadVlanTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    PUSHORT   pwVID,
    PUCHAR    pbFID,
    PUCHAR    pbMember );

void SwitchWriteVlanTable (
    PHARDWARE pHardware,
    USHORT    wAddr,
    USHORT    wVID,
    UCHAR     bFID,
    UCHAR     bMember,
    BOOLEAN   fValid );

#ifdef KS_ISA
void PortReadMIBCounter_ISA (
    PHARDWARE  pHardware,
    UCHAR      bPort,
    USHORT     wAddr,
    PULONGLONG pqData );

void PortReadMIBPacket_ISA (
    PHARDWARE  pHardware,
    UCHAR      bPort,
    PULONG     pdwCurrent,
    PULONGLONG pqData );
#endif

#ifdef KS_PCI
void PortReadMIBCounter_PCI (
    PHARDWARE  pHardware,
    UCHAR      bPort,
    USHORT     wAddr,
    PULONGLONG pqData );

void PortReadMIBPacket_PCI (
    PHARDWARE  pHardware,
    UCHAR      bPort,
    PULONG     pdwCurrent,
    PULONGLONG pqData );
#endif

#define PortReadMIBCounter( phwi, port, counter, data )                     \
    ( phwi )->m_hwfn->fnPortReadMIBCounter( phwi, port, counter, data )

#define PortReadMIBPacket( phwi, port, counter, data )                      \
    ( phwi )->m_hwfn->fnPortReadMIBPacket( phwi, port, counter, data )


void PortInitCounters (
    PHARDWARE pHardware,
    UCHAR     bPort );

int PortReadCounters (
    PHARDWARE pHardware,
    UCHAR     bPort );

/* -------------------------------------------------------------------------- */

/* ks_vlan.c */

void SwitchDisableVlan (
    PHARDWARE pHardware );

#ifdef KS_ISA
void SwitchEnableVlan_ISA (
    PHARDWARE pHardware );
#endif

#ifdef KS_PCI
void SwitchEnableVlan_PCI (
    PHARDWARE pHardware );
#endif

#define SwitchEnableVlan( phwi )                                            \
    ( phwi )->m_hwfn->fnSwitchEnableVlan( phwi )

void SwitchInitVlan (
    PHARDWARE pHardware );

void HardwareConfigDefaultVID (
    PHARDWARE pHardware,
    UCHAR     bPort,
    USHORT    wVID );

void HardwareConfigPortBaseVlan
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    UCHAR     bMember
);

void SwitchVlanConfigDiscardNonVID
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
);

void SwitchVlanConfigIngressFiltering
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
);

void SwitchVlanConfigInsertTag
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
);

void SwitchVlanConfigRemoveTag
(
    PHARDWARE pHardware,
    UCHAR     bPort,
    BOOLEAN   fSet
);

/* -------------------------------------------------------------------------- */

#endif
