/***************************************************************************
	hdmi_opr.h
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          hdcp_drv.h  -  description
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
#ifndef __HDCP_DRV_H
#define __HDCP_DRV_H

//extern void hdcp_XmtState(UINT32 tmr_data);
INT32 hdcp_CheckRiUpdate(void);
INT32 hdcp_ReadRip(UINT8 *rip);
INT32 hdcp_CheckAuthentication(void);
INT32 hdcp_CheckAuthentication_Pj(void);
INT32 hdcp_ForceUnauthentication(void);
INT32 hdcp_CheckPjUpdate(void);
INT32 hdcp_ReadPjp(UINT8 *pjp);
INT32 hdcp_timer_init(void);
INT32 hdcp_timer_stop(void);
INT32 hdcp_loadAnInfluenceSeed(struct _HDCP_RNG_Seed *seed);
INT32 hdcp_sendHDCPAuthResultMessageToAP(INT32 result);
INT32 hdcpXmtState_thread_init(void);
INT32 hdcpXmtState_thread_kill(void);
void runHDCPXmtState(unsigned long tmp);
BOOL hdcp_forceUnauthentication(BOOL isforceUnauthentication);

#endif

