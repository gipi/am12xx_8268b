/***************************************************************************
	hdmi_opr.h
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          hdmi_ddc.h  -  description
                             -------------------
    begin                : Monday January 18, 2005
    copyright            : (C) 2004 by Kao, Kuo-Chun
    email                : kckao@realtek.com.tw 
 ***************************************************************************/

/**
 * @file island.h
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
  $Header:  $
 *=============================================================*/
#ifndef __HDMI_DDC_H
#define __HDMI_DDC_H


//extern INT32 hdmi_get_edid(unsigned char *dbuf);
//extern INT32 hdcp_ddc_init(void);
extern INT32 hdcp_ReadBksv(UINT8 *Bksv);
extern INT32 hdcp_TriggerRxAuth(UINT8 *Aksv, UINT8 *An);
extern INT32 hdcp_CheckRepeater(void);
extern INT32 hdcp_ReadRip(UINT8 *rip);
extern INT32 hdcp_CheckKsvListReady(void);
extern INT32 hdcp_ReadKsvList(UINT8 *Bstatus, UINT8 *ksvlist);
extern INT32 hdcp_ReadVprime(UINT8 *Vp);

#endif


