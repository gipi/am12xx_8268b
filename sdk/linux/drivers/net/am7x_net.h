#ifndef _AM7X_NET_H_
#define _AM7X_NET_H_


#include "actions_io.h"
#include "actions_regs.h"
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h> ///for eth_type_trans function etc
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <dma.h>
#include <sys_at24c02.h>
#ifdef CONFIG_AM_8251
#include <am7x_board.h>
#endif


#define MAC_BD_TOTALNUM 128
//Mos: Set TX_BD_NUM=1 to send packet once a time, add time gap between packet, to avoid LAN packet drop/disconnect
//#define MAC_TX_BD_NUM   48
#define MAC_TX_BD_NUM   1
#define MAC_TX_BD_GAP   0

#define MAC_BaseAddr	0xB0200000
#define MAC_CtlReg			0x0000			////Control Register 
#define MAC_Interrupt		0x0004			/// Interrupt Register
#define MAC_PackageLen		0x0008			/// Packet Length Register
#define MAC_InterPacketGap	0x000c			/// IPG Register
#define MAC_MIIInfo			0x0010			/// MII Infomation Register
#define MAC_MIIData			0x0014			/// MII Data Register
#define MAC_MACAddr0		0x0018			/// MAC Address Register 0
#define MAC_MACAddr1		0x001c			/// MAC Address Register 1
#define MAC_MACAddr2		0x0020			/// MAC Address Register 2
#define MAC_MACAddr3		0x0024			/// MAC Address Register 3
#define MAC_MACAddr4		0x0028			/// MAC Address Register 4
#define MAC_TypeLenOp		0x002c			/// Type_len_Op Register
#define MAC_TxCtl			0x0030			/// TX Control Register
#define MAC_SDRBaseAddr		0x0034			/// SDR Base addr Register
#define MAC_BDAddrLatch		0x0038			/// Bd Addr Latch Register
#define MAC_RXPcktCnt		0x003c			/// Rx Packet Counter Register
#define MAC_RXDropCnt		0x0040			/// RX DROP Counter Register
#define MAC_CRCErrCnt		0x0044			/// RX CRC Error Counter Register
#define MAC_RXNibCnt			0x0048			/// RX Nibber Counter Register
#define MAC_CtlFrameCnt		0x004c			/// RX Control Frame Counter Register
#define MAC_MACMbistAddr		0x0050			/// MAC MBIST Register
#define MAC_BDSwapAddr		0x0054			/// RX & TX BD Swap Address Register
#define MAC_TXBDPtr			0x0058			/// TX BD Pointer Register
#define MAC_RXBDPtr			0x005c			/// RX BD Pointer Register
#define MAC_RXAddrCnt		0x0060			/// RX addr abort counter Register
#define MAC_RXErrCnt			0x0064			/// RX error abort counter Register
#define MAC_RXFrameCnt		0x0068			/// RX frame abort counter Register

#define MAC_BDDescription		0x0200			/// BD Description Base Addr

#define MAC_Slot_Size (0x200+MAC_BD_TOTALNUM*4)
#define MTU_MAX_BUF_LEN	2048			///the max length of the buffer for transmit or receive


#define test_bit(bit, var)	      ((var) & (1 << (bit)))
#define set_bit(bit, var)	      		(var) |= 1 << (bit)
#define clear_bit(bit, var)	      (var) &= ~(1 << (bit))


typedef enum{
	MACADDR_TYPE_SOURCE,			///< source address of the mac
	MACADDR_TYPE_DESTINATION,		///< destination address of the mac
	MACADDR_TYPE_MULTICAST,		///< multicast address of the mac
}macaddr_type_e;

typedef enum{
	BD_TX,
	BD_RX,
}bd_type_e;

typedef struct bd_swap_addr_s
{
	unsigned int rx_swap_addr;
	unsigned int tx_swap_addr;
}bd_swap_addr_t;

typedef struct bd_pointer_s
{
	unsigned int read_ptr;
	unsigned int write_ptr;
	unsigned int read_around:1;			//the 7th bit changed means it is turned around
	unsigned int write_around:1;
}bd_pointer_t;

static int watchdog = 5000;

typedef struct am7x_board_info_s
{
	unsigned char *bd_mem_addr;		///< bd sdram virtual addr
	unsigned int bd_phy_addr;			///< bd sdram physical addr
	
	unsigned int tx_bd_num;			///< tx bd number
	bd_swap_addr_t swap_addr;			///< bd swap addr
	
	bd_pointer_t txbd_pointer;			///< tx bd pointer read only
	bd_pointer_t rxbd_pointer;			///< rx bd pointer read only

	unsigned int txdb_full:1;						///< is tx bd full
	unsigned int use_napi:1;						///< if this bit is 1 , use poll function to receive packages

	unsigned int rx_timeout;
	
	struct sk_buff *skb;
	struct device	*dev;	     /* parent device */
	spinlock_t	lock;
	struct mutex phy_lock;	/* phy lock */
	struct mii_if_info mii;
	u32	msg_enable;
	struct net_device_stats stats;

	struct delayed_work phy_poll;///Phy status poll

	struct net_device *ndev;		///< net device
}am7x_board_info_t;

/*
typedef struct mac_address_s{
	unsigned char mac_addr_0;
	unsigned char mac_addr_1;
	unsigned char mac_addr_2;
	unsigned char mac_addr_3;
	unsigned char mac_addr_4;
	unsigned char mac_addr_5;
}mac_address_t;
typedef struct mac_address_s * mac_address_p;



typedef struct ip_address_s{
	unsigned char ip_addr_0;
	unsigned char ip_addr_1;
	unsigned char ip_addr_2;
	unsigned char ip_addr_3;
}ip_address_t;

typedef struct ip_address_s * ip_address_p; 


#define MACIO_MAXNR					8
#define MACIO_SET_MAC_ADDR			_IOW('d',1,mac_address_p)
#define MACIO_GET_MAC_ADDR	   	 	_IOR('d',2,mac_address_p)	
#define MACIO_SET_WAKEUP_IP			_IOW('d',3,ip_address_p)
#define MACIO_SET_WAKEUP_MASK		_IOW('d',4,ip_address_p)

#define MACIO_GET_WAKEUP_IP			_IOR('d',5,ip_address_p)
#define MACIO_GET_WAKEUP_MASK 		_IOR('d',6,ip_address_p)
#define MACIO_SET_WAKEUP_MODE       _IOW('d',7,unsigned char)
#define MACIO_GET_WAKEUP_MODE		_IOR('d',8,unsigned char)
*/
#define MAC_DEFAULT_WAKEUPMODE      (0x1f)       //default wake up mode:support all mac wakeup mode
#define MAC_DEFAULT_WAKEUPIP        (0xc0a80102) //default wake up ip is 192.168.1.2
#define MAC_DEFAULT_WAKEUPMASK      (0x0)        //default wake up ip is 0.0.0.0

/****KSZ8051 Information and Control Register****/
#define PHY_BASIC_CONTROL 			(0x00)			///basic control
#define PHY_BASIC_STATUS			(0x01)			///basic status
#define PHY_IDENTIFY1				(0x02)			///phy identifier 1
#define PHY_IDENTIFY2				(0x03)			///phy identifier 2
#define PHY_AUTONEGO_ADV			(0x04)			///auto-negotiation advertisement
#define PHY_AUTONEGO_LINK			(0x05)			///auto-negotiation link partner ability
#define PHY_AUTONEGO_EXP			(0x06)			///auto-negotiation expansion
#define PHY_AUTONEGO_NP			(0x07)			///auto-negotiation next page
#define PHY_LINKPART					(0x08)			///link partner next page
#define PHY_AFECONTROL				(0x11)			///AFE Control 1
#define PHY_RXERCOUNTER				(0x15)			///RXER Counter
#define PHY_OP_MODESTRAP_OVERRIDE (0x16)			///operation mode strap override
#define PHY_OP_MODESTRAP_STATUS	(0x17)			///operation mode strap status
#define PHY_EXP_CONTROL			(0x18)			///expanded control
#define PHY_INT_CTL_ST				(0x1B)			///interrupt control/status
#define PHY_LINKMD_CTL_ST			(0x1D)			///linkMD control/status
#define PHY_CONTROL_1				(0x1E)			///phy control 1
#define PHY_CONTROL_2				(0x1F)			///phy control 2
#if(ENABLE_8201F_WOL_MODE)
#define ENABLE_8201F_TX_RX_ISOLATION    1
#define MAX_PACKET_LENGTH                         0x1FFF
#define ENABLE_MAGIC_PACKET_EVENT         0x1000
#define ENABLE_TX_RX_ISOLATION_BIT         (1<<15)
#define ENABLE_PHY_ISOLATE_BIT                   (1<<10)
#endif

typedef struct mii_info_reg_s
{
	unsigned int lnk_fail:1;	
	unsigned int mii_busy:1;
	unsigned int invalid:1;
	unsigned int reserved1:2;
	unsigned int phy_addr:5;
	unsigned int reg_addr:5;
	unsigned int reserved2:1;
	unsigned int sc_status:1;
	unsigned int rd_status:1;
	unsigned int wr_ctl_data:1;
	unsigned int reserved3:1;
	unsigned int mii_clk_div:8;
	unsigned int mii_no_pre:1;
	unsigned int mii_clk_en:1;
	unsigned int reserved4:2;
}mii_info_reg_t;


typedef struct mii_data_reg_s
{
	unsigned int rd_status:16;
	unsigned int ctl_data:16;
}mii_data_reg_t;

#if CONFIG_AM_CHIP_ID==1213
/** all in MFCTL1 */
#define MAC_TD3    (1<<5)		
#define MAC_TD2    (1<<7)
#define MAC_TD1    (1<<9)
#define MAC_TD0    (1<<11)
#define MAC_TVLD   (1<<13)	
#define MAC_TCLK   (1<<13)	
#define MAC_RD0    (1<<16)
#define MAC_RD1    (1<<18)
#define MAC_RD2    (1<<20)
#define MAC_RD3    (1<<22)
#define MAC_MDC    (1<<24)
#define MAC_MDIO   (1<<26)
#define MAC_TER    (1<<13)
#define MAC_RER    (1<<13)
#define MAC_RCLK   (1<<13)
#define MAC_RVLD   (1<<13)
/** mask */
#define MAC_TD3_MASK    (~(3<<5))		
#define MAC_TD2_MASK    (~(3<<7))
#define MAC_TD1_MASK    (~(3<<9))
#define MAC_TD0_MASK    (~(3<<11))
#define MAC_TVLD_MASK   (~(7<<13))
#define MAC_TCLK_MASK   (~(7<<13))
#define MAC_RD0_MASK    (~(3<<16))
#define MAC_RD1_MASK    (~(3<<18))
#define MAC_RD2_MASK    (~(3<<20))
#define MAC_RD3_MASK    (~(3<<22))
#define MAC_MDC_MASK    (~(3<<24))
#define MAC_MDIO_MASK   (~(3<<26))
#define MAC_TER_MASK    (~(7<<13))
#define MAC_RER_MASK    (~(7<<13))
#define MAC_RCLK_MASK   (~(7<<13))
#define MAC_RVLD_MASK   (~(7<<13))

#else
//*** the following pads are in mulit function register 6
#define MAC_TD3 		(1)				///3bits		TXD3
#define MAC_TD2 		(1<<3)			///3bits		TXD2
#define MAC_TD1		(1<<6)			///3bits		TXD1
#define MAC_TD0		(1<<9)			///3bits		TXD0
#define MAC_TVLD 	(1<<12)			///3bits		TXEN
#define MAC_TCLK 	(1<<15)			///3bits		TXCLK
#define MAC_RD0		(1<<22)			///2bits		RXD0
#define MAC_RD1		(1<<24)			///2bits		RXD1
#define MAC_RD2		(1<<26)			///2bits		RXD2
#define MAC_RD3		(1<<28)			///2bits		RXD3
#define MAC_MDC		(1<<30)			///1bits		MDC
#define MAC_MDIO	(1<<31)			///1bits		MDIO


//*** the following pads are in mulit function register 4
#define MAC_TER		(1<<17)		///2bits   	TXER
#define MAC_RER		(1<<19)		///2bits	RXER
#define MAC_RCLK		(1<<21)		///2bits	RXCLK
#define MAC_RVLD		(1<<23)		///2bits	RXDV
#endif

typedef struct ctl_reg_info_s
{
	unsigned int tx_bd_num:7;			///transmit buffer descriptor number(TX BD)
	unsigned int pass_all:1;				///pass all receive frames
	unsigned int addr_check:1;			///address check enable
	unsigned int rx_st_mode:1;			///received frame storage mode
	unsigned int bro:1;					///bro---broadcast address
	unsigned int ifg:1;					/// inter frame gap for incoming frames
	unsigned int rx_flow:1;				///receive flow control
	unsigned int tx_flow:1;				///transmit flow control
	unsigned int rx_en:1;				///rxen--receive enable
	unsigned int tx_en:1;				///txen--transmit enable
	unsigned int dly_rcr_en:1;			///delayed crc enabled
	unsigned int crc_en:1;				///crc enable must be 1 if mac_en is 1
	unsigned int pad:1;					///padding enabled
	unsigned int mac_en:1;				///mac information can be added in mac module
	unsigned int huge_en:1;				///huge packets enable
	unsigned int full_duplex:1;			///full duplex
	unsigned int loop_en:2;				///loop back enable
	unsigned int tx_error_en:1;			///tx_error signal enable
	unsigned int r_gap_cnt:5;			///rx inter-frame gap cnt
	unsigned int reserved:2;
}ctl_reg_info_t;

typedef struct int_reg_info_s
{
	unsigned int txb_m:1;				///transmit buffer mask
	unsigned int txe_m:1;				///transmit error mask
	unsigned int rxb_m:1;				///receive frame mask
	unsigned int rxe_m:1;				///receive error mask
	unsigned int busy_m:1;				///busy mask
	unsigned int txc_m:1;				///transmit control frame mask
	unsigned int rxc_m:1;				///receive Control frame mask
	unsigned int swbdad_m:1;			///software bd write address error MASK
	unsigned int txb:1;					///transmit buffer
	unsigned int txe:1;					///transmit error
	unsigned int rxb:1;					///receive frame
	unsigned int rxe:1;					///receiver error
	unsigned int busy:1;				///busy
	unsigned int txc:1;					///transmit control frame
	unsigned int rxc:1;					///receive control frame
	unsigned int swad:1;				///software BD write address error
	unsigned int rx_over_run_pd:1;		///rx over run error
	unsigned int invalid_symbol_pd:1;	///invalid symbol error
	unsigned int rx_pckt_too_big_pd:1;	///rx packet too big error
	unsigned int rx_dribble_nib:1;		///rx dribble nibble error
	unsigned int rx_short_frame_pd:1;	///rx short frame error
	unsigned int rx_crc_error_pd:1;		///rx crc error
	unsigned int reserved:10;
}int_reg_info_t;


typedef struct ipg_reg_info_s
{
	unsigned int ipg2:7;
	unsigned int rx_pre_min:5;
	unsigned int reserved1:4;
	unsigned int ipgt:7;
	unsigned int rx_pre_max:5;
	unsigned int reserved2:4;
}ipg_reg_info_t;


#endif
