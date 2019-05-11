/* ---------------------------------------------------------------------------
          Copyright (c) 2006 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device_ethtool.c - Linux network device driver ethtool support.

    Author      Date        Description
    THa         05/25/06    Created file.
   ---------------------------------------------------------------------------
*/
#include <linux/mii.h>

#define EEPROM_SIZE  0x40

//#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);

static USHORT eeprom_data[ EEPROM_SIZE ] = { 0 };


#define ADVERTISED_ALL  (       \
    ADVERTISED_10baseT_Half |   \
    ADVERTISED_10baseT_Full |   \
    ADVERTISED_100baseT_Half |  \
    ADVERTISED_100baseT_Full )


/* These functions use the MII functions in mii.c. */
#ifdef MII
/*
    netdev_get_settings

    Description:
        This function queries the PHY and returns its state in the ethtool
        command.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_cmd* cmd
            Pointer to ethtool command.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_get_settings (
    struct net_device  *dev,
    struct ethtool_cmd *cmd )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    int              rc;

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    mii_ethtool_gset( &priv->mii_if, cmd );
    ReleaseHardware( hw_priv, FALSE );

    /* Save advertised settings for workaround in next function. */
    priv->advertising = cmd->advertising;
    return 0;
}  /* netdev_get_settings */


/*
    netdev_set_settings

    Description:
        This function sets the PHY according to the ethtool command.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_cmd* cmd
            Pointer to ethtool command.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_settings (
    struct net_device  *dev,
    struct ethtool_cmd *cmd )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              rc;

    /* ethtool utility does not change advertised setting if auto negotiation
       is not specified explicitly.
    */
    if ( cmd->autoneg  &&  priv->advertising == cmd->advertising ) {
        cmd->advertising |= ADVERTISED_ALL;
        if ( 10 ==  cmd->speed )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Full | ADVERTISED_100baseT_Half );
        else if ( 100 == cmd->speed )
            cmd->advertising &=
                ~( ADVERTISED_10baseT_Full | ADVERTISED_10baseT_Half );
        if ( 0 == cmd->duplex )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Full | ADVERTISED_10baseT_Full );
        else if ( 1 == cmd->duplex )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Half | ADVERTISED_10baseT_Half );
    }
    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    if ( ( cmd->advertising & ADVERTISED_ALL ) == ADVERTISED_ALL ) {
        pHardware->m_bDuplex = 0;
        pHardware->m_bSpeed = 0;
    }
    else {
        pHardware->m_bDuplex = cmd->duplex + 1;
        if ( cmd->speed != 1000 )
            pHardware->m_bSpeed = cmd->speed;
    }
    rc = mii_ethtool_sset( &priv->mii_if, cmd );

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    if ( !rc ) {
        int phy_id = cmd->phy_address;

        cmd->phy_address = priv->mii_if_2.phy_id;
        rc |= mii_ethtool_sset( &priv->mii_if_2, cmd );
        cmd->phy_address = phy_id;
    }
#endif
    ReleaseHardware( hw_priv, FALSE );
    return rc;
}  /* netdev_set_settings */


/*
    netdev_nway_reset

    Description:
        This function restarts the PHY for auto-negotiation.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_nway_reset (
    struct net_device *dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    int              rc;

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    rc = mii_nway_restart( &priv->mii_if );

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    rc |= mii_nway_restart( &priv->mii_if_2 );
#endif
    ReleaseHardware( hw_priv, FALSE );
    return rc;
}  /* netdev_nway_reset */


/*
    netdev_get_link

    Description:
        This function gets the link status from the PHY.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        True if PHY is linked and false otherwise.
*/

static u32 netdev_get_link (
    struct net_device *dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    int              rc;

    rc = mii_link_ok( &priv->mii_if );
    return rc;
}  /* netdev_get_link */
#endif


/*
    netdev_get_drvinfo

    Description:
        This procedure returns the driver information.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_drvinfo* info
            Pointer to ethtool driver info data structure.

    Return (None):
*/

static void netdev_get_drvinfo (
    struct net_device      *dev,
    struct ethtool_drvinfo *info )
{
    strcpy( info->driver, DRV_NAME );
    strcpy( info->version, DRV_VERSION );
}  /* netdev_get_drvinfo */


/*
    netdev_get_regs_len

    Description:
        This function returns the length of the register dump.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Length of the register dump.
*/

static struct hw_regs {
    int start;
    int end;
} hw_regs_range[] = {
    { 0, 3 },
    { 16, 19 },
    { 32, 33 },
    { 39, 42 },
    { 44, 52 },
    { 0, 0 }
};

static int netdev_get_regs_len (
    struct net_device *dev )
{
    struct hw_regs* pRange = hw_regs_range;
    int             regs_len = 0;
    
    while ( pRange->end > pRange->start ) {
        regs_len += ( pRange->end - pRange->start + 1 ) * 14;
        pRange++;
    }
    return( regs_len );
}  /* netdev_get_regs_len */


/*
    netdev_get_regs

    Description:
        This procedure dumps the register values in the provided buffer.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_regs* regs
            Pointer to ethtool registers data structure.

        void* ptr
            Pointer to buffer to store the register values.

    Return (None):
*/

static void netdev_get_regs (
    struct net_device   *dev,
    struct ethtool_regs *regs,
    void                *ptr )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    USHORT*          buf = ( USHORT* ) ptr;
    struct hw_regs*  pRange = hw_regs_range;
    int              bank;
    int              len;
    int              rc;
    
ACT_DEBUG_
    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return;
    }
    regs->version = 0;
    while ( pRange->end > pRange->start ) {
        for ( bank = pRange->start; bank <= pRange->end; bank++ ) {
            HardwareSelectBank( &hw_priv->hw, ( UCHAR ) bank );
            for ( len = 0; len < 14; len += 2 ) {
                HW_READ_WORD( &hw_priv->hw, len, buf );
                buf++;
            }
        }
        pRange++;
    }
    ReleaseHardware( hw_priv, FALSE );
}  /* netdev_get_regs */


#if 0
/*
    netdev_get_wol

    Description:
        This procedure returns Wake-on-LAN support.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_wolinfo* wol
            Pointer to ethtool Wake-on-LAN data structure.

    Return (None):
*/

static void netdev_get_wol (
    struct net_device      *dev,
    struct ethtool_wolinfo *wol )
{

    wol->supported = WAKE_PHY | WAKE_MAGIC;
    wol->wolopts = WAKE_PHY | WAKE_MAGIC;
    memset( &wol->sopass, 0, sizeof( wol->sopass ));
}  /* netdev_get_wol */


/*
    netdev_set_wol

    Description:
        This function sets Wake-on-LAN support.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_wolinfo* wol
            Pointer to ethtool Wake-on-LAN data structure.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_wol (
    struct net_device      *dev,
    struct ethtool_wolinfo *wol )
{
    u32 support;

    support = WAKE_PHY | WAKE_MAGIC;
    if ( wol->wolopts & ~support )
        return -EINVAL;

    return 0;
}  /* netdev_set_wol */
#endif


/*
    netdev_get_msglevel

    Description:
        This function returns current debug message level.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (u32):
        Current debug message flags.
*/

static u32 netdev_get_msglevel (
    struct net_device *dev )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);

    return priv->msg_enable;
}  /* netdev_get_msglevel */


/*
    netdev_set_msglevel

    Description:
        This procedure sets debug message level.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        u32 value
            Debug message flags.

    Return (None):
*/

static void netdev_set_msglevel (
    struct net_device *dev, 
    u32               value )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);

    priv->msg_enable = value;
}  /* netdev_set_msglevel */


/*
    netdev_get_eeprom_len

    Description:
        This function returns the length of the EEPROM.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Length of the EEPROM.
*/

static int netdev_get_eeprom_len (
    struct net_device *dev )
{
    return( EEPROM_SIZE * 2 );
}  /* netdev_get_eeprom_len */


/*
    netdev_get_eeprom

    Description:
        This function dumps the EEPROM data in the provided buffer.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_eeprom* eeprom
            Pointer to ethtool EEPROM data structure.

        u8* data
            Pointer to buffer store the EEPROM data.

    Return (int):
        Zero if successful; otherwise an error code.
*/

#define EEPROM_MAGIC  0x10A18842

static int netdev_get_eeprom (
    struct net_device     *dev,
    struct ethtool_eeprom *eeprom,
    u8                    *data )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    UCHAR*           eeprom_byte = ( UCHAR* ) eeprom_data;
    UCHAR            i;
    UCHAR            len;

ACT_DEBUG_
    len = ( eeprom->offset + eeprom->len + 1 ) / 2;
    for ( i = eeprom->offset / 2; i < len; i++ )
        eeprom_data[ i ] = EepromReadWord( &hw_priv->hw, i );
    eeprom->magic = EEPROM_MAGIC;
    memcpy( data, &eeprom_byte[ eeprom->offset ], eeprom->len );

    return 0;
}  /* netdev_get_eeprom */


/*
    netdev_set_eeprom

    Description:
        This function modifies the EEPROM data one byte at a time.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_eeprom* eeprom
            Pointer to ethtool EEPROM data structure.

        u8* data
            Pointer to data buffer.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_eeprom (
    struct net_device     *dev,
    struct ethtool_eeprom *eeprom,
    u8                    *data )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    USHORT           eeprom_word[ EEPROM_SIZE ];
    UCHAR*           eeprom_byte = ( UCHAR* ) eeprom_word;
    UCHAR            i;
    UCHAR            len;

ACT_DEBUG_
    if ( eeprom->magic != EEPROM_MAGIC )
        return 1;

    len = ( eeprom->offset + eeprom->len + 1 ) / 2;
    for ( i = eeprom->offset / 2; i < len; i++ )
        eeprom_data[ i ] = EepromReadWord( &hw_priv->hw, i );
    memcpy( eeprom_word, eeprom_data, EEPROM_SIZE * 2 );
    memcpy( &eeprom_byte[ eeprom->offset ], data, eeprom->len );
    for ( i = 0; i < EEPROM_SIZE; i++ )
        if ( eeprom_word[ i ] != eeprom_data[ i ] ) {
            eeprom_data[ i ] = eeprom_word[ i ];
            EepromWriteWord( &hw_priv->hw, i, eeprom_data[ i ]);
        }

    return 0;
}  /* netdev_set_eeprom */


/*
    netdev_get_pauseparam

    Description:
        This procedures returns the PAUSE control flow settings.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_pauseparam* pause
            Pointer to ethtool PAUSE settings data structure.

    Return (None):
*/

static void netdev_get_pauseparam (
    struct net_device         *dev,
    struct ethtool_pauseparam *pause )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;

ACT_DEBUG_
    pause->autoneg = 1;
    pause->rx_pause =
        ( pHardware->m_wReceiveConfig & RX_CTRL_FLOW_ENABLE ) ? 1 : 0;
    pause->tx_pause =
        ( pHardware->m_wTransmitConfig & TX_CTRL_FLOW_ENABLE ) ? 1 : 0;
}  /* netdev_get_pauseparam */


/*
    netdev_set_pauseparam

    Description:
        This function sets the PAUSE control flow settings.
        Not implemented.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_pauseparam* pause
            Pointer to ethtool PAUSE settings data structure.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_pauseparam (
    struct net_device         *dev,
    struct ethtool_pauseparam *pause )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    USHORT           wConfig;

    wConfig = pHardware->m_wReceiveConfig;
    if ( pause->rx_pause )
        wConfig |= RX_CTRL_FLOW_ENABLE;
    else
        wConfig &= ~RX_CTRL_FLOW_ENABLE;
    if ( wConfig != pHardware->m_wReceiveConfig ) {
        pHardware->m_wReceiveConfig = wConfig;
    }

    wConfig = pHardware->m_wTransmitConfig;
    if ( pause->tx_pause )
        wConfig |= TX_CTRL_FLOW_ENABLE;
    else
        wConfig &= ~TX_CTRL_FLOW_ENABLE;
    if ( wConfig != pHardware->m_wTransmitConfig ) {
        pHardware->m_wTransmitConfig = wConfig;
    }

    return 0;
}  /* netdev_set_pauseparam */


#if defined( DEF_KS8841 )  ||  defined( TWO_NETWORK_INTERFACE )
#define STATS_LEN  ( TOTAL_PORT_COUNTER_NUM )
#else
#define STATS_LEN  ( TOTAL_PORT_COUNTER_NUM * TOTAL_PORT_NUM )
#endif

static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_keys[ STATS_LEN ] = {
    { "rx_lo_priority_octets" },
    { "rx_hi_priority_octets" },
    { "rx_undersize_packets" },
    { "rx_fragments" },
    { "rx_oversize_packets" },
    { "rx_jabbers" },
    { "rx_symbol_errors" },
    { "rx_crc_errors" },
    { "rx_align_errors" },
    { "rx_mac_ctrl_packets" },
    { "rx_pause_packets" },
    { "rx_bcast_packets" },
    { "rx_mcast_packets" },
    { "rx_ucast_packets" },
    { "rx_64_or_less_octet_packets" },
    { "rx_65_to_127_octet_packets" },
    { "rx_128_to_255_octet_packets" },
    { "rx_256_to_511_octet_packets" },
    { "rx_512_to_1023_octet_packets" },
    { "rx_1024_to_1522_octet_packets" },

    { "tx_lo_priority_octets" },
    { "tx_hi_priority_octets" },
    { "tx_late_collisions" },
    { "tx_pause_packets" },
    { "tx_bcast_packets" },
    { "tx_mcast_packets" },
    { "tx_ucast_packets" },
    { "tx_deferred" },
    { "tx_total_collisions" },
    { "tx_excessive_collisions" },
    { "tx_single_collisions" },
    { "tx_mult_collisions" },

    { "rx_discards" },
    { "tx_discards" },

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    { "rx_1_lo_priority_octets" },
    { "rx_1_hi_priority_octets" },
    { "rx_1_undersize_packets" },
    { "rx_1_fragments" },
    { "rx_1_oversize_packets" },
    { "rx_1_jabbers" },
    { "rx_1_symbol_errors" },
    { "rx_1_crc_errors" },
    { "rx_1_align_errors" },
    { "rx_1_mac_ctrl_packets" },
    { "rx_1_pause_packets" },
    { "rx_1_bcast_packets" },
    { "rx_1_mcast_packets" },
    { "rx_1_ucast_packets" },
    { "rx_1_64_or_less_octet_packets" },
    { "rx_1_65_to_127_octet_packets" },
    { "rx_1_128_to_255_octet_packets" },
    { "rx_1_256_to_511_octet_packets" },
    { "rx_1_512_to_1023_octet_packets" },
    { "rx_1_1024_to_1522_octet_packets" },

    { "tx_1_lo_priority_octets" },
    { "tx_1_hi_priority_octets" },
    { "tx_1_late_collisions" },
    { "tx_1_pause_packets" },
    { "tx_1_bcast_packets" },
    { "tx_1_mcast_packets" },
    { "tx_1_ucast_packets" },
    { "tx_1_deferred" },
    { "tx_1_total_collisions" },
    { "tx_1_excessive_collisions" },
    { "tx_1_single_collisions" },
    { "tx_1_mult_collisions" },

    { "rx_1_discards" },
    { "tx_1_discards" },

    { "rx_2_lo_priority_octets" },
    { "rx_2_hi_priority_octets" },
    { "rx_2_undersize_packets" },
    { "rx_2_fragments" },
    { "rx_2_oversize_packets" },
    { "rx_2_jabbers" },
    { "rx_2_symbol_errors" },
    { "rx_2_crc_errors" },
    { "rx_2_align_errors" },
    { "rx_2_mac_ctrl_packets" },
    { "rx_2_pause_packets" },
    { "rx_2_bcast_packets" },
    { "rx_2_mcast_packets" },
    { "rx_2_ucast_packets" },
    { "rx_2_64_or_less_octet_packets" },
    { "rx_2_65_to_127_octet_packets" },
    { "rx_2_128_to_255_octet_packets" },
    { "rx_2_256_to_511_octet_packets" },
    { "rx_2_512_to_1023_octet_packets" },
    { "rx_2_1024_to_1522_octet_packets" },

    { "tx_2_lo_priority_octets" },
    { "tx_2_hi_priority_octets" },
    { "tx_2_late_collisions" },
    { "tx_2_pause_packets" },
    { "tx_2_bcast_packets" },
    { "tx_2_mcast_packets" },
    { "tx_2_ucast_packets" },
    { "tx_2_deferred" },
    { "tx_2_total_collisions" },
    { "tx_2_excessive_collisions" },
    { "tx_2_single_collisions" },
    { "tx_2_mult_collisions" },

    { "rx_2_discards" },
    { "tx_2_discards" },
#endif

#if 0
    { "1234567890123456789012345678901" }
#endif
};


/*
    netdev_get_strings

    Description:
        This procedure returns the strings used to identify the statistics.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        u32 stringset
            String set identifier.

        u8* buf
            Pointer to buffer to store the strings.

    Return (None):
*/

static void netdev_get_strings (
    struct net_device *dev,
    u32               stringset,
    u8                *buf )
{
    if ( ETH_SS_STATS == stringset ) {
        memcpy( buf, &ethtool_stats_keys, sizeof( ethtool_stats_keys ));
    }
}  /* netdev_get_strings */


/*
    netdev_get_stats_count

    Description:
        This function returns the size of the statistics to be reported.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Size of the statistics to be reported.
*/

static int netdev_get_stats_count (
    struct net_device *dev )
{
    return( STATS_LEN );
}  /* netdev_get_stats_count */


/*
    netdev_get_ethtool_stats

    Description:
        This procedure returns the statistics.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_stats* stats
            Pointer to ethtool statistics data structure.

        u64* data
            Pointer to buffer to store the statistics.

    Return (None):
*/

static void netdev_get_ethtool_stats (
    struct net_device    *dev,
    struct ethtool_stats *stats,
    u64                  *data )
{
    struct dev_priv* priv = ( struct dev_priv* ) netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PPORT_CONFIG     pPort;
    int              n_stats = stats->n_stats;
    int              port = MAIN_PORT;
    int              i;
    int              n;
    int              rc;

ACT_DEBUG_

#ifdef TWO_NETWORK_INTERFACE
    port = priv->port;
#endif
    hw_priv->Counter[ port ].fRead = 1;

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    hw_priv->Counter[ OTHER_PORT ].fRead = 1;
    hw_priv->Counter[ HOST_PORT ].fRead = 1;
#endif
    rc = interruptible_sleep_on_timeout(
        &hw_priv->Counter[ port ].wqhCounter, HZ * 2 );

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    if ( pHardware->m_Port[ OTHER_PORT ].bCurrentCounter )
        rc = interruptible_sleep_on_timeout(
            &hw_priv->Counter[ OTHER_PORT ].wqhCounter, HZ * 1 );
    if ( pHardware->m_Port[ HOST_PORT ].bCurrentCounter )
        rc = interruptible_sleep_on_timeout(
            &hw_priv->Counter[ HOST_PORT ].wqhCounter, HZ * 1 );
#endif
    pPort = &pHardware->m_Port[ port ];
    n = TOTAL_PORT_COUNTER_NUM;
    if ( n > n_stats )
        n = n_stats;
    n_stats -= n;
    for ( i = 0; i < n; i++ ) {
        *data++ = ( u64 ) pPort->cnCounter[ i ];
    }

#if defined( DEF_KS8842 )  &&  !defined( TWO_NETWORK_INTERFACE )
    pPort = &pHardware->m_Port[ OTHER_PORT ];
    if ( n > n_stats )
        n = n_stats;
    n_stats -= n;
    for ( i = 0; i < n; i++ ) {
        *data++ = ( u64 ) pPort->cnCounter[ i ];
    }
    pPort = &pHardware->m_Port[ HOST_PORT ];
    if ( n > n_stats )
        n = n_stats;
    n_stats -= n;
    for ( i = 0; i < n; i++ ) {
        *data++ = ( u64 ) pPort->cnCounter[ i ];
    }
#endif

    return;
}  /* netdev_get_ethtool_stats */


static struct ethtool_ops netdev_ethtool_ops = {
#ifdef MII
    .get_settings       = netdev_get_settings,
    .set_settings       = netdev_set_settings,
    .nway_reset         = netdev_nway_reset,
    .get_link           = netdev_get_link,
#endif
    .get_drvinfo        = netdev_get_drvinfo,
    .get_regs_len       = netdev_get_regs_len,
    .get_regs           = netdev_get_regs,
    .get_msglevel       = netdev_get_msglevel,
    .set_msglevel       = netdev_set_msglevel,
    .get_eeprom_len     = netdev_get_eeprom_len,
    .get_eeprom         = netdev_get_eeprom,
    .set_eeprom         = netdev_set_eeprom,
    .get_pauseparam     = netdev_get_pauseparam,
    .set_pauseparam     = netdev_set_pauseparam,
    .get_strings        = netdev_get_strings,
    .get_stats_count    = netdev_get_stats_count,
    .get_ethtool_stats  = netdev_get_ethtool_stats,
};
