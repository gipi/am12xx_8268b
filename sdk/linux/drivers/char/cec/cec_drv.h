/***************************************************************************
	cec_drv.c
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Actionsmicro Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          cec_drv.c  -  description
                             -------------------
    begin                : Thursday February 13, 2013
    copyright            : (C) 2013 by Yang Jianying
    email                : yangjy@actions-micro.com 
 ***************************************************************************/

/**
 * @file cec_drv.h
 * CEC Low Level driver.
 *
 * @author Yang Jianying
 * @email yangjy@actions-micro.com  
 * @date Thursday February 13, 2013
 * @version 0.1
 * @ingroup pres_hdmi
 *
 */

#include <actions_regs.h>
#include "rtd_types.h"


#define EDID_INFO_PATH  "/mnt/vram/edidinfo.bin"

/* Device Type */
#define CEC_DEVTYPE_TV				0x00
#define CEC_DEVTYPE_RECORDING		0x01
#define CEC_DEVTYPE_RESERVED		0x02
#define CEC_DEVTYPE_TUNER			0x03
#define CEC_DEVTYPE_PLAYBACK		0x04
#define CEC_DEVTYPE_AUDIO			0x05
#define CEC_DEVTYPE_FREEUSE			0x06

#define CEC_MAJOR	0
#define N_CEC_CHAN	1
#define CEC_FRAME_LEN 16
#define CEC_QUEUE_LEN 8	

#define BROADCAST (logical_address<<4 | 0x0F)

typedef  struct 	/* CEC_CHAN * */
    {
    /* always goes first */

    void *     pDrvFuncs;      /* driver functions */

    /* callbacks */
    UINT32      (*getXmtFrame) (); /* pointer to xmitr function */	
    UINT32      (*putRcvFrame) (); /* pointer to rcvr function */
    void *      getXmtArg;
    void *      putRcvArg;

    UINT32 		*regs;			/* CEC registers */
    UINT32      regDelta;		/* register address spacing */
    UINT32      channelMode;	/* such as INT, POLL modes */
    
    UINT8		port;
    UINT8		txBusy;
	UINT8		rxMode;				/* normal/continue mode */
	UINT8		txMode;				/* normal/continue mode */

#if 0
	UINT8		dacClock;		/* DAC Clock */
	UINT8		logicAddr;		/* Logical Address */
	UINT8		clkDivisor;		/* Clock Divisor */
	UINT8		retryTime;		/* Retry Wait Time */
	UINT8		txMaxRetry;			/* Maximum re-transmission times */

	UINT8		rxStartLowMin;		/* minmum width (3.5ms) */
	UINT8		rxStartPeriodMax;	/* maximum width (4.7ms) */
	UINT8		rxSampleTime;		/* sample time (1.05ms) */
	UINT8		rxDataPeriod;		/* minimum data bit width (2.05ms) */
	UINT8		txStartLowMin;		/* minmum width (3.7ms = 0.025*148) */
	UINT8		txStartPeriodMax;	/* maximum width (0.8ms = 4.5ms-3.7ms) */
	UINT8		txSampleTime;		/* sample time (0.6ms) */
	UINT8		txDataPeriod;		/* minimum data bit width (0.9ms) */
	UINT8		txDataHigh;			/* transmit data high (0.9ms) */
#endif

    UINT32		options;	/* hardware setup options */
    
	UINT32		txPutIdx;	/* Tx Put by Driver */
	UINT32		txGetIdx;	/* Tx Get by HW */
	UINT32		rxPutIdx;	/* Rx put by HW */
	UINT32		rxGetIdx;	/* Rx get by driver */
    UINT8		tx_buf[CEC_FRAME_LEN * CEC_QUEUE_LEN];
    UINT8		rx_buf[CEC_FRAME_LEN * CEC_QUEUE_LEN];

    } CEC_CHAN;
    
#define TxIsEmpty(p)	(p->txPutIdx==p->txGetIdx)
#define TxIsFull(p)		(((p->txPutIdx+1)&(CEC_QUEUE_LEN-1))==p->txGetIdx)
#define RxIsEmpty(p)	(p->rxPutIdx==p->rxGetIdx)
#define RxIsFull(p)		(((p->rxPutIdx+1)&(CEC_QUEUE_LEN-1))==p->rxGetIdx)
    

CEC_CHAN	cec_chan[N_CEC_CHAN];
unsigned int physical_address;
unsigned int device_type = CEC_DEVTYPE_PLAYBACK;


#define CEC_BASE_ADDR	0xB005009C
#define CEC_DELTA_LEN	0x100
#define CEC_CR0			0x00
#define CEC_RTCR0		0x04
#define CEC_RxCR0		0x08
#define CEC_TxCR0		0x0C
#define CEC_TxDR		0x10
#define CEC_RxDR		0x14
#define CEC_RxTCR0		0x18
#define CEC_TxTCR0		0x1C
#define CEC_TxTCR1		0x20

#define CEC_CR0_ENABLE			(1<<30)
#define CEC_CR0_DIV_DEFAULT		(135<<16)
#define CEC_CR0_TEST_MODE		(1<<19)
#define CEC_CR0_DAC_DRIVE		(1<<18)
#define CEC_CR0_DAC_ENABLE		(1<<17)
#define CEC_CR0_DAC_DATA			(0x1f<<12)
#define CEC_CR0_LOGIC_ADDR		(0xf<<8)
#define CEC_CR0_CLK_DEFAULT		(20)

#define CEC_RXCR0_ENABLE		(1<<15)
#define CEC_RXCR0_RESET			(1<<14)
#define CEC_RXCR0_INT_EN		(1<<12)

#define CEC_TXCR0_ENABLE		(1<<15)
#define CEC_TXCR0_RESET			(1<<14)
#define CEC_TXCR0_INT_EN		(1<<12)

/*
 * Feature Opcode / CEC commands
 */
#define CEC_OP_FEATURE_ABORT				0x00
#define CEC_OP_IMAGE_VIEW_ON				0x04
#define CEC_OP_TUNER_STEP_INCREMENT			0x05
#define CEC_OP_TUNER_STEP_DECREMENT			0x06
#define CEC_OP_TUNER_DEVICE_STATUS			0x07
#define CEC_OP_GIVE_TUNER_DEVICE_STATUS		0x08
#define CEC_OP_RECORD_ON					0x09
#define CEC_OP_RECORD_STATUS				0x0A
#define CEC_OP_RECORD_OFF					0x0B
#define CEC_OP_TEXT_VIEW_ON					0x0D
#define CEC_OP_RECORD_TV_SCREEN				0x0F
#define CEC_OP_GIVE_DECK_STATUS				0x1A
#define CEC_OP_DECK_STATUS					0x1B
#define CEC_OP_SET_MENU_LANGUAGE			0x32
#define CEC_OP_CLEAR_ANALOGUE_TIMER			0x33
#define CEC_OP_SET_ANALOGUE_TIMER			0x34
#define CEC_OP_TIMER_STATUS					0x35
#define CEC_OP_STANDBY						0x36
#define CEC_OP_PLAY							0x41
#define CEC_OP_DECK_CONTROL					0x42
#define CEC_OP_TIMER_CLEARED_STATUS			0x43
#define CEC_OP_USER_CONTROL_PRESSED			0x44
#define CEC_OP_USER_CONTROL_RELEASED		0x45
#define CEC_OP_GIVE_OSD_NAME				0x46
#define CEC_OP_SET_OSD_NAME					0x47
#define CEC_OP_SET_OSD_STRING				0x64
#define CEC_OP_SET_TIMER_PROGRAM_TITLE		0x67
#define CEC_OP_SYSTEM_AUDIO_MODE_REQUEST	0x70
#define CEC_OP_GIVE_AUDIO_STATUS			0x71
#define CEC_OP_SET_SYSTEM_AUDIO_MODE		0x72
#define CEC_OP_REPORT_AUDIO_STATUS			0x7A
#define CEC_OP_GIVE_SYSTEM_AUDIO_MODE_STATUS 0x7D
#define CEC_OP_SYSTEM_AUDIO_MODE_STATUS		0x7E
#define CEC_OP_ROUTING_CHANGE				0x80
#define CEC_OP_ROUTING_INFORMATION			0x81
#define CEC_OP_ACTIVE_SOURCE				0x82
#define CEC_OP_GIVE_PHYSICAL_ADDRESS		0x83
#define CEC_OP_REPORT_PHYSICAL_ADDRESS		0x84
#define CEC_OP_REQUEST_ACTIVE_SOURCE		0x85
#define CEC_OP_SET_STREAM_PATH				0x86
#define CEC_OP_DEVICE_VENDOR_ID				0x87
#define CEC_OP_VENDOR_COMMAND				0x89
#define CEC_OP_VENDOR_REMOTE_BUTTON_DOWN	0x8A
#define CEC_OP_VENDOR_REMOTE_BUTTON_UP		0x8B
#define CEC_OP_GIVE_DEVICE_VENDOR_ID		0x8C
#define CEC_OP_MENU_REQUEST					0x8D
#define CEC_OP_MENU_STATUS					0x8E
#define CEC_OP_GIVE_DEVICE_POWER_STATUS		0x8F
#define CEC_OP_REPORT_POWER_STATUS			0x90
#define CEC_OP_GET_MENU_LANGUAGE			0x91
#define CEC_OP_SELECT_ANALOGUE_SERVICE		0x92
#define CEC_OP_SELECT_DIGITAL_SERVICE		0x93
#define CEC_OP_SET_DIGITAL_TIMER			0x97
#define CEC_OP_CLEAR_DIGITAL_TIMER			0x99
#define CEC_OP_SET_AUDIO_RATE				0x9A
#define CEC_OP_INACTIVE_SOURCE				0x9D
#define CEC_OP_CEC_VERSION					0x9E
#define CEC_OP_GET_CEC_VERSION				0x9F
#define CEC_OP_VENDOR_COMMAND_WITH_ID		0xA0
#define CEC_OP_CLEAR_EXTERNAL_TIMER			0xA1
#define CEC_OP_SET_EXTERNAL_TIMER			0xA2
#define CEC_OP_ABORT						0xFF

UINT32 bits[] = {0,\ 
	0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, \
	0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, \
	0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, \
	0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

#define REG(pchan,reg) \
 (*(volatile UINT32 *)((UINT32)pchan->regs + reg))
#define REGPTR(pchan,reg) \
 ((volatile UINT32 *)((UINT32)pchan->regs + reg))
 
#define SET_REG_BITS(r,s,e,v)	r=(r&~(bits[s-e+1]<<e))|(v<<e)
#define GET_BITS(r,s,e)	((r>>e)&~(0xffffffff<<(s-e+1)))
#define SET_REG_BIT(r,b,v)		r=(r&~(0x1<<b))|(v<<b)
#define GET_BIT(r,b)	((r>>b)&0x1)

#define CEC_SetEnable(p)			SET_REG_BITS(REG(p,CEC_CR0),31,30,1)
#define CEC_SetDisable(p)			SET_REG_BITS(REG(p,CEC_CR0),31,30,0)

#define CEC_GetDACClock(p)			GET_BITS(REG(p,CEC_CR0),15,8)
#define CEC_SetDACClock(p,v)		SET_REG_BITS(REG(p,CEC_CR0),15,8,v)
#define CEC_GetDACTestData(p)		GET_BITS(REG(p,CEC_CR0),4,0)
#define CEC_SetDACTestData(p,v)		SET_REG_BITS(REG(p,CEC_CR0),4,0,v)
#define CEC_GetLogicalAddress(p)	GET_BITS(REG(p,CEC_CR0),27,24)
#define CEC_SetLogicalAddress(p,v)	SET_REG_BITS(REG(p,CEC_CR0),27,24,v)
#define CEC_GetClockDivisor(p)		GET_BITS(REG(p,CEC_CR0),23,16)
#define CEC_SetClockDivisor(p,v)	SET_REG_BITS(REG(p,CEC_CR0),23,16,v)

#define CEC_GetRetryWaitTime(p)		GET_BITS(REG(p,CEC_RTCR0),10,5)
#define CEC_CheckTxLastInitiator(p)	GET_BIT(REG(p,CEC_RTCR0),4)
#define CEC_GetTxMaxRetransmit(p)	GET_BITS(REG(p,CEC_RTCR0),3,0)
#define CEC_SetTxMaxRetransmit(p,v)	SET_REG_BITS(REG(p,CEC_RTCR0),3,0,v)

#define CEC_GetRxEn(p)				GET_BIT(REG(p,CEC_RxCR0),15)
#define CEC_SetRxEn(p)				SET_REG_BIT(REG(p,CEC_RxCR0),15,1)
#define CEC_ClrRxEn(p)				SET_REG_BIT(REG(p,CEC_RxCR0),15,0)
#define CEC_SetRxReset(p)			SET_REG_BIT(REG(p,CEC_RxCR0),14,1)
#define CEC_ClrRxReset(p)			SET_REG_BIT(REG(p,CEC_RxCR0),14,0)
#define CEC_GetRxConti(p)			GET_BIT(REG(p,CEC_RxCR0),13)
#define CEC_GetRxEOM(p)				GET_BIT(REG(p,CEC_RxCR0),7)
#define CEC_SetRxIntEn(p)			SET_REG_BIT(REG(p,CEC_RxCR0),12,1)
#define CEC_ClrRxIntEn(p)			SET_REG_BIT(REG(p,CEC_RxCR0),12,0)
#define CEC_GetRxInt(p)				GET_BIT(REG(p,CEC_RxCR0),6)
#define CEC_ClrRxInt(p)				SET_REG_BIT(REG(p,CEC_RxCR0),6,1)
#define CEC_GetRxLen(p)				GET_BITS(REG(p,CEC_RxCR0),4,0)
#define CEC_GetInitiatorAddr(p)		GET_BITS(REG(p,CEC_RxCR0),11,8)


#define CEC_GetTxEn(p)				GET_BIT(REG(p,CEC_TxCR0),15)
#define CEC_SetTxEn(p)				SET_REG_BIT(REG(p,CEC_TxCR0),15,1)
#define CEC_ClrTxEn(p)				SET_REG_BIT(REG(p,CEC_TxCR0),15,0)
#define CEC_SetTxReset(p)			SET_REG_BIT(REG(p,CEC_TxCR0),14,1)
#define CEC_ClrTxReset(p)			SET_REG_BIT(REG(p,CEC_TxCR0),14,0)
#define CEC_GetTxIntEn(p)			GET_BIT(REG(p,CEC_TxCR0),12)
#define CEC_SetTxIntEn(p)			SET_REG_BIT(REG(p,CEC_TxCR0),12,1)
#define CEC_ClrTxIntEn(p)			SET_REG_BIT(REG(p,CEC_TxCR0),12,0)
#define CEC_GetTxInt(p)				GET_BIT(REG(p,CEC_TxCR0),6)
#define CEC_ClrTxInt(p)				SET_REG_BIT(REG(p,CEC_TxCR0),6,1)
#define CEC_GetTxEom(p)				GET_BIT(REG(p,CEC_TxCR0),7)
#define CEC_ClrTxEom(p)				SET_REG_BIT(REG(p,CEC_TxCR0),7,1)
#define CEC_GetTxLen(p)				GET_BITS(REG(p,CEC_TxCR0),4,0)
#define CEC_GetDestAddr(p)			GET_BITS(REG(p,CEC_TxCR0),11,8)
#define CEC_SetDestAddr(p,v)		SET_REG_BITS(REG(p,CEC_TxCR0),11,8,v)
#define CEC_GetTxAddr(p)			GET_BITS(REG(p,CEC_TxCR0),19,16)
#define CEC_SetTxAddr(p,v)			SET_REG_BITS(REG(p,CEC_TxCR0),19,16,v)
#define CEC_GetTxAddrEn(p)				GET_BIT(REG(p,CEC_TxCR0),20)
#define CEC_SetTxAddrEn(p)				SET_REG_BIT(REG(p,CEC_TxCR0),20,1)
#define CEC_ClrTxAddrEn(p)				SET_REG_BIT(REG(p,CEC_TxCR0),20,0)

#define CEC_GetRxStartLowMin(p)		GET_BITS(REG(p,CEC_RxTCR0),31,24)
#define CEC_SetRxStartLowMin(p,v)	SET_REG_BITS(REG(p,CEC_RxTCR0),31,24,v)
#define CEC_GetRxStartPeriodMax(p)	GET_BITS(REG(p,CEC_RxTCR0),23,16)
#define CEC_SetRxStartPeriodMax(p,v)	SET_REG_BITS(REG(p,CEC_RxTCR0),23,16,v)
#define CEC_GetRxDataSample(p)		GET_BITS(REG(p,CEC_RxTCR0),15,8)
#define CEC_SetRxDataSample(p,v)	SET_REG_BITS(REG(p,CEC_RxTCR0),15,8,v)
#define CEC_GetRxDataPeriodMin(p)	GET_BITS(REG(p,CEC_RxTCR0),7,0)
#define CEC_SetRxDataPeriodMin(p,v)	SET_REG_BITS(REG(p,CEC_RxTCR0),7,0,v)

#define CEC_GetTxStartLowMin(p)		GET_BITS(REG(p,CEC_TxTCR0),15,8)
#define CEC_SetTxStartLowMin(p,v)	SET_REG_BITS(REG(p,CEC_TxTCR0),15,8,v)
#define CEC_GetTxStartPeriodMax(p)	GET_BITS(REG(p,CEC_TxTCR0),7,0)
#define CEC_SetTxStartPeriodMax(p,v)	SET_REG_BITS(REG(p,CEC_TxTCR0),7,0,v)
#define CEC_GetTxDataSample(p)		GET_BITS(REG(p,CEC_TxTCR1),23,16)
#define CEC_SetTxDataSample(p,v)	SET_REG_BITS(REG(p,CEC_TxTCR1),23,16,v)
#define CEC_GetTxDataPeriodMin(p)	GET_BITS(REG(p,CEC_TxTCR1),15,8)
#define CEC_SetTxDataPeriodMin(p,v)	SET_REG_BITS(REG(p,CEC_TxTCR1),15,8,v)
#define CEC_GetTxDataHigh(p)		GET_BITS(REG(p,CEC_TxTCR1),7,0)
#define CEC_SetTxDataHigh(p,v)		SET_REG_BITS(REG(p,CEC_TxTCR1),7,0,v)

#define CEC_GET_DEST_ADDR				0x1001
#define CEC_SET_DEST_ADDR				0x1002
#define CEC_GET_DAC_CLOCK				0x1003
#define CEC_SET_DAC_CLOCK				0x1004
#define CEC_GET_CLOCK_DIVISOR			0x1005
#define CEC_SET_CLOCK_DIVISOR			0x1006
#define CEC_GET_TX_MAX_RETRANSMIT		0x1007
#define CEC_SET_TX_MAX_RETRANSMIT		0x1008
#define CEC_GET_RX_START_LOW_MIN		0x1009
#define CEC_SET_RX_START_LOW_MIN		0x100a
#define CEC_GET_RX_START_PERIOD_MAX		0x100b
#define CEC_SET_RX_START_PERIOD_MAX		0x100c
#define CEC_GET_RX_DATA_SAMPLE			0x100d
#define CEC_SET_RX_DATA_SAMPLE			0x100e
#define CEC_GET_RX_DATA_PERIOD_MIN		0x100f
#define CEC_SET_RX_DATA_PERIOD_MIN		0x1010
#define CEC_GET_TX_START_LOW_MIN		0x1011
#define CEC_SET_TX_START_LOW_MIN		0x1012
#define CEC_GET_TX_START_PERIOD_MAX		0x1013
#define CEC_SET_TX_START_PERIOD_MAX		0x1014
#define CEC_GET_TX_DATA_SAMPLE			0x1015
#define CEC_SET_TX_DATA_SAMPLE			0x1016
#define CEC_GET_TX_DATA_PERIOD_MIN		0x1017
#define CEC_SET_TX_DATA_PERIOD_MIN		0x1018
#define CEC_GET_TX_DATA_HIGH			0x1019
#define CEC_SET_TX_DATA_HIGH			0x101a
#define CEC_GET_SRC_ADDR				0x101b
#define CEC_SET_SRC_ADDR				0x101c
#define CEC_GET_PHYSICAL_ADDR			0x101d

