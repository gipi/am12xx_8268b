/***************************************************************************
	hdmi_reg.h
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          hdmi_drv.h  -  description
                             -------------------
    begin                : Monday January 18, 2005
    copyright            : (C) 2004 by Kao, Kuo-Chun
    email                : kckao@realtek.com.tw 
 ***************************************************************************/

/**
 * @file hdmi_drv.h
 * HDMI Source driver.
 *
 * @author Kao, Kuo-Chun
 * @email kckao@realtek.com.tw 
 * @date Monday January 18, 2005
 * @version 0.1
 * @ingroup os_2880_disp_interface
 *
 */

/*======================= CVS Headers =========================
  $Header: /cvsroot/dtv/linux-2.6/include/rtd2880/disp/hdmi_drv.h,v 1.1 2005/12/19 06:23:54 kckao Exp $
  
  $Log: hdmi_drv.h,v $
  Revision 1.1  2005/12/19 06:23:54  kckao
  moved from include/rtd2880/hdmi

  Revision 1.4  2005/07/08 05:52:45  kckao
  +: driver model

  Revision 1.3  2005/05/03 11:37:12  kckao
  no message

  Revision 1.2  2005/05/03 03:04:24  kckao
  no message

  Revision 1.1  2005/04/25 02:37:47  kckao
  *:initial version



 *=============================================================*/

#ifndef __LINUX_HDMI_SOURCE_H
#define __LINUX_HDMI_SOURCE_H


/* 
	DVI Quick Configuration
*/
struct _DVI_Config {
	UINT8 	vformat;  	/* Video Format ID for RTD2882 FW */
	UINT8	cformat;	/* Color Format Defined in CEA-861B */
};


/* 
	HDMI Generic Configuration (for debug)
*/
struct _HDMI_Generic {
	UINT8 	mode;  	/* Force mode for Debug, */
					/* 0:Disable, 1:DVI, 2:Enable */
	UINT8	hdcp;	/* hdcp enable/disable */
	 				/* 1:disable, 0:enable */
 	UINT8	audio; 	/* Audio Enable / Disable */
					/* in HDMI mode */
 	UINT8	avmute; /* AV Mute Enable / Disable */
					/* in HDMI mode */
	UINT8	Vsync_Polarity; 
	UINT8	Hsync_Polarity; 	
	UINT8	Vsync_Polarity_INV;/* 1:INV enable , 0:Disable */
	UINT8	Hsync_Polarity_INV;/* 1:INV enable , 0:Disable */
	UINT8	pixelColorFormatByUser; /*0: RGB444, 1:YCbCr422, 2:YcbCr444, 0x20: auto-detect.*/
};



/* 
	HDCP Private Keys List
*/
struct _HDCP_PrivateKey {
	UINT8	pnt;
	UINT8	ksv[5];
		/* Private Key Encryption Pattern */
	UINT8	key[40][7];	
		/* 40 Private keys 56 bits, 
			byte order in Little-Endian */
};

/* 
	HDCP AN Influence Seed
*/
struct _HDCP_RNG_Seed {
	UINT32 msw; 	/* most significant word of 				
						64 bits seed in big-endian */
	UINT32 lsw; 	/* last significant word of 
						64 bits seed in big-endian */
};


/* 
	AVI InfoFrame
	description in CEA-861B 6.1
*/
struct _AVI_Infoframe {
	UINT8	version;			/* version 1/2 */
	UINT8	cformat;			/* pixel format */
	UINT8	bar_info;			/* Bar Info */
	UINT8	scan_info;			/* Scan Information */
	UINT8	colorimetry;		/* Colorimetry */
	UINT8	pic_ratio;			/* Picture Aspect Ratio */
	UINT8	act_pic_ratio;		/* Active Format Aspect Ratio */
	UINT8	quan_range;			/* Quantization Range */
	UINT8	scaling;			/* Non-Uniform Picture Scaling */
	
	UINT8	video_id;			/* Video Identification Codes (Ver2) */
	UINT8	repeat;				/* Pixel Repetition for Optional (Ver2) */
	
	UINT16	top_bar_end;		/* Line Number of End of Top Bar */
	UINT16	bottom_bar_start;	/* Line Number of End of Bottom Bar */
	UINT16	left_bar_end;		/* Pixel Number of End of Left Bar */
	UINT16	right_bar_start;	/* Pixel Number of Start of Right Bar */
};

/* 
	Audio InfoFrame
	description in CEA-861B 6.3
*/
struct _Audio_InfoFrame {
	UINT8	version;	/* Version */
	UINT8	type;		/* Audio Coding Type */
	UINT8	ch;			/* Audio Channel Count */
	UINT8	rate;		/* Sampling Frequency */
	UINT8	bits;		/* Sample Size */
	UINT8	max_bit_rate;	/* Maximum bit rate divided by 8kHz (for compressed format */
	UINT8	speaker;	/* Speaker Placement */
	UINT8	lsv;		/* Level Shift Value */
	UINT8	dm_inh;		/* Down-mix Inhibit Flag */
};

/* 
	Source Product Description (SPD) InfoFrame
	description in CEA-861B 6.2
*/
struct _SPD_InfoFrame {
	UINT8	version;	/* Version */
	UINT8	device;		/* Source Device Infomation */
	UINT8	name[9];	/* Product Name (8 bytes) */
	UINT8	desc[17];	/* product Description (16 bytes) */
};
/* 
	MPEG InfoFrame
	description in CEA-861B 6.2
*/
struct _MPEG_InfoFrame {
	UINT8	version;	/* Version */
	UINT8	bit_rate[5];	/* Bit rate (4 bytes) */
	UINT8	repeat;	
	UINT8	Frame;	
};




/* the total langth of hdcp key in flash (ROM).  thsi define is used by kernal (rtd2885_spiFlash.c */
#define HDMI_HDCP_PRIVATE_KEY_LENGTH 288
#define CONFIG_NOR_ROM_DATA_SIZE 288

/*enum HDMI_MODE {
	HDMI_MODE_DISABLE = 0,
	HDMI_MODE_DVI,
	HDMI_MODE_HDMI,
};*/

enum HDCP_MODE {
	HDMI_HDCP_DISABLE = 0,
	HDMI_HDCP_ENABLE,
};

enum HDMI_AUDIO {
	HDMI_AUDIO_DISABLE = 0,
	HDMI_AUDIO_ENABLE,
};

enum HDMI_AVMUTE {
	HDMI_AVMUTE_DISABLE = 0,
	HDMI_AVMUTE_ENABLE,
};



enum HDCP_XMT {
	HDCP_XMT_LINK_H0 = 0,
	HDCP_XMT_LINK_H1,
	HDCP_XMT_LINK_H2,
	HDCP_XMT_LINK_H3,
	HDCP_XMT_AUTH_A0,
	HDCP_XMT_AUTH_A1,
	HDCP_XMT_AUTH_A2,
	HDCP_XMT_AUTH_A3,
	HDCP_XMT_AUTH_A4,
	HDCP_XMT_AUTH_A5,
	HDCP_XMT_AUTH_A6,
	HDCP_XMT_AUTH_A8,
	HDCP_XMT_AUTH_A9,
};

enum HDCP_AUTH_RESULT {
	HDCP_AUTH_FAIL = 0,
	HDCP_AUTH_SUCCESS,
	HDCP_AUTH_UNKNOW_RESULT,
	HDMI_HPD_PLUG,
	HDMI_HPD_UNPLUG,
	HDMI_TX_NULL_KEY
};

INT32 hdmi_setVideo(INT8 mode, struct _AVI_Infoframe *video);
INT32 hdmi_gen_avi_infoframe(struct _AVI_Infoframe *video);

#endif
