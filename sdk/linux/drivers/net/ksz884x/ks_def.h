/* ---------------------------------------------------------------------------
          Copyright (c) 2004-2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_def.h - KS884X switch definitions.

    Author  Date      Version  Description
    THa     02/28/06           Do not use HW_WRITE_BYTE because of limitation of
                               some hardware platforms.
    THa     02/23/06           Removed KS884X_HW conditional.
    PCD     04/29/05  0.1.3    Correct reset device function, and check device ID for device.
    THa     02/13/04           Created file.
    THa     10/05/04           Updated for PCI version.
    THa     10/14/04           Updated with latest specs.
   ---------------------------------------------------------------------------
*/


#ifndef __KS_DEF_H
#define __KS_DEF_H


#if 0
#define DEVELOP
#endif

/* -------------------------------------------------------------------------- */

#define TO_LO_BYTE( x )  (( UCHAR )(( x ) >> 8 ))
#define TO_HI_BYTE( x )  (( USHORT )(( x ) << 8 ))


/* KS884X byte registers */

#define REG_FAMILY_ID               0x88

/* SIDER */
#define REG_CHIP_ID_41              0x8810
#define REG_CHIP_ID_42              0x8800

#define SWITCH_CHIP_ID_MASK_41      0xFF10
#define SWITCH_CHIP_ID_MASK         0xFFF0
#define SWITCH_CHIP_ID_SHIFT        4
#define SWITCH_REVISION_MASK        0x000E
#define SWITCH_START                0x01

#define CHIP_IP_41_ISA              0x8810
#define CHIP_IP_42_ISA              0x8800
#define CHIP_IP_61_ISA              0x8890
#define CHIP_IP_62_ISA              0x8880

#define CHIP_IP_41_PCI              0x8850
#define CHIP_IP_42_PCI              0x8840
#define CHIP_IP_61_PCI              0x88D0
#define CHIP_IP_62_PCI              0x88C0

/* SGCR1 */
#define REG_SWITCH_CTRL_1           0x02

#define SWITCH_NEW_BACKOFF          0x80
#define SWITCH_802_1P_MASK          0x70
#define SWITCH_802_1P_BASE          7
#define SWITCH_802_1P_SHIFT         4
#define SWITCH_PASS_PAUSE           0x08
#define SWITCH_BUFFER_SHARE         0x04
#define SWITCH_RECEIVE_PAUSE        0x02
#define SWITCH_LINK_AGE             0x01

#define REG_SWITCH_CTRL_1_HI        0x03

#define SWITCH_PASS_ALL             0x80
#define SWITCH_TX_FLOW_CTRL         0x20
#define SWITCH_RX_FLOW_CTRL         0x10
#define SWITCH_CHECK_LENGTH         0x08
#define SWITCH_AGING_ENABLE         0x04
#define SWITCH_FAST_AGE             0x02
#define SWITCH_AGGR_BACKOFF         0x01

/* SGCR2 */
#define REG_SWITCH_CTRL_2           0x04

#define UNICAST_VLAN_BOUNDARY       0x80
#define MULTICAST_STORM_DISABLE     0x40
#define SWITCH_BACK_PRESSURE        0x20
#define FAIR_FLOW_CTRL              0x10
#define NO_EXC_COLLISION_DROP       0x08
#define SWITCH_HUGE_PACKET          0x04
#define SWITCH_LEGAL_PACKET         0x02
#define SWITCH_BUF_RESERVE          0x01

#define REG_SWITCH_CTRL_2_HI        0x05

#define SWITCH_VLAN_ENABLE          0x80
#define SWITCH_IGMP_SNOOP           0x40
#define PRIORITY_SCHEME_SELECT      0x0C
#define PRIORITY_RATIO_HIGH         0x00
#define PRIORITY_RATIO_10           0x04
#define PRIORITY_RATIO_5            0x08
#define PRIORITY_RATIO_2            0x0C
#define SWITCH_MIRROR_RX_TX         0x01

/* SGCR3 */
#define REG_SWITCH_CTRL_3           0x06

#define SWITCH_REPEATER             0x80
#define SWITCH_HALF_DUPLEX          0x40
#define SWITCH_FLOW_CTRL            0x20
#define SWITCH_10_MBIT              0x10
#define SWITCH_REPLACE_VID          0x08
#define BROADCAST_STORM_RATE_HI     0x07

#define REG_SWITCH_CTRL_3_HI        0x07

#define BROADCAST_STORM_RATE_LO     0xFF
#define BROADCAST_STORM_RATE        0x07FF

/* SGCR4 */
#define REG_SWITCH_CTRL_4           0x08

/* SGCR5 */
#define REG_SWITCH_CTRL_5           0x0A

#define PHY_POWER_SAVE              0x40
#define CRC_DROP                    0x20
#define LED_MODE                    0x02
#define TPID_MODE_ENABLE            0x01

/* SGCR6 */
#define REG_SWITCH_CTRL_6           0x0C

#define SWITCH_802_1P_MAP_MASK      3
#define SWITCH_802_1P_MAP_SHIFT     2

/* SGCR7 */
#define REG_SWITCH_CTRL_7           0x0E


/* P1CR1 */
#define REG_PORT_1_CTRL_1           0x10
/* P2CR1 */
#define REG_PORT_2_CTRL_1           0x20
/* P3CR1 */
#define REG_PORT_3_CTRL_1           0x30

#define PORT_BROADCAST_STORM        0x80
#define PORT_DIFFSERV_ENABLE        0x40
#define PORT_802_1P_ENABLE          0x20
#define PORT_BASED_PRIORITY_MASK    0x18
#define PORT_BASED_PRIORITY_BASE    0x03
#define PORT_BASED_PRIORITY_SHIFT   3
#define PORT_PORT_PRIORITY_0        0x00
#define PORT_PORT_PRIORITY_1        0x08
#define PORT_PORT_PRIORITY_2        0x10
#define PORT_PORT_PRIORITY_3        0x18
#define PORT_INSERT_TAG             0x04
#define PORT_REMOVE_TAG             0x02
#define PORT_PRIORITY_ENABLE        0x01

/* P1CR2 */
#define REG_PORT_1_CTRL_2           0x11
/* P2CR2 */
#define REG_PORT_2_CTRL_2           0x21
/* P3CR2 */
#define REG_PORT_3_CTRL_2           0x31

#define PORT_MIRROR_SNIFFER         0x80
#define PORT_MIRROR_RX              0x40
#define PORT_MIRROR_TX              0x20
#define PORT_DOUBLE_TAG             0x10
#define PORT_802_1P_REMAPPING       0x08
#define PORT_VLAN_MEMBERSHIP        0x07

#define REG_PORT_1_CTRL_2_HI        0x12
#define REG_PORT_2_CTRL_2_HI        0x22
#define REG_PORT_3_CTRL_2_HI        0x32

#define PORT_REMOTE_LOOPBACK        0x80
#define PORT_INGRESS_FILTER         0x40
#define PORT_DISCARD_NON_VID        0x20
#define PORT_FORCE_FLOW_CTRL        0x10
#define PORT_BACK_PRESSURE          0x08
#define PORT_TX_ENABLE              0x04
#define PORT_RX_ENABLE              0x02
#define PORT_LEARN_DISABLE          0x01

/* P1VIDCR */
#define REG_PORT_1_CTRL_VID         0x13
/* P2VIDCR */
#define REG_PORT_2_CTRL_VID         0x23
/* P3VIDCR */
#define REG_PORT_3_CTRL_VID         0x33

#define PORT_DEFAULT_VID            0xFFFF

/* P1CR3 */
#define REG_PORT_1_CTRL_3           0x15
/* P2CR3 */
#define REG_PORT_2_CTRL_3           0x25
/* P3CR3 */
#define REG_PORT_3_CTRL_3           0x35

#define PORT_USER_PRIORITY_CEILING  0x10
#define PORT_INGRESS_LIMIT_MODE     0x0C
#define PORT_INGRESS_ALL            0x00
#define PORT_INGRESS_UNICAST        0x04
#define PORT_INGRESS_MULTICAST      0x08
#define PORT_INGRESS_BROADCAST      0x0C
#define PORT_COUNT_IFG              0x02
#define PORT_COUNT_PREAMBLE         0x01

/* P1IRCR */
#define REG_PORT_1_IN_RATE          0x16
/* P1ERCR */
#define REG_PORT_1_OUT_RATE         0x18
/* P2IRCR */
#define REG_PORT_2_IN_RATE          0x26
/* P2ERCR */
#define REG_PORT_2_OUT_RATE         0x28
/* P3IRCR */
#define REG_PORT_3_IN_RATE          0x36
/* P3ERCR */
#define REG_PORT_3_OUT_RATE         0x38

#define PORT_PRIORITY_RATE          0x0F
#define PORT_PRIORITY_RATE_SHIFT    4


#define REG_PORT_1_LINK_MD_CTRL     0x1A
/* P1SCSLMD */
#define REG_PORT_1_LINK_MD_RESULT   0x1B
#define REG_PORT_2_LINK_MD_CTRL     0x2A
/* P2SCSLMD */
#define REG_PORT_2_LINK_MD_RESULT   0x2B

#define PORT_CABLE_DIAG_RESULT      0x60
#define PORT_CABLE_STAT_NORMAL      0x00
#define PORT_CABLE_STAT_OPEN        0x20
#define PORT_CABLE_STAT_SHORT       0x40
#define PORT_CABLE_STAT_FAILED      0x60
#define PORT_START_CABLE_DIAG       0x10
#define PORT_FORCE_LINK             0x08
#define PORT_POWER_SAVING           0x04
#define PORT_PHY_REMOTE_LOOPBACK    0x02
#define PORT_CABLE_FAULT_COUNTER_H  0x01

#define PORT_CABLE_FAULT_COUNTER_L  0xFF
#define PORT_CABLE_FAULT_COUNTER    0x1FF

/* P1CR4 */
#define REG_PORT_1_CTRL_4           0x1C
/* P2CR4 */
#define REG_PORT_2_CTRL_4           0x2C

#define PORT_AUTO_NEG_ENABLE        0x80
#define PORT_FORCE_100_MBIT         0x40
#define PORT_FORCE_FULL_DUPLEX      0x20
#define PORT_AUTO_NEG_SYM_PAUSE     0x10
#define PORT_AUTO_NEG_100BTX_FD     0x08
#define PORT_AUTO_NEG_100BTX        0x04
#define PORT_AUTO_NEG_10BT_FD       0x02
#define PORT_AUTO_NEG_10BT          0x01

#define REG_PORT_1_CTRL_4_HI        0x1D
#define REG_PORT_2_CTRL_4_HI        0x2D

#define PORT_LED_OFF                0x80
#define PORT_TX_DISABLE             0x40
#define PORT_AUTO_NEG_RESTART       0x20
#define PORT_REMOTE_FAULT_DISABLE   0x10
#define PORT_POWER_DOWN             0x08
#define PORT_AUTO_MDIX_DISABLE      0x04
#define PORT_FORCE_MDIX             0x02
#define PORT_LOOPBACK               0x01

/* P1SR */
#define REG_PORT_1_STATUS           0x1E
/* P2SR */
#define REG_PORT_2_STATUS           0x2E

#define PORT_MDIX_STATUS            0x80
#define PORT_AUTO_NEG_COMPLETE      0x40
#define PORT_STATUS_LINK_GOOD       0x20
#define PORT_REMOTE_SYM_PAUSE       0x10
#define PORT_REMOTE_100BTX_FD       0x08
#define PORT_REMOTE_100BTX          0x04
#define PORT_REMOTE_10BT_FD         0x02
#define PORT_REMOTE_10BT            0x01

#define REG_PORT_1_STATUS_HI        0x1F
#define REG_PORT_2_STATUS_HI        0x2F
#define REG_PORT_3_STATUS_HI        0x3F

#define PORT_HP_MDIX                0x80
#define PORT_REVERSED_POLARITY      0x20
#define PORT_RX_FLOW_CTRL           0x10
#define PORT_TX_FLOW_CTRL           0x08
#define PORT_STAT_SPEED_100MBIT     0x04
#define PORT_STAT_FULL_DUPLEX       0x02
#define PORT_REMOTE_FAULT           0x01


#define REG_MEDIA_CONV_PHY_ADDR     0x40


/* TOSR1 */
#define REG_TOS_PRIORITY_CTRL_1     0x60
/* TOSR2 */
#define REG_TOS_PRIORITY_CTRL_2     0x62
/* TOSR3 */
#define REG_TOS_PRIORITY_CTRL_3     0x64
/* TOSR4 */
#define REG_TOS_PRIORITY_CTRL_4     0x66
/* TOSR5 */
#define REG_TOS_PRIORITY_CTRL_5     0x68
/* TOSR6 */
#define REG_TOS_PRIORITY_CTRL_6     0x6A
/* TOSR7 */
#define REG_TOS_PRIORITY_CTRL_7     0x6C
/* TOSR8 */
#define REG_TOS_PRIORITY_CTRL_8     0x6E

#define TOS_PRIORITY_MASK           3
#define TOS_PRIORITY_SHIFT          2


/* MACAR1 */
#define REG_MAC_ADDR_0              0x70
#define REG_MAC_ADDR_1              0x71
/* MACAR2 */
#define REG_MAC_ADDR_2              0x72
#define REG_MAC_ADDR_3              0x73
/* MACAR3 */
#define REG_MAC_ADDR_4              0x74
#define REG_MAC_ADDR_5              0x75


#define REG_USER_DEFINED_0          0x76
#define REG_USER_DEFINED_1          0x77
#define REG_USER_DEFINED_2          0x78


/* IACR */
#define REG_INDIRECT_ACCESS_CTRL_0  0x79
#define REG_INDIRECT_ACCESS_CTRL_1  0x7A

/* IADR1 */
#define REG_INDIRECT_DATA_8         0x7B
/* IADR3 */
#define REG_INDIRECT_DATA_7         0x7C
#define REG_INDIRECT_DATA_6         0x7D
/* IADR2 */
#define REG_INDIRECT_DATA_5         0x7E
#define REG_INDIRECT_DATA_4         0x7F
/* IADR5 */
#define REG_INDIRECT_DATA_3         0x80
#define REG_INDIRECT_DATA_2         0x81
/* IADR4 */
#define REG_INDIRECT_DATA_1         0x82
#define REG_INDIRECT_DATA_0         0x83


/* PHAR */


/* P1MBCR */
/* P2MBCR */
#define PHY_REG_CTRL                0

#define PHY_RESET                   0x8000
#define PHY_LOOPBACK                0x4000
#define PHY_SPEED_100MBIT           0x2000
#define PHY_AUTO_NEG_ENABLE         0x1000
#define PHY_POWER_DOWN              0x0800
#define PHY_MII_DISABLE             0x0400
#define PHY_AUTO_NEG_RESTART        0x0200
#define PHY_FULL_DUPLEX             0x0100
#define PHY_COLLISION_TEST          0x0080
#define PHY_HP_MDIX                 0x0020
#define PHY_FORCE_MDIX              0x0010
#define PHY_AUTO_MDIX_DISABLE       0x0008
#define PHY_REMOTE_FAULT_DISABLE    0x0004
#define PHY_TRANSMIT_DISABLE        0x0002
#define PHY_LED_DISABLE             0x0001

/* P1MBSR */
/* P2MBSR */
#define PHY_REG_STATUS              1

#define PHY_100BT4_CAPABLE          0x8000
#define PHY_100BTX_FD_CAPABLE       0x4000
#define PHY_100BTX_CAPABLE          0x2000
#define PHY_10BT_FD_CAPABLE         0x1000
#define PHY_10BT_CAPABLE            0x0800
#define PHY_MII_SUPPRESS_CAPABLE    0x0040
#define PHY_AUTO_NEG_ACKNOWLEDGE    0x0020
#define PHY_REMOTE_FAULT            0x0010
#define PHY_AUTO_NEG_CAPABLE        0x0008
#define PHY_LINK_STATUS             0x0004
#define PHY_JABBER_DETECT           0x0002
#define PHY_EXTENDED_CAPABILITY     0x0001

/* PHY1ILR */
/* PHY1IHR */
/* PHY2ILR */
/* PHY2IHR */
#define PHY_REG_ID_1                2
#define PHY_REG_ID_2                3

/* P1ANAR */
/* P2ANAR */
#define PHY_REG_AUTO_NEGOTIATION    4

#define PHY_AUTO_NEG_NEXT_PAGE      0x8000
#define PHY_AUTO_NEG_REMOTE_FAULT   0x2000
#if 0
#define PHY_AUTO_NEG_ASYM_PAUSE     0x0800
#endif
#define PHY_AUTO_NEG_SYM_PAUSE      0x0400
#define PHY_AUTO_NEG_100BT4         0x0200
#define PHY_AUTO_NEG_100BTX_FD      0x0100
#define PHY_AUTO_NEG_100BTX         0x0080
#define PHY_AUTO_NEG_10BT_FD        0x0040
#define PHY_AUTO_NEG_10BT           0x0020
#define PHY_AUTO_NEG_SELECTOR       0x001F
#define PHY_AUTO_NEG_802_3          0x0001

/* P1ANLPR */
/* P2ANLPR */
#define PHY_REG_REMOTE_CAPABILITY   5

#define PHY_REMOTE_NEXT_PAGE        0x8000
#define PHY_REMOTE_ACKNOWLEDGE      0x4000
#define PHY_REMOTE_REMOTE_FAULT     0x2000
#define PHY_REMOTE_SYM_PAUSE        0x0400
#define PHY_REMOTE_100BTX_FD        0x0100
#define PHY_REMOTE_100BTX           0x0080
#define PHY_REMOTE_10BT_FD          0x0040
#define PHY_REMOTE_10BT             0x0020

/* P1VCT */
/* P2VCT */
#define PHY_REG_LINK_MD             29

#define PHY_START_CABLE_DIAG        0x8000
#define PHY_CABLE_DIAG_RESULT       0x6000
#define PHY_CABLE_STAT_NORMAL       0x0000
#define PHY_CABLE_STAT_OPEN         0x2000
#define PHY_CABLE_STAT_SHORT        0x4000
#define PHY_CABLE_STAT_FAILED       0x6000
#define PHY_CABLE_10M_SHORT         0x1000
#define PHY_CABLE_FAULT_COUNTER     0x01FF

/* P1PHYCTRL */
/* P2PHYCTRL */
#define PHY_REG_PHY_CTRL            30

#define PHY_STAT_REVERSED_POLARITY  0x0020
#define PHY_STAT_MDIX               0x0010
#define PHY_FORCE_LINK              0x0008
#define PHY_POWER_SAVING            0x0004
#define PHY_REMOTE_LOOPBACK         0x0002

/* -------------------------------------------------------------------------- */

/* KS884X ISA registers */

#ifdef KS_ISA_BUS
#define REG_SWITCH_CTRL_BANK        32

#define REG_SIDER_BANK              REG_SWITCH_CTRL_BANK
#define REG_SIDER_OFFSET            0x00

#define REG_CHIP_ID_OFFSET          REG_SIDER_OFFSET
#define REG_FAMILY_ID_OFFSET        ( REG_CHIP_ID_OFFSET + 1 )

#define REG_SGCR1_BANK              REG_SWITCH_CTRL_BANK
#define REG_SGCR1_OFFSET            0x02

#define REG_SWITCH_CTRL_1_OFFSET    REG_SGCR1_OFFSET
#define REG_SWITCH_CTRL_1_HI_OFFSET ( REG_SWITCH_CTRL_1_OFFSET + 1 )

#define REG_SGCR2_BANK              REG_SWTICH_CTRL_BANK
#define REG_SGCR2_OFFSET            0x04

#define REG_SWITCH_CTRL_2_OFFSET    REG_SGCR2_OFFSET
#define REG_SWITCH_CTRL_2_HI_OFFSET ( REG_SWITCH_CTRL_2_OFFSET + 1 )

#define REG_SGCR3_BANK              REG_SWITCH_CTRL_BANK
#define REG_SGCR3_OFFSET            0x06

#define REG_SWITCH_CTRL_3_OFFSET    REG_SGCR3_OFFSET
#define REG_SWITCH_CTRL_3_HI_OFFSET ( REG_SWITCH_CTRL_3_OFFSET + 1 )

#define REG_SGCR4_BANK              REG_SWITCH_CTRL_BANK
#define REG_SGCR4_OFFSET            0x08

#define REG_SWITCH_CTRL_4_OFFSET    REG_SGCR4_OFFSET

#define REG_SGCR5_BANK              REG_SWITCH_CTRL_BANK
#define REG_SGCR5_OFFSET            0x0A

#define REG_SWITCH_CTRL_5_OFFSET    REG_SGCR5_OFFSET


#define REG_SWITCH_802_1P_BANK      33

#define REG_SGCR6_BANK              REG_SWITCH_802_1P_BANK
#define REG_SGCR6_OFFSET            0x00

#define REG_SWITCH_CTRL_6_OFFSET    REG_SGCR6_OFFSET


#define REG_PHAR_BANK               34
#define REG_PHAR_OFFSET             0x00

#define REG_LBS21R_OFFSET           0x04

#define REG_LBRCTCER_OFFSET         0x08

#define REG_LBRCGR_OFFSET           0x0A


#define REG_CSCR_BANK               35
#define REG_CSCR_OFFSET             0x00

#define REG_PSWIR_OFFSET            0x02

#define REG_PC21R_OFFSET            0x04

#define REG_PC3R_OFFSET             0x06

#define REG_VMCRTCR_OFFSET          0x08


#define REG_S58R_BANK               36
#define REG_S58R_OFFSET             0x00

#define REG_MVI21R_OFFSET           0x04

#define REG_MM1V31R_OFFSET          0x06

#define REG_MMI32R_OFFSET           0x08


#define REG_LPVI21R_BANK            37
#define REG_LPVI21R_OFFSET          0x00

#define REG_LPM1V31R_OFFSET         0x04


#define REG_CSSR_BANK               38
#define REG_CSSR_OFFSET             0x00

#define REG_ASCTR_OFFSET            0x04

#define REG_MS21R_OFFSET            0x08

#define REG_LPS21R_OFFSET           0x0A


#define REG_MAC_ADDR_BANK           39

#define REG_MACAR1_BANK             REG_MAC_ADDR_BANK
#define REG_MACAR1_OFFSET           0x00
#define REG_MACAR2_OFFSET           0x02
#define REG_MACAR3_OFFSET           0x04

#define REG_MAC_ADDR_0_OFFSET       REG_MACAR1_OFFSET
#define REG_MAC_ADDR_1_OFFSET       ( REG_MAC_ADDR_0_OFFSET + 1 )
#define REG_MAC_ADDR_2_OFFSET       REG_MACAR2_OFFSET
#define REG_MAC_ADDR_3_OFFSET       ( REG_MAC_ADDR_2_OFFSET + 1 )
#define REG_MAC_ADDR_4_OFFSET       REG_MACAR3_OFFSET
#define REG_MAC_ADDR_5_OFFSET       ( REG_MAC_ADDR_4_OFFSET + 1 )


#define REG_TOS_PRIORITY_BANK       40
#define REG_TOS_PRIORITY_2_BANK     ( REG_TOS_PRIORITY_BANK + 1 )

#define REG_TOSR1_BANK              REG_TOS_PRIORITY_BANK
#define REG_TOSR1_OFFSET            0x00
#define REG_TOSR2_OFFSET            0x02
#define REG_TOSR3_OFFSET            0x04
#define REG_TOSR4_OFFSET            0x06
#define REG_TOSR5_OFFSET            0x08
#define REG_TOSR6_OFFSET            0x0A

#define REG_TOSR7_BANK              REG_TOS_PRIORITY_2_BANK
#define REG_TOSR7_OFFSET            0x00
#define REG_TOSR8_OFFSET            0x02

#define REG_TOS_1_OFFSET            REG_TOSR1_OFFSET
#define REG_TOS_2_OFFSET            REG_TOSR2_OFFSET
#define REG_TOS_3_OFFSET            REG_TOSR3_OFFSET
#define REG_TOS_4_OFFSET            REG_TOSR4_OFFSET
#define REG_TOS_5_OFFSET            REG_TOSR5_OFFSET
#define REG_TOS_6_OFFSET            REG_TOSR6_OFFSET

#define REG_TOS_7_OFFSET            REG_TOSR7_OFFSET
#define REG_TOS_8_OFFSET            REG_TOSR8_OFFSET


#define REG_IND_ACC_CTRL_BANK       42

#define REG_IACR_BANK               REG_IND_ACC_CTRL_BANK
#define REG_IACR_OFFSET             0x00

#define REG_IADR1_BANK              REG_IND_ACC_CTRL_BANK
#define REG_IADR1_OFFSET            0x02

#define REG_IADR2_BANK              REG_IND_ACC_CTRL_BANK
#define REG_IADR2_OFFSET            0x04

#define REG_IADR3_BANK              REG_IND_ACC_CTRL_BANK
#define REG_IADR3_OFFSET            0x06

#define REG_IADR4_BANK              REG_IND_ACC_CTRL_BANK
#define REG_IADR4_OFFSET            0x08

#define REG_IADR5_BANK              REG_IND_ACC_CTRL_BANK
#define REG_IADR5_OFFSET            0x0A

#define REG_ACC_CTRL_INDEX_OFFSET   REG_IACR_OFFSET
#define REG_ACC_CTRL_SEL_OFFSET     ( REG_ACC_CTRL_INDEX_OFFSET + 1 )

#define REG_ACC_DATA_0_OFFSET       REG_IADR4_OFFSET
#define REG_ACC_DATA_1_OFFSET       ( REG_ACC_DATA_0_OFFSET + 1 )
#define REG_ACC_DATA_2_OFFSET       REG_IADR5_OFFSET
#define REG_ACC_DATA_3_OFFSET       ( REG_ACC_DATA_2_OFFSET + 1 )
#define REG_ACC_DATA_4_OFFSET       REG_IADR2_OFFSET
#define REG_ACC_DATA_5_OFFSET       ( REG_ACC_DATA_4_OFFSET + 1 )
#define REG_ACC_DATA_6_OFFSET       REG_IADR3_OFFSET
#define REG_ACC_DATA_7_OFFSET       ( REG_ACC_DATA_6_OFFSET + 1 )
#define REG_ACC_DATA_8_OFFSET       REG_IADR1_OFFSET


#define REG_PHY_1_CTRL_BANK         45
#define REG_PHY_2_CTRL_BANK         46

#define PHY_BANK_INTERVAL           \
    ( REG_PHY_2_CTRL_BANK - REG_PHY_1_CTRL_BANK )

#define REG_P1MBCR_BANK             REG_PHY_1_CTRL_BANK
#define REG_P1MBCR_OFFSET           0x00

#define REG_P1MBSR_BANK             REG_PHY_1_CTRL_BANK
#define REG_P1MBSR_OFFSET           0x02

#define REG_PHY1ILR_BANK            REG_PHY_1_CTRL_BANK
#define REG_PHY1ILR_OFFSET          0x04

#define REG_PHY1LHR_BANK            REG_PHY_1_CTRL_BANK
#define REG_PHY1LHR_OFFSET          0x06

#define REG_P1ANAR_BANK             REG_PHY_1_CTRL_BANK
#define REG_P1ANAR_OFFSET           0x08

#define REG_P1ANLPR_BANK            REG_PHY_1_CTRL_BANK
#define REG_P1ANLPR_OFFSET          0x0A

#define REG_P2MBCR_BANK             REG_PHY_2_CTRL_BANK
#define REG_P2MBCR_OFFSET           0x00

#define REG_P2MBSR_BANK             REG_PHY_2_CTRL_BANK
#define REG_P2MBSR_OFFSET           0x02

#define REG_PHY2ILR_BANK            REG_PHY_2_CTRL_BANK
#define REG_PHY2ILR_OFFSET          0x04

#define REG_PHY2LHR_BANK            REG_PHY_2_CTRL_BANK
#define REG_PHY2LHR_OFFSET          0x06

#define REG_P2ANAR_BANK             REG_PHY_2_CTRL_BANK
#define REG_P2ANAR_OFFSET           0x08

#define REG_P2ANLPR_BANK            REG_PHY_2_CTRL_BANK
#define REG_P2ANLPR_OFFSET          0x0A


#define REG_PHY_SPECIAL_BANK        47

#define REG_P1VCT_BANK              REG_PHY_SPECIAL_BANK
#define REG_P1VCT_OFFSET            0x00

#define REG_P1PHYCTRL_BANK          REG_PHY_SPECIAL_BANK
#define REG_P1PHYCTRL_OFFSET        0x02

#define REG_P2VCT_BANK              REG_PHY_SPECIAL_BANK
#define REG_P2VCT_OFFSET            0x04

#define REG_P2PHYCTRL_BANK          REG_PHY_SPECIAL_BANK
#define REG_P2PHYCTRL_OFFSET        0x06


#define REG_PHY_CTRL_OFFSET         0x00
#define REG_PHY_STATUS_OFFSET       0x02
#define REG_PHY_ID_1_OFFSET         0x04
#define REG_PHY_ID_2_OFFSET         0x06
#define REG_PHY_AUTO_NEG_OFFSET     0x08
#define REG_PHY_REMOTE_CAP_OFFSET   0x0A

#define REG_PHY_LINK_MD_1_OFFSET    0x00
#define REG_PHY_PHY_CTRL_1_OFFSET   0x02
#define REG_PHY_LINK_MD_2_OFFSET    0x04
#define REG_PHY_PHY_CTRL_2_OFFSET   0x06

#define PHY_SPECIAL_INTERVAL        \
    ( REG_PHY_LINK_MD_2_OFFSET - REG_PHY_LINK_MD_1_OFFSET )


#define REG_PORT_1_CTRL_BANK        48
#define REG_PORT_1_LINK_CTRL_BANK   49
#define REG_PORT_1_LINK_STATUS_BANK 49

#define REG_PORT_2_CTRL_BANK        50
#define REG_PORT_2_LINK_CTRL_BANK   51
#define REG_PORT_2_LINK_STATUS_BANK 51

#define REG_PORT_3_CTRL_BANK        52
#define REG_PORT_3_LINK_CTRL_BANK   53
#define REG_PORT_3_LINK_STATUS_BANK 53

/* Port# Control Register */
#define REG_PORT_CTRL_BANK          REG_PORT_1_CTRL_BANK

/* Port# Link Control Register */
#define REG_PORT_LINK_CTRL_BANK     REG_PORT_1_LINK_CTRL_BANK
#define REG_PORT_LINK_STATUS_BANK   REG_PORT_1_LINK_STATUS_BANK

#define PORT_BANK_INTERVAL          \
    ( REG_PORT_2_CTRL_BANK - REG_PORT_1_CTRL_BANK )

#define REG_P1CR1_BANK              REG_PORT_1_CTRL_BANK
#define REG_P1CR1_OFFSET            0x00

#define REG_P1CR2_BANK              REG_PORT_1_CTRL_BANK
#define REG_P1CR2_OFFSET            0x02

#define REG_P1VIDCR_BANK            REG_PORT_1_CTRL_BANK
#define REG_P1VIDCR_OFFSET          0x04

#define REG_P1CR3_BANK              REG_PORT_1_CTRL_BANK
#define REG_P1CR3_OFFSET            0x06

#define REG_P1IRCR_BANK             REG_PORT_1_CTRL_BANK
#define REG_P1IRCR_OFFSET           0x08

#define REG_P1ERCR_BANK             REG_PORT_1_CTRL_BANK
#define REG_P1ERCR_OFFSET           0x0A

#define REG_P1SCSLMD_BANK           REG_PORT_1_LINK_CTRL_BANK
#define REG_P1SCSLMD_OFFSET         0x00

#define REG_P1CR4_BANK              REG_PORT_1_LINK_CTRL_BANK
#define REG_P1CR4_OFFSET            0x02

#define REG_P1SR_BANK               REG_PORT_1_LINK_CTRL_BANK
#define REG_P1SR_OFFSET             0x04

#define REG_P2CR1_BANK              REG_PORT_2_CTRL_BANK
#define REG_P2CR1_OFFSET            0x00

#define REG_P2CR2_BANK              REG_PORT_2_CTRL_BANK
#define REG_P2CR2_OFFSET            0x02

#define REG_P2VIDCR_BANK            REG_PORT_2_CTRL_BANK
#define REG_P2VIDCR_OFFSET          0x04

#define REG_P2CR3_BANK              REG_PORT_2_CTRL_BANK
#define REG_P2CR3_OFFSET            0x06

#define REG_P2IRCR_BANK             REG_PORT_2_CTRL_BANK
#define REG_P2IRCR_OFFSET           0x08

#define REG_P2ERCR_BANK             REG_PORT_2_CTRL_BANK
#define REG_P2ERCR_OFFSET           0x0A

#define REG_P2SCSLMD_BANK           REG_PORT_2_LINK_CTRL_BANK
#define REG_P2SCSLMD_OFFSET         0x00

#define REG_P2CR4_BANK              REG_PORT_2_LINK_CTRL_BANK
#define REG_P2CR4_OFFSET            0x02

#define REG_P2SR_BANK               REG_PORT_1_LINK_CTRL_BANK
#define REG_P2SR_OFFSET             0x04

#define REG_P3CR1_BANK              REG_PORT_3_CTRL_BANK
#define REG_P3CR1_OFFSET            0x02

#define REG_P3CR2_BANK              REG_PORT_3_CTRL_BANK
#define REG_P3CR2_OFFSET            0x02

#define REG_P3VIDCR_BANK            REG_PORT_3_CTRL_BANK
#define REG_P3VIDCR_OFFSET          0x04

#define REG_P3CR3_BANK              REG_PORT_3_CTRL_BANK
#define REG_P3CR3_OFFSET            0x06

#define REG_P3IRCR_BANK             REG_PORT_3_CTRL_BANK
#define REG_P3IRCR_OFFSET           0x08

#define REG_P3ERCR_BANK             REG_PORT_3_CTRL_BANK
#define REG_P3ERCR_OFFSET           0x0A

#define REG_P3SCSLMD_BANK           REG_PORT_3_LINK_CTRL_BANK
#define REG_P3SCSLMD_OFFSET         0x00

#define REG_P3CR4_BANK              REG_PORT_3_LINK_CTRL_BANK
#define REG_P3CR4_OFFSET            0x02

#define REG_P3SR_BANK               REG_PORT_1_LINK_CTRL_BANK
#define REG_P3SR_OFFSET             0x04

#define REG_PORT_CTRL_1_OFFSET      0x00

#define REG_PORT_CTRL_2_OFFSET      0x02
#define REG_PORT_CTRL_2_HI_OFFSET   0x03

#define REG_PORT_CTRL_VID_OFFSET    0x04

#define REG_PORT_CTRL_3_OFFSET      0x06

#define REG_PORT_IN_RATE_OFFSET     0x08
#define REG_PORT_OUT_RATE_OFFSET    0x0A

#define REG_PORT_LINK_MD_RESULT     0x00
#define REG_PORT_LINK_MD_CTRL       0x01

#define REG_PORT_CTRL_4_OFFSET      0x02

#define REG_PORT_STATUS_OFFSET      0x04
#define REG_PORT_STATUS_HI_OFFSET   0x05
#endif

/* -------------------------------------------------------------------------- */

/* KS884X PCI registers */

#ifdef KS_PCI_BUS
#define REG_SIDER_PCI               0x0400

#define REG_CHIP_ID_OFFSET          REG_SIDER_PCI
#define REG_FAMILY_ID_OFFSET        ( REG_CHIP_ID_OFFSET + 1 )

#define REG_SGCR1_PCI               0x0402

#define REG_SWITCH_CTRL_1_OFFSET    REG_SGCR1_PCI
#define REG_SWITCH_CTRL_1_HI_OFFSET ( REG_SWITCH_CTRL_1_OFFSET + 1 )

#define REG_SGCR2_PCI               0x0404

#define REG_SWITCH_CTRL_2_OFFSET    REG_SGCR2_PCI
#define REG_SWITCH_CTRL_2_HI_OFFSET ( REG_SWITCH_CTRL_2_OFFSET + 1 )

#define REG_SGCR3_PCI               0x0406
#define REG_SGCR3_OFFSET            REG_SGCR3_PCI

#define REG_SWITCH_CTRL_3_OFFSET    REG_SGCR3_PCI
#define REG_SWITCH_CTRL_3_HI_OFFSET ( REG_SWITCH_CTRL_3_OFFSET + 1 )

#define REG_SGCR4_PCI               0x0408
#define REG_SGCR5_PCI               0x040A

#define REG_SGCR6_PCI               0x0410

#define REG_SWITCH_CTRL_6_OFFSET    REG_SGCR6_PCI


#define REG_PHAR_PCI                0x0420
#define REG_LBS21R_PCI              0x0424
#define REG_LBRCTCER_PCI            0x0426


#define REG_MACAR1_PCI              0x0470
#define REG_MACAR2_PCI              0x0472
#define REG_MACAR3_PCI              0x0474

#define REG_MAC_ADDR_0_OFFSET       REG_MACAR1_PCI
#define REG_MAC_ADDR_1_OFFSET       ( REG_MAC_ADDR_0_OFFSET + 1 )
#define REG_MAC_ADDR_2_OFFSET       REG_MACAR2_PCI
#define REG_MAC_ADDR_3_OFFSET       ( REG_MAC_ADDR_2_OFFSET + 1 )
#define REG_MAC_ADDR_4_OFFSET       REG_MACAR3_PCI
#define REG_MAC_ADDR_5_OFFSET       ( REG_MAC_ADDR_4_OFFSET + 1 )


#define REG_TOSR1_PCI               0x0480
#define REG_TOSR2_PCI               0x0482
#define REG_TOSR3_PCI               0x0484
#define REG_TOSR4_PCI               0x0486
#define REG_TOSR5_PCI               0x0488
#define REG_TOSR6_PCI               0x048A
#define REG_TOSR7_PCI               0x0490
#define REG_TOSR8_PCI               0x0492

#define REG_TOS_1_OFFSET            REG_TOSR1_PCI
#define REG_TOS_2_OFFSET            REG_TOSR2_PCI
#define REG_TOS_3_OFFSET            REG_TOSR3_PCI
#define REG_TOS_4_OFFSET            REG_TOSR4_PCI
#define REG_TOS_5_OFFSET            REG_TOSR5_PCI
#define REG_TOS_6_OFFSET            REG_TOSR6_PCI

#define REG_TOS_7_OFFSET            REG_TOSR7_PCI
#define REG_TOS_8_OFFSET            REG_TOSR8_PCI


#define REG_IACR_PCI                0x04A0
#define REG_IACR_OFFSET             REG_IACR_PCI

#define REG_IADR1_PCI               0x04A2
#define REG_IADR2_PCI               0x04A4
#define REG_IADR3_PCI               0x04A6
#define REG_IADR4_PCI               0x04A8
#define REG_IADR5_PCI               0x04AA

#define REG_ACC_CTRL_SEL_OFFSET     REG_IACR_PCI
#define REG_ACC_CTRL_INDEX_OFFSET   ( REG_ACC_CTRL_SEL_OFFSET + 1 )

#define REG_ACC_DATA_0_OFFSET       REG_IADR4_PCI
#define REG_ACC_DATA_1_OFFSET       ( REG_ACC_DATA_0_OFFSET + 1 )
#define REG_ACC_DATA_2_OFFSET       REG_IADR5_PCI
#define REG_ACC_DATA_3_OFFSET       ( REG_ACC_DATA_2_OFFSET + 1 )
#define REG_ACC_DATA_4_OFFSET       REG_IADR2_PCI
#define REG_ACC_DATA_5_OFFSET       ( REG_ACC_DATA_4_OFFSET + 1 )
#define REG_ACC_DATA_6_OFFSET       REG_IADR3_PCI
#define REG_ACC_DATA_7_OFFSET       ( REG_ACC_DATA_6_OFFSET + 1 )
#define REG_ACC_DATA_8_OFFSET       REG_IADR1_PCI


#define REG_P1MBCR_PCI              0x04D0
#define REG_P1MBSR_PCI              0x04D2

#define REG_PHY1ILR_PCI             0x04D4
#define REG_PHY1IHR_PCI             0x04D6

#define REG_P1ANAR_PCI              0x04D8
#define REG_P1ANLPR_PCI             0x04DA

#define REG_P2MBCR_PCI              0x04E0
#define REG_P2MBSR_PCI              0x04E2

#define REG_PHY2ILR_PCI             0x04E4
#define REG_PHY2IHR_PCI             0x04E6

#define REG_P2ANAR_PCI              0x04E8
#define REG_P2ANLPR_PCI             0x04EA

#define REG_P1VCT_PCI               0x04F0
#define REG_P1PHYCTRL_PCI           0x04F2
#define REG_P2VCT_PCI               0x04F4
#define REG_P2PHYCTRL_PCI           0x04F6

#define REG_PHY_1_CTRL_OFFSET       REG_P1MBCR_PCI

#define REG_PHY_SPECIAL_OFFSET      REG_P1VCT_PCI

#define PHY_CTRL_INTERVAL           \
    ( REG_P2MBCR_PCI - REG_P1MBCR_PCI )

#define REG_PHY_CTRL_OFFSET         0x00
#define REG_PHY_STATUS_OFFSET       0x02
#define REG_PHY_ID_1_OFFSET         0x04
#define REG_PHY_ID_2_OFFSET         0x06
#define REG_PHY_AUTO_NEG_OFFSET     0x08
#define REG_PHY_REMOTE_CAP_OFFSET   0x0A

#define REG_PHY_LINK_MD_1_OFFSET    0x00
#define REG_PHY_PHY_CTRL_1_OFFSET   0x02
#define REG_PHY_LINK_MD_2_OFFSET    0x04
#define REG_PHY_PHY_CTRL_2_OFFSET   0x06

#define PHY_SPECIAL_INTERVAL        \
    ( REG_PHY_LINK_MD_2_OFFSET - REG_PHY_LINK_MD_1_OFFSET )

#define REG_P1CR1_PCI               0x0500
#define REG_P1CR2_PCI               0x0502
#define REG_P1VIDR_PCI              0x0504
#define REG_P1CR3_PCI               0x0506
#define REG_P1IRCR_PCI              0x0508
#define REG_P1ERCR_PCI              0x050A
#define REG_P1SCSLMD_PCI            0x0510
#define REG_P1CR4_PCI               0x0512
#define REG_P1SR_PCI                0x0514

#define REG_P2CR1_PCI               0x0520
#define REG_P2CR2_PCI               0x0522
#define REG_P2VIDR_PCI              0x0524
#define REG_P2CR3_PCI               0x0526
#define REG_P2IRCR_PCI              0x0528
#define REG_P2ERCR_PCI              0x052A
#define REG_P2SCSLMD_PCI            0x0530
#define REG_P2CR4_PCI               0x0532
#define REG_P2SR_PCI                0x0534

#define REG_P3CR1_PCI               0x0540
#define REG_P3CR2_PCI               0x0542
#define REG_P3VIDR_PCI              0x0544
#define REG_P3CR3_PCI               0x0546
#define REG_P3IRCR_PCI              0x0548
#define REG_P3ERCR_PCI              0x054A
#define REG_P3SR_PCI                0x0554

#define REG_PORT_1_CTRL_1_PCI       REG_P1CR1_PCI
#define REG_PORT_2_CTRL_1_PCI       REG_P2CR1_PCI
#define REG_PORT_3_CTRL_1_PCI       REG_P3CR1_PCI

#define REG_PORT_CTRL_1             REG_PORT_1_CTRL_1_PCI

#define PORT_CTRL_ADDR( port, addr )                                        \
    addr = REG_PORT_CTRL_1 + port *                                         \
        ( REG_PORT_2_CTRL_1_PCI - REG_PORT_1_CTRL_1_PCI )

#define REG_PORT_CTRL_1_OFFSET      0x00

#define REG_PORT_CTRL_2_OFFSET      0x02
#define REG_PORT_CTRL_2_HI_OFFSET   0x03

#define REG_PORT_CTRL_VID_OFFSET    0x04

#define REG_PORT_CTRL_3_OFFSET      0x06

#define REG_PORT_IN_RATE_OFFSET     0x08
#define REG_PORT_OUT_RATE_OFFSET    0x0A

#define REG_PORT_LINK_MD_RESULT     0x10
#define REG_PORT_LINK_MD_CTRL       0x11

#define REG_PORT_CTRL_4_OFFSET      0x12

#define REG_PORT_STATUS_OFFSET      0x14
#define REG_PORT_STATUS_HI_OFFSET   0x15
#endif

/* -------------------------------------------------------------------------- */

enum {
    TABLE_STATIC_MAC = 0,
    TABLE_VLAN,
    TABLE_DYNAMIC_MAC,
    TABLE_MIB
};

#define LEARNED_MAC_TABLE_ENTRIES  1024
#define STATIC_MAC_TABLE_ENTRIES   8

typedef struct {
    UCHAR  MacAddr[ MAC_ADDRESS_LENGTH ];
    USHORT wVID;
    UCHAR  bFID;
    UCHAR  bPorts;
    UCHAR  fOverride : 1;
    UCHAR  fUseFID : 1;
    UCHAR  fValid : 1;
} MAC_TABLE, *PMAC_TABLE;



#define VLAN_TABLE_ENTRIES  16

typedef struct {
    USHORT wVID;
    UCHAR  bFID;
    UCHAR  bMember;
} VLAN_TABLE, *PVLAN_TABLE;


#define SWITCH_PORT_NUM         2
#define TOTAL_PORT_NUM          ( SWITCH_PORT_NUM + 1 )

#define PORT_COUNTER_NUM        0x20
#define TOTAL_PORT_COUNTER_NUM  ( PORT_COUNTER_NUM + 2 )

#define MIB_COUNTER_RX_LO_PRIORITY       0x00
#define MIB_COUNTER_RX_HI_PRIORITY       0x01
#define MIB_COUNTER_RX_UNDERSIZE         0x02
#define MIB_COUNTER_RX_FRAGMENT          0x03
#define MIB_COUNTER_RX_OVERSIZE          0x04
#define MIB_COUNTER_RX_JABBER            0x05
#define MIB_COUNTER_RX_SYMBOL_ERR        0x06
#define MIB_COUNTER_RX_CRC_ERR           0x07
#define MIB_COUNTER_RX_ALIGNMENT_ERR     0x08
#define MIB_COUNTER_RX_CTRL_8808         0x09
#define MIB_COUNTER_RX_PAUSE             0x0A
#define MIB_COUNTER_RX_BROADCAST         0x0B
#define MIB_COUNTER_RX_MULTICAST         0x0C
#define MIB_COUNTER_RX_UNICAST           0x0D
#define MIB_COUNTER_RX_OCTET_64          0x0E
#define MIB_COUNTER_RX_OCTET_65_127      0x0F
#define MIB_COUNTER_RX_OCTET_128_255     0x10
#define MIB_COUNTER_RX_OCTET_256_511     0x11
#define MIB_COUNTER_RX_OCTET_512_1023    0x12
#define MIB_COUNTER_RX_OCTET_1024_1522   0x13
#define MIB_COUNTER_TX_LO_PRIORITY       0x14
#define MIB_COUNTER_TX_HI_PRIORITY       0x15
#define MIB_COUNTER_TX_LATE_COLLISION    0x16
#define MIB_COUNTER_TX_PAUSE             0x17
#define MIB_COUNTER_TX_BROADCAST         0x18
#define MIB_COUNTER_TX_MULTICAST         0x19
#define MIB_COUNTER_TX_UNICAST           0x1A
#define MIB_COUNTER_TX_DEFERRED          0x1B
#define MIB_COUNTER_TX_TOTAL_COLLISION   0x1C
#define MIB_COUNTER_TX_EXCESS_COLLISION  0x1D
#define MIB_COUNTER_TX_SINGLE_COLLISION  0x1E
#define MIB_COUNTER_TX_MULTI_COLLISION   0x1F

#define MIB_COUNTER_RX_DROPPED_PACKET    0x20
#define MIB_COUNTER_TX_DROPPED_PACKET    0x21


#define MIB_TABLE_ENTRIES  (TOTAL_PORT_COUNTER_NUM * 3 )

typedef struct {
    USHORT    wVID;
    UCHAR     bCurrentCounter;
    UCHAR     bMember;
    ULONG     dwRxRate[ 4 ];
    ULONG     dwTxRate[ 4 ];
    UCHAR     bPortPriority;
    UCHAR     bReserved1[ 3 ];

    ULONGLONG cnCounter[ TOTAL_PORT_COUNTER_NUM ];
    ULONG     cnDropped[ 2 ];
} PORT_CONFIG, *PPORT_CONFIG;


typedef struct {
    ULONG  ulHardwareState;
    ULONG  ulSpeed;
    UCHAR  bDuplex;
    UCHAR  bLinkPartner;
    UCHAR  bAdvertised;
    UCHAR  bReserved1;
    int    nSTP_State;
} PORT_INFO, *PPORT_INFO;


/* -------------------------------------------------------------------------- */

#define FP()  printk("%s:%d\n",__func__,__LINE__)

#endif
