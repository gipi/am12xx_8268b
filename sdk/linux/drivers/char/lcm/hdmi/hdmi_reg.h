/***************************************************************************
	hdmi_reg.h
 ***************************************************************************/

#ifndef __HDMI_REG_H__
#define __HDMI_REG_H__

//#define HDMI_RUN_ON_MODEL

#ifdef HDMI_RUN_ON_MODEL
	extern unsigned long hdmi_reg[0x200];
    #define HDMI_REG(x)		hdmi_reg[x/4]
#else
    #define HDMI_REG(x)		*((volatile unsigned long *)(0xB0050000+x))
#endif

#if MODULE_CONFIG_HDMI_HDCP_ENABLE

//#define ENABLE_HDCP 0
/* If wish to use the HDCP private key in NANDFLASH, set 1. */
#define USE_HDCP_P_KEY_IN_FLASH 1
#define LCM_HDCP_PKEY_PATH   "/am7x/case/data/HDCPPkey.bin"
#define DATA_OFFSET		0
#else
//#define ENABLE_HDCP 0
#define USE_HDCP_P_KEY_IN_FLASH 0
#define LCM_HDCP_PKEY_PATH   0
#define DATA_OFFSET		0
#endif


	/* HDMI */
	#define HDMI_REG_VR				HDMI_REG(0x000)
    #define HDMI_REG_CR				HDMI_REG(0x004)

	/* HDCP */    
    #define HDCP_REG_CR				HDMI_REG(0x008)
    #define HDCP_REG_SR				HDMI_REG(0x00C)
    #define HDCP_REG_AnLR			HDMI_REG(0x010)
    #define HDCP_REG_AnMR			HDMI_REG(0x014)
    #define HDCP_REG_AnILR			HDMI_REG(0x018)
    #define HDCP_REG_AnIMR			HDMI_REG(0x01C)
    #define HDCP_REG_DPKLR			HDMI_REG(0x020)
    #define HDCP_REG_DPKMR			HDMI_REG(0x024)
    #define HDCP_REG_LIR			HDMI_REG(0x028)
    #define HDCP_REG_SHACR			HDMI_REG(0x02C)
    #define HDCP_REG_SHADR			HDMI_REG(0x030)
    #define HDCP_REG_ICR			HDMI_REG(0x034)
    #define HDCP_REG_KMMR			HDMI_REG(0x038)
    #define HDCP_REG_KMLR			HDMI_REG(0x03C)
    #define HDCP_REG_MILR			HDMI_REG(0x040)
    #define HDCP_REG_MIMR			HDMI_REG(0x044)

	/* TMDS Serializer */
    #define TMDS_REG_SCR0			HDMI_REG(0x058)
    #define TMDS_REG_SCR1			HDMI_REG(0x05C)
    #define TMDS_REG_STR0			HDMI_REG(0x060)
    #define TMDS_REG_STR1			HDMI_REG(0x064)

	/* Data Island Packet */
    #define HDMI_REG_ASPCR			HDMI_REG(0x068)
    #define HDMI_REG_ACACR			HDMI_REG(0x06C)
    #define HDMI_REG_ACRPCR			HDMI_REG(0x070)
    #define HDMI_REG_ACRPTCSR		HDMI_REG(0x074)
    #define HDMI_REG_ACRPPR			HDMI_REG(0x078)
    #define HDMI_REG_GCPCR			HDMI_REG(0x07C)
    #define HDMI_REG_RPCR			HDMI_REG(0x080)
    #define HDMI_REG_RPRBDR			HDMI_REG(0x084)
    #define HDMI_REG_OPCR			HDMI_REG(0x088)
    #define HDMI_REG_DIPCCR			HDMI_REG(0x08C)

	/* Scheduler */
    #define HDCP_REG_KOWR			HDMI_REG(0x090)
    #define HDCP_REG_OWR			HDMI_REG(0x094)
    #define HDMI_REG_SCHCR			HDMI_REG(0x098)
    #define HDMI_REG_ICR			HDMI_REG(0x0DC)
    #define HDMI_REG_SCR			HDMI_REG(0x0E0)


	/* Debug */
    #define TMDS_REG_EODR0			HDMI_REG(0x0D4)
    #define TMDS_REG_EODR1			HDMI_REG(0x0D8)
    #define HDMI_REG_VINPUT			HDMI_REG(0x0DC)
    #define HDMI_REG_CRCCR			HDMI_REG(0x0E4)
    #define HDMI_REG_CRCDOR			HDMI_REG(0x0E8)
    #define HDMI_REG_VHCNT			HDMI_REG(0x0EC)

	/* Data Island User Packet (RAM Packet) */
	#define HDMI_REG_ORP6PH			HDMI_REG(0x100)
	#define HDMI_REG_ORSP6W0		HDMI_REG(0x104)
	#define HDMI_REG_ORSP6W1		HDMI_REG(0x108)
	#define HDMI_REG_ORSP6W2		HDMI_REG(0x10C)
	#define HDMI_REG_ORSP6W3		HDMI_REG(0x110)
	#define HDMI_REG_ORSP6W4		HDMI_REG(0x114)
	#define HDMI_REG_ORSP6W5		HDMI_REG(0x118)
	#define HDMI_REG_ORSP6W6		HDMI_REG(0x11C)
	#define HDMI_REG_ORSP6W7		HDMI_REG(0x120)

#endif

/* HDMI_REG_SCHCR */
#define HDMI_REG_SCHCR_DVI			0x0
#define HDMI_REG_SCHCR_HDMI			0x1
#define HDMI_REG_SCHCR_SYNC_INV		(0x3<<1)
#define HDMI_REG_SCHCR_REPEAT		(1<<3)
#define HDMI_REG_SCHCR_RGB			(0x0<<4)
#define HDMI_REG_SCHCR_YCbCr444		(0x2<<4)
#define HDMI_REG_SCHCR_YCbCr422		(0x3<<4)
#define HDMI_REG_SCHCR_LOW_ACTIVE	(0x3<<6)
#define HDMI_REG_SCHCR_VIDEOXS(x)	(x<<8)
#define HDMI_REG_SCHCR_REP422		(1<<13)
#define HDMI_REG_SCHCR_ACT_PXL		((32-14) << 8)
#define HDMI_REG_SCHCR_REP_ACT_PXL	(25 << 8)
#define HDMI_REG_SCHCR_ACT_PXL_BITS	(0x1f << 8)


/* HDCP_OWR */
#define HDCP_REG_OWR_OPP_START(x)	(x-4)
#define HDCP_REG_OWR_OPP_END(x)		((x-4)<<12)

/* HDCP_KOWR */
#define HDCP_REG_KOWR_KO_START(x)	(x-4)
#define HDCP_REG_KOWR_KO_END(x)		((x-4)<<12)
#define HDCP_REG_KOWR_KO_H(x)		((x-16)<<24)

/* HDMI_REG_DIPCCR */
#define HDMI_REG_DIPCCR_NARROW		(((8-1)<<8) | (2-1))
#define HDMI_REG_DIPCCR_WIDTH		(((18-1)<<8) | (4-1))

/* TMDS_OUTPUT_ENABLE */
#define TMDS_OUTPUT_ENABLE			(1<<31)

/* HDMI_REG_CR */
#define HDMI_REG_CR_HOTPLUG_INT		(1<<31)
#define HDMI_REG_CR_HOTPLUG_PENDING	(1<<30)
#define HDMI_REG_CR_HOTPLUG_STATUS	(1<<29)
#define HDMI_REG_CR_HOTPLUG_DB		(0xf<<24)
#define HDMI_REG_CR_BIT_REPE_AT_EN	(1<<5)
#define HDMI_REG_CR_FIFO_FILL			(1<<4)
#define HDMI_REG_CR_RESET_HDCP		(1<<3)
#define HDMI_REG_CR_ENABLE_HDCP		(1<<2)
#define HDMI_REG_CR_RESET			(1<<1)
#define HDMI_REG_CR_ENABLE			(1<<0)

/* HDCP_REG_CR */
#define HDCP_CR_11_FEATURE		(1<<31)
#define HDCP_CR_DS_REPEATER		(1<<30)
#define HDCP_CR_AN_INF_REQ		(1<<25)
#define HDCP_CR_SEED_LOAD		(1<<24)
#define HDCP_CR_RI_INT			(1<<9)
#define HDCP_CR_PJ_INT			(1<<8)
#define HDCP_CR_LOAD_AN			(1<<7)
#define HDCP_CR_ENC_ENABLE		(1<<6)
#define HDCP_CR_RESET_KM_ACC	(1<<4)
#define HDCP_CR_FORCE_UNAUTH	(1<<3)
#define HDCP_CR_AUTHENTICATED	(1<<2)
#define HDCP_CR_AUTH_COMP		(1<<1)
#define HDCP_CR_AUTH_REQ		(1<<0)


/* HDCP_REG_SR */
#define HDCP_SR_CIPHER_STAT		(0xff<<24)
#define HDCP_SR_RI_UPDATE		(1<<5)
#define HDCP_SR_PJ_UPDATE		(1<<4)
#define HDCP_SR_DPK_ACC			(1<<3)
#define HDCP_SR_ENCRYPT			(1<<2)
#define HDCP_SR_AUTH_OK			(1<<1)
#define HDCP_SR_AN_READY		(1<<0)

/* Internal SRAM allocation for Periodic Data Island Packet */
#define HDMI_RAMPKT_AVI_SLOT	0
#define HDMI_RAMPKT_AUDIO_SLOT	1
#define HDMI_RAMPKT_SPD_SLOT	2
#define HDMI_RAMPKT_VS_SLOT		3
#define HDMI_RAMPKT_ACP_SLOT	4
#define HDMI_RAMPKT_MPEG_SLOT	5

#define HDMI_RAMPKT_PERIOD		1

/* Audio */
/* HDMI_REG_ICR bit25 */
#define HDMI_AUDIO_ENABLE		(1<<25)

/* Packet */
#define HDMI_GCP_ENABLE			(1<<31)
#define HDMI_GCP_AVMUTE_ENABLE	(1<<0)
#define HDMI_GCP_AVMUTE_DISABLE	(1<<1)
