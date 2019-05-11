/***************************************************************************
	hdmi_opr.h
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
 * @ingroup pres_hdmi
 *
 */

/*======================= CVS Headers =========================
  $Header: /cvsroot/dtv/linux-2.6/drivers/rtd2880/hdmi/hdmi_opr.h,v 1.3 2005/11/25 03:33:36 kckao Exp $
  
  $Log: hdmi_opr.h,v $
  Revision 1.3  2005/11/25 03:33:36  kckao
  HDMI Tx Driver tested with RTD2880 Demo Board

  Revision 1.2  2005/07/08 05:56:06  kckao
  *: splited header file for driver, user level definetion are in hdmi_drv.h


  Revision 1.1  2005/04/25 02:37:47  kckao
  *:initial version



 *=============================================================*/

#ifndef __LINUX_HDMI_OPER_H
#define __LINUX_HDMI_OPER_H



struct _HDMI_Oper {
	struct _HDMI_Generic	generic;
	struct _AVI_Infoframe	video;
	struct _Audio_InfoFrame	audio;
	struct _SPD_InfoFrame	spd;
	struct _HDCP_PrivateKey	pk;
	struct _HDCP_RNG_Seed	seed;
	struct _MPEG_InfoFrame	mpeg;
};


struct hdcp_oper {
	UINT8	state;
	UINT8	repeater;
	UINT8	retry;	
	UINT8	Aksv[5];
	UINT8	Bksv[5];
	UINT8	An[8];
	UINT8	Ri[2];
	UINT8	Rip[2];
	UINT8	Pj;
	UINT8	Pjp;
	UINT8	V[20];
	UINT8	Vp[20];
	UINT16	llen;
	UINT8	M0[8];
	//UINT8	Bstatus;//mark by keith
	UINT8	Bstatus[2];//add by keith, port 41: Bstatus[0], port 42: Bstatus[1]
	UINT8	ksvList[128*5];
	UINT8	HDCPAuthResult;//add by keith, 0: FAIL, 1:SUCCESS
	UINT8	HDCPAuthFailCounter;
	UINT8	isVideoFlash;//video flash when hdcp auth fail.
	UINT8	isForceUnauthentication;
};



enum EDID_DESC {
	EDID_HEADER = 0,
	EDID_AUDIO,
	EDID_VIDEO,
	EDID_VENDOR,
	EDID_SPEAKER,
	EDID_TIMING,
};


#endif
