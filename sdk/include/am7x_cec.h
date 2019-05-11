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


/* Device Type */
#define CEC_DEVTYPE_TV				0x00
#define CEC_DEVTYPE_RECORDING		0x01
#define CEC_DEVTYPE_RESERVED		0x02
#define CEC_DEVTYPE_TUNER			0x03
#define CEC_DEVTYPE_PLAYBACK		0x04
#define CEC_DEVTYPE_AUDIO			0x05
#define CEC_DEVTYPE_FREEUSE			0x06

#define BROADCAST (logical_address<<4 | 0x0F) 
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

