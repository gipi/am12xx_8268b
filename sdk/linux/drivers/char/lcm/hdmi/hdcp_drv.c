/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          hdcp_drv.c  -  description
                             -------------------
    begin                : Monday January 18, 2005
    copyright            : (C) 2004 by Kao, Kuo-Chun
    email                : kckao@realtek.com.tw 
 ***************************************************************************/

/**
 * @file hdcp_drv.c
 * HDCP Source driver.
 *
 * @author Kao, Kuo-Chun
 * @email kckao@realtek.com.tw 
 * @date Monday January 18, 2005
 * @version 0.1
 * @ingroup pres_hdmi
 *
 */

/*======================= CVS Headers =========================
  $Header: /cvsroot/dtv/linux-2.6/drivers/rtd2880/hdmi/hdcp_drv.c,v 1.6 2005/12/23 07:53:13 kckao Exp $
  

  Revision 1.1  2005/04/25 02:37:47  kckao
  *:initial version



 *=============================================================*/
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif

#if MODULE_CONFIG_HDMI_HDCP_ENABLE
//#ifdef __LINUX__
#include <linux/config.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/init.h>
#include "rtd_types.h"
//#endif

/* HDMI Tx include File */
#include "hdmi_drv.h"
/* HDMI Tx local include File */
#include "hdmi_opr.h"
#include "hdmi_reg.h"
#include "hdmi_ddc.h"
#include "hdcp_drv.h"



extern INT32 hdmi_hot_plug_test(void);
extern INT32 loadHDCPPrivateKeys_form_Flash(void);

#define ENABLE_HDCP_EESS 0
#define HDCP_MSG 0

#if 0
#define DPRINTK(format, args...) \
	printk("HDMI TX: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

extern struct _HDMI_Oper	hdmiDB;
struct hdcp_oper hdcpOper;


#define GET_BITS(r,s,e)	((r>>e)&~(0xffffffff<<(s-e+1)))
#define GET_BIT(r,b)	((r>>b)&1)


/* 
	SHA-1 calculation 

*/

#define SXORN(n,x)	((x<<n)|(x>>(32-n)))

#define FUN00_19(B,C,D)	((B&C)|((~B)&D))
#define FUN20_39(B,C,D)	(B^C^D)
#define FUN40_59(B,C,D)	((B&C)|(B&D)|(C&D))
#define FUN60_79(B,C,D)	(B^C^D)


unsigned int W[80];
#define swapV(x)	((((x)&0x000000ff)<<24)+(((x)&0x0000ff00)<<8)+(((x)&0x00ff0000)>>8)+(((x)&0xff000000)>>24))
#if 1 //for mips target 
#define swaps(x)	(x)
#define swapl(x)	(x)
#else //for x86
#define swaps(x)	((((x)&0xff00)>>8)+(((x)&0x00ff)<<8))
#define swapl(x)	((((x)&0x000000ff)<<24)+(((x)&0x0000ff00)<<8)+(((x)&0x00ff0000)>>8)+(((x)&0xff000000)>>24))
#endif

static struct timer_list hdcp_state_timer;

INT32 do_sha_1(UINT8 *v, INT32 len, UINT8 *data)
{
	unsigned int Kt[4] = {
		0x5a827999,
		0x6ed9eba1,
		0x8f1bbcdc,
		0xca62c1d6
	};



	unsigned int h[5] = {
		0x67452301,
		0xefcdab89,
		0x98badcfe,
		0x10325476,
		0xc3d2e1f0
	};

	int t, n, n_block;
	unsigned int Ft, x, temp;
	unsigned int A, B, C, D, E;
	int i;
	int len1 = swapl(len);

	/* do message padding */
	n_block = (len + 1 + 64)/512 + 1;
	data[len/8] = 0x80;
	/* set len */
	memcpy(&data[(n_block-1)*64+60], &len1, 4);

	#if 0
	printf("no. block = %d\n", n_block);
	for(n=0; n<n_block; n++) {
		for(i=0; i<16; i++)
			printf("%02x ", data[n*64+i]);
		printf("\n");
		for(i=16; i<32; i++)
			printf("%02x ", data[n*64+i]);
		printf("\n");
		for(i=32; i<48; i++)
			printf("%02x ", data[n*64+i]);
		printf("\n");
		for(i=48; i<64; i++)
			printf("%02x ", data[n*64+i]);
		printf("\n");
	}
	#endif
	
	for(n=0; n<n_block; n++) {
		for(t=0; t<16; t++) {
			x = *((unsigned long *)&data[n*64+t*4]);
			W[t] = swapl(x);
		}
		for(t=16; t<80; t++) {
			x = W[t-3] ^ W[t-8] ^ W[t-14] ^W[t-16];
			W[t] = SXORN(1, x);
		}

			A = h[0];
			B = h[1];
			C = h[2];
			D = h[3];
			E = h[4];

		for(t=0; t<80; t++) {
			if(t>=0 && t<=19)
				Ft = FUN00_19(B, C, D);
			else if(t>=20 && t<=39)
				Ft = FUN20_39(B, C, D);
			else if(t>=40 && t<=59)
				Ft = FUN40_59(B, C, D);
			else 
				Ft = FUN60_79(B, C, D);
		
			/* temp = S^5(A) + f(t;B,C,D) + E + W(t) + K(t) */
			temp = SXORN(5, A) + Ft + E + W[t] + Kt[t/20];
	
			E = D;
			D = C;
			/* C = S^30(B) */
			C = SXORN(30, B);
			B = A;
			A = temp;

#if 0
			printf("t = %d, A = %08x, B = %08x, C = %08x, D = %08x, E = %08x\n", \
					t, A, B, C, D, E);
#endif			
		}
		/* H0 = H0 + A */
		h[0] += A;
		/* H1 = H1 + B */
		h[1] += B;
		/* H2 = H2 + C */
		h[2] += C;
		/* H3 = H3 + D */
		h[3] += D;
		/* H4 = H4 + E */
		h[4] += E;
	}

	/* conert to little endien */	
	for(i=0; i<5; i++)
		h[i] = swapl(h[i]);

	/* conerting for match Vp reading from Rx */ //add by keith
	for(i=0; i<5; i++)
		h[i] = swapV(h[i]);
	
	/* copy to output pointer */
	memcpy(v, h, 20);

	return 0;
}


//add by keith
#define M0_LENGTH		8
#define BSTATUS_LENGTH	2
#define KSV_LENGTH		5
#define MAX_SHA_1_INPUT_LENGTH  704
INT32 generate_V(UINT8 *v, UINT8 *ksvlist, UINT8 *bstatus, UINT8 * m0){
	INT32 data_counter;

	UINT8 sha_1_input_data[MAX_SHA_1_INPUT_LENGTH];
	
	int cnt2=GET_BITS(hdcpOper.Bstatus[0], 6, 0);
	
	hdcpOper.llen = 8 * M0_LENGTH + 8 * BSTATUS_LENGTH + cnt2 * 8 * KSV_LENGTH;

	memset(sha_1_input_data, 0, MAX_SHA_1_INPUT_LENGTH);

	for(data_counter=0; data_counter < cnt2 * KSV_LENGTH +  BSTATUS_LENGTH + M0_LENGTH; data_counter++){
		if(data_counter < cnt2 * KSV_LENGTH) {
			
			sha_1_input_data[data_counter] = ksvlist[data_counter];
			
		} else if( (data_counter >=  cnt2 * KSV_LENGTH) && (data_counter <  cnt2 * KSV_LENGTH +  BSTATUS_LENGTH ) ){
		
			sha_1_input_data[data_counter] = bstatus[ data_counter - (cnt2 * KSV_LENGTH) ];
				
		} else {
			
			sha_1_input_data[data_counter] = m0[ data_counter - (cnt2 * KSV_LENGTH +  BSTATUS_LENGTH) ];
			
		}		
	}
	
	
	do_sha_1(v, hdcpOper.llen, sha_1_input_data);
	
	return 0;
}


/* convert INT8 number to little endien number */
INT32 c2ln(UINT8 *num, INT8 *a)
{
INT32 i;
INT32 n = strlen(a);
	memset(num, 0, 11);
	for(i=0; i<n; i++) {
		if(i%2) {
			if(a[n-i-1]>='0' && a[n-i-1]<='9')
				num[i/2] |= (a[n-i-1] - '0')<<4;
			else if (a[n-i-1]>='a' && a[n-i-1]<='f')
				num[i/2] |= (a[n-i-1] - 'a' + 10)<<4;
		} else {
			if(a[n-i-1]>='0' && a[n-i-1]<='9')
				num[i/2] |= (a[n-i-1] - '0');
			else if (a[n-i-1]>='a' && a[n-i-1]<='f')
				num[i/2] |= (a[n-i-1] - 'a' + 10);
		}
	}
	return 0;
}


/**

 * This function encrypt 1 private key with a 8 bits pattern

 * This function encrypt 1 private key with a 8 bits pattern
 *

 * @param *key	Private key in little endian bytes
 * @param pnt	encryption pattern
 * 

 * @return
 process status
 *

 * @note
	 twice encryptions will get the original key
 *

 * @author 	Casey
 * @date 	2005/03/21

 * @version 	1.0

 */

INT32 hdcp_encPrivateKey(UINT8 *key, UINT8 pnt)
{
UINT8 dKey[11];
	dKey[0] = key[0] ^ pnt;
	dKey[1] = ~key[1] ^ dKey[0];
	dKey[2] = key[2] ^ dKey[1];
	dKey[3] = key[3] ^ dKey[2];
	dKey[4] = key[4] ^ dKey[3];
	dKey[5] = ~key[5] ^ dKey[4];
	dKey[6] = ~key[6] ^ dKey[5];
	/* write to HW */
	HDCP_REG_DPKLR = pnt | (dKey[0]<<8) | (dKey[1]<<16) | (dKey[2]<<24);
	HDCP_REG_DPKMR = dKey[3] | (dKey[4]<<8) | (dKey[5]<<16) | (dKey[6]<<24);
	/* wait HW accumulation */
	while(!(HDCP_REG_SR & (1<<3)));
	return 0;
}


/**

 * This function set a encrypted private key to HDCP module
 * This function set a encrypted private key to HDCP module
 *

 * @param *key	encrypted Private key in little endian bytes
 * @param pnt	encryption pattern
 * 

 * @return
 process status
 *

 * @note
	 
 *

 * @author 	Casey
 * @date 	2005/03/21

 * @version 	1.0

 */


INT32 hdcp_setPrivateKey(UINT8 *key, UINT8 pnt)
{
	/* write to HW */
	HDCP_REG_DPKLR = pnt | (key[0]<<8) | (key[1]<<16) | (key[2]<<24);
	HDCP_REG_DPKMR = key[3] | (key[4]<<8) | (key[5]<<16) | (key[6]<<24);
	/* wait HW accumulation */
	while(!(HDCP_REG_SR & (1<<3)));
	return 0;
}

/*
	Debug Function
	we should encrypt the private keys when it is saved to stored device.
	When HDCP startup, it should set encrypted private key to HDCP module
	
*/
INT32 hdcp_debugGenerateKm(UINT8 *Bksv, struct _HDCP_PrivateKey *pk)
{
INT32 i, j;
	for(i=0; i<5; i++) {
		for(j=0; j<8; j++) {
			if(Bksv[i] & (1<<j)) {
				hdcp_encPrivateKey(pk->key[i*8+j], pk->pnt);
			}
		}
	}
	return 0;
}

INT32 hdcp_GenerateKm(UINT8 *Bksv, struct _HDCP_PrivateKey *pk)
{
INT32 i, j;
	for(i=0; i<5; i++) {
		for(j=0; j<8; j++) {
			if(Bksv[i] & (1<<j)) {
				hdcp_setPrivateKey(pk->key[i*8+j], pk->pnt);
			}
		}
	}
	return 0;
}


/*
	HDCP cipher RNG Generation Function
*/
INT32 hdcp_loadAnInfluenceSeed(struct _HDCP_RNG_Seed *seed)
{
	/* set An Influence Seed at An Influence Registers */
	HDCP_REG_AnIMR = seed->msw;
	HDCP_REG_AnILR = seed->lsw;
	/* trigger loading AnInfluence seed */
	HDCP_REG_CR |= HDCP_CR_SEED_LOAD;

	/*waiting for done*/
	while( (HDCP_REG_CR&HDCP_CR_SEED_LOAD) );//add by keith, for solving the problem about R0!= R0p sometimes.
	
	return 0;
}

INT32 hdcp_FreeRunGetAn(UINT8 *an)
{

	/* Get An */
	/* get An influence from CRC64 */
	HDCP_REG_CR |= HDCP_CR_AN_INF_REQ;
		
	/* set An Influence Mode, influence will be load from AnIR0, AnIR1 */
	HDCP_REG_CR |= HDCP_CR_LOAD_AN;
	/* trigger to get An */
	HDCP_REG_CR |= HDCP_CR_AUTH_REQ;
	while(!(HDCP_REG_SR & HDCP_SR_AN_READY));
		//printf("wait An\n");
	/* leave An influence mode */
	HDCP_REG_CR &= ~HDCP_CR_LOAD_AN;

	/* 	Convert HDCP An from bit endien to little endien 
		HDCP An should stored in little endien, 
		but HDCP HW store in bit endien. */
	an[0] = HDCP_REG_AnLR & 0xff;
	an[1] = (HDCP_REG_AnLR>>8) & 0xff;
	an[2] = (HDCP_REG_AnLR>>16) & 0xff;
	an[3] = (HDCP_REG_AnLR>>24) & 0xff;
	an[4] = HDCP_REG_AnMR & 0xff;
	an[5] = (HDCP_REG_AnMR>>8) & 0xff;
	an[6] = (HDCP_REG_AnMR>>16) & 0xff;
	an[7] = (HDCP_REG_AnMR>>24) & 0xff;

	return 0;
}

/* 
	hdcp_AuthenticationSequence apply authentication sequence,
	run 48+56 pixel clock with An, REPEATER bit, Bksv and selected private keys
	generate R0, and return R0

*/
INT32 hdcp_AuthenticationSequence(void)
{

	/* force Encryption disable */
	HDCP_REG_CR &= ~HDCP_CR_ENC_ENABLE;
	/* reset Km accumulation */
	HDCP_REG_CR |= HDCP_CR_RESET_KM_ACC;
	
	/* set Bksv to accumulate Km */
	hdcp_GenerateKm(hdcpOper.Bksv, &hdmiDB.pk);

	DPRINTK("Bksv = 0x%02x%02x%02x%02x%02x,   km = %08x%08x\n",
		hdcpOper.Bksv[0], hdcpOper.Bksv[1], hdcpOper.Bksv[2], hdcpOper.Bksv[3], hdcpOper.Bksv[4],
		HDCP_REG_KMMR, HDCP_REG_KMLR);
		
	/* disable Ri update interrupt */
	HDCP_REG_CR &= ~HDCP_CR_RI_INT;
	/* clear Ri updated pending bit */
	HDCP_REG_SR |= HDCP_SR_RI_UPDATE;
	/* trigger hdcpBlockCipher at authentication */
	HDCP_REG_CR |= HDCP_CR_AUTH_COMP;
	/* wait 48+56 pixel clock to get R0 */
	while(!(HDCP_REG_SR & HDCP_SR_RI_UPDATE));
		//printf("wait Ri\n");
	//HDCP_REG_CR |= HDCP_CR_AUTHENTICATED;
	/* get Ri */
	hdcpOper.Ri[0] = (HDCP_REG_LIR>>16)&0xff;
	hdcpOper.Ri[1] = (HDCP_REG_LIR>>24)&0xff;
		
	return 0;
}

INT32 hdcp_ForceUnauthentication(void)
{
//volatile int delay=0;
	/* disable link integry check */
	HDCP_REG_CR &= ~HDCP_CR_RI_INT;
	HDCP_REG_CR &= ~HDCP_CR_PJ_INT;

	/* force Encryption disable */
	HDCP_REG_CR &= ~HDCP_CR_ENC_ENABLE;
	/* force HDCP module to unauthenticated state */
	HDCP_REG_CR |= HDCP_CR_FORCE_UNAUTH;
	
	hdcpOper.state = HDCP_XMT_AUTH_A0;

	DPRINTK("hdcp_ForceUnauthentication done \n");
	return 0;
}

INT32 hdcp_CheckAuthentication(void)
{
	if((hdcpOper.Ri[0] == hdcpOper.Rip[0]) && (hdcpOper.Ri[1] == hdcpOper.Rip[1]))
		return 0;
	return -1;
}

INT32 hdcp_CheckAuthentication_Pj(void)
{
	if( hdcpOper.Pj == hdcpOper.Pjp )
		return 0;
	return -1;
}

INT32 hdcp_SetAuthenticated(void)
{
	/* set HDCP module to authenticated state */
	HDCP_REG_CR |= HDCP_CR_AUTHENTICATED;
	/* start encryption */
	HDCP_REG_CR |= HDCP_CR_ENC_ENABLE;
	
	return 0;

}


INT32 hdcp_AuthenticationPart3Init(void)
{

	/* encryption enable */
	HDCP_REG_CR |= HDCP_CR_ENC_ENABLE;

	/* Enable Ri update interrupt */
	HDCP_REG_CR |= HDCP_CR_RI_INT;
	/* Initialize Ri Checking base */

	/* Initialize HDCP 1.1 feature */
	/* Enable Pj update interrupt */
	HDCP_REG_CR |= HDCP_CR_PJ_INT;
	/* Initialize Pj Checking base */
		
	return 0;
}

INT32 hdcp_AuthenticationPart3Exit(void)
{

	/* encryption enable */
	HDCP_REG_CR &= ~HDCP_CR_ENC_ENABLE;

	/* Disable Ri update interrupt */
	HDCP_REG_CR &= ~HDCP_CR_RI_INT;
	/* Flush Ri Checking base */

	/* Initialize HDCP 1.1 feature */
	/* Disable Pj update interrupt */
	HDCP_REG_CR &= ~HDCP_CR_PJ_INT;
	/* Flush Pj Checking base */
		
	return 0;
}
extern unsigned int outstanding_HDCP_SR;

INT32 hdcp_CheckRiUpdate(void)
{
int Ri;
	if(outstanding_HDCP_SR & HDCP_SR_RI_UPDATE) {
		outstanding_HDCP_SR &= ~HDCP_SR_RI_UPDATE;
		/* get Ri value */
		Ri = (HDCP_REG_LIR>>16);
		/* record Ri to checking base */
		hdcpOper.Ri[0] = Ri & 0xff;
		hdcpOper.Ri[1] = (Ri>>8) & 0xff;
		return 1;
	}
	return 0;
}

INT32 hdcp_CheckPjUpdate(void)
{
int Pj;
	if(outstanding_HDCP_SR & HDCP_SR_PJ_UPDATE) {
		outstanding_HDCP_SR &= ~HDCP_SR_PJ_UPDATE;
		/* get Ri value */
		Pj = (HDCP_REG_LIR>>8)&0xff;
		/* record Ri to checking base */
		hdcpOper.Pj = Pj;
		return 1;
	}
	return 0;
}

INT32 hdcp_CheckRxExist(void)
{

	//hdcp_ddc_init();
	return hdcp_ReadBksv(hdcpOper.Bksv);

}



INT32 hdcp_AuthenticationPart1(void)
{

		
	if(hdcp_ReadBksv(hdcpOper.Bksv))
		return -1;
	DPRINTK("hdcp_AuthenticationSequence()\n");
	hdcp_AuthenticationSequence();
	DPRINTK("hdcp_CheckAuthentication()\n");
	if(hdcp_CheckAuthentication())
		return -1;
	DPRINTK("hdcp_SetAuthenticated()\n");
	hdcp_SetAuthenticated();
	return 0;
}

#ifdef HDCP_MSG	
#define FAIL_CHDCK_NUMBER 3
INT32 hdcp_sendHDCPAuthResultMessageToAP(INT32 result){
#if 0
	SC_MSGBUF msgBuf;
	switch(result){
		case HDCP_AUTH_FAIL:
			if(hdcpOper.HDCPAuthFailCounter < FAIL_CHDCK_NUMBER) hdcpOper.HDCPAuthFailCounter++;
			
			if( (hdcpOper.HDCPAuthResult != HDCP_AUTH_FAIL) && (hdcpOper.HDCPAuthFailCounter == FAIL_CHDCK_NUMBER) ) {
				hdcpOper.HDCPAuthResult= HDCP_AUTH_FAIL;
				hdcpOper.HDCPAuthFailCounter++;
				memset(&msgBuf, 0, sizeof(SC_MSGBUF));
				msgBuf.msgType = SC_AVP_MSG;
				msgBuf.msgSubType = MSG_SYS_HDCP_AUTHENTICATION; 
				msgBuf.msgIntData = HDCP_AUTH_FAIL;
				//msgBuf.msgBodyByteSize = 0;
				//msgBuf.msgDataPtr = NULL;
				//msgBuf.isValidate = TRUE;
				Kernel_SendNlMsg( &msgBuf );
				DPRINTK("send MSG_SYS_HDCP_AUTHENTICATION = %d \n", msgBuf.msgIntData);				
			}
			return 0;

		case HDCP_AUTH_SUCCESS:
			hdcpOper.isVideoFlash = 0;
			hdcpOper.HDCPAuthFailCounter = 0;			
			if( hdcpOper.HDCPAuthResult != HDCP_AUTH_SUCCESS) {
				hdcpOper.HDCPAuthResult= HDCP_AUTH_SUCCESS;
				memset(&msgBuf, 0, sizeof(SC_MSGBUF));
				msgBuf.msgType = SC_AVP_MSG;
				msgBuf.msgSubType = MSG_SYS_HDCP_AUTHENTICATION; 
				msgBuf.msgIntData = HDCP_AUTH_SUCCESS;
				//msgBuf.msgBodyByteSize = 0;
				//msgBuf.msgDataPtr = NULL;
				//msgBuf.isValidate = TRUE;
				Kernel_SendNlMsg( &msgBuf );
				DPRINTK("send MSG_SYS_HDCP_AUTHENTICATION = %d \n", msgBuf.msgIntData);
			}
			return 0;

		case HDMI_HPD_PLUG:
		case HDMI_HPD_UNPLUG:
			hdcpOper.isVideoFlash = 0;
			hdcpOper.HDCPAuthFailCounter = 0;
			hdcpOper.HDCPAuthResult = HDCP_AUTH_UNKNOW_RESULT;
			memset(&msgBuf, 0, sizeof(SC_MSGBUF));
			msgBuf.msgType = SC_AVP_MSG;
			msgBuf.msgSubType = MSG_SYS_HDCP_AUTHENTICATION; 
			msgBuf.msgIntData = result;
			//msgBuf.msgBodyByteSize = 0;
			//msgBuf.msgDataPtr = NULL;
			//msgBuf.isValidate = TRUE;
			Kernel_SendNlMsg( &msgBuf );		
			return 0;			
			
		default:
			return -1;
	}			
	return 0;
#endif
}
#endif


static INT32 retry_times_for_set_up_5_second = 0;
static void hdcp_XmtState(unsigned long tmr_data)
{
INT32 link = hdmi_hot_plug_test();
	DPRINTK("In hdcp_XmtState ... \n");
	//DPRINTK("link = %d\n", link);
	//DPRINTK("state = %d\n", hdcpOper.state);
	
	if(!link) {
		/* check HDMI enable */
		/* check HDCP enable */
		/* check in next second */		
		if(hdcpOper.state >= HDCP_XMT_AUTH_A0)
			hdcp_ForceUnauthentication();		
		hdcpOper.state = HDCP_XMT_LINK_H0;
		HDMI_REG_CR &= ~0x00000001;	//disable HDMI for prodecting AV context, add by keith
		DPRINTK("hdcp_XmtState: no link\n");
		return;
	}

	//If AVMute is ON, no HDCP Auth.
	//DPRINTK("AVMUTE is %d \n", hdmiDB.generic.avmute);
	if(hdmiDB.generic.avmute){ //add by keith
		hdcp_ForceUnauthentication();
		hdcpOper.state = HDCP_XMT_LINK_H0;		
		DPRINTK("hdcp_XmtState: avmute: \n");
		return;
	}

	//If Tx's KSV is NULL, disable HDCP AUTH.
	if(hdmiDB.pk.ksv[0] == 0x00 && hdmiDB.pk.ksv[1] == 0x00 && hdmiDB.pk.ksv[2] == 0x00 && hdmiDB.pk.ksv[3] == 0x00 && hdmiDB.pk.ksv[4] == 0x00){
		hdcp_ForceUnauthentication();
		hdcpOper.state = HDCP_XMT_LINK_H0;
#ifdef HDCP_MSG
		hdcp_sendHDCPAuthResultMessageToAP(HDMI_TX_NULL_KEY);
		printk("TX Key is NULL: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif	        
		DPRINTK("TX's HDCP KEY ERROR\n");
		return;
	}


	//If Force UN-Auth, disable HDCP AUTH.
	if(hdcpOper.isForceUnauthentication == TRUE){
		hdcp_ForceUnauthentication();
		hdcpOper.state = HDCP_XMT_LINK_H0;
#ifdef HDCP_MSG
		hdcp_sendHDCPAuthResultMessageToAP(HDMI_TX_NULL_KEY);
		printk("Force UN-Auth by User/CHI : hdcp_sendHDCPAuthResultMessageToAP\n");
#endif	        
		DPRINTK("Force UN-Auth by User/CH\n");
		return;
	}	
	//HDCP_REG_AnILR = 0xffffffff;
	//HDCP_REG_AnIMR = 0xffffffff;
	//HDMI_REG_CR |=  HDMI_REG_CR_ENABLE_HDCP;
	/* force Encryption disable */
	//HDCP_REG_CR &= ~HDCP_CR_ENC_ENABLE;
	//while(HDCP_REG_SR & (1<<2));
	/* force HDCP module to unauthenticated state */
	//HDCP_REG_CR |= HDCP_CR_FORCE_UNAUTH;
	//while(HDCP_REG_SR & (1<<1));
	
	switch(hdcpOper.state) {
	case HDCP_XMT_LINK_H0:
		/* wait for Rx attached */
		/* check hot plug detect every 2 seconds */
		/* if HPD active, change to H1 */
		hdcpOper.state = HDCP_XMT_LINK_H1;
		DPRINTK("in hdmi H0 state \n");
	case HDCP_XMT_LINK_H1:
		DPRINTK("in hdmi H1 state \n");
		DPRINTK(" hdmiDB.generic.hdcp =%d\n",hdmiDB.generic.hdcp);
		if(hdmiDB.generic.hdcp) {
			hdcpOper.state = HDCP_XMT_AUTH_A0;
		}
		else {
			//mod_timer(&hdcp_state_timer, jiffies+HZ);//mark by keith
			return;
		}		

#if 0
		/* Read EDID */
		/* if Sink HDMI is capable, transit to H3(HDMI), otherwise, transit to H2(DVI) */
		if(cea861bReadEDID()<0) {
			/* read failed, try to read next time */
			mod_timer(&hdcp_state_timer, jiffies+HZ);
			break;
		}
		/* get EDID content, check HDMI support */
		if(cea861bCheckHDMI()) {
			/* set to send HDMI video signel (H3) */
			/* load HDMI default */
		}
		else {
			/* set to send DVI video signal (H2) */
			/* load DVI default */
		}
		/* set Video only */
		hdmi_set_video();
#endif

	case HDCP_XMT_AUTH_A0:
		/* wait for active Rx */
		/* genrate An, get Aksv */
		HDCP_REG_CR &= ~(1<<9);
		
#ifdef HDCP_MSG		
		hdcpOper.retry = 5;//add by keith
#else 
		hdcpOper.retry = 0;//mark by keith
#endif

		hdcp_FreeRunGetAn(hdcpOper.An);

		DPRINTK("aksv = 0x%02x%02x%02x%02x%02x,   An = 0x%02x%02x%02x%02x%02x%02x%02x%02x\n", \
			hdmiDB.pk.ksv[4], hdmiDB.pk.ksv[3], hdmiDB.pk.ksv[2], hdmiDB.pk.ksv[1], hdmiDB.pk.ksv[0], \
			hdcpOper.An[7], hdcpOper.An[6], hdcpOper.An[5], hdcpOper.An[4], 
			hdcpOper.An[3], hdcpOper.An[2], hdcpOper.An[1], hdcpOper.An[0]);

		hdcpOper.state = HDCP_XMT_AUTH_A1;
		DPRINTK("in hdmi A0 state \n");

	case HDCP_XMT_AUTH_A1:
		/* Exchange KSVs */
		/* process 1st part authentication */
		/* write An and Aksv, read Bksv */
		/* if get Bksv successful mean receiver/repeater support HDCP */
		/* check valid Bksv 20 ones and 20 zero */
		/* The successful read of I2C HDCP register initiates the authenticatin  */
		//DPRINTK("in hdmi A1 state \n");
		if(hdcp_ReadBksv(hdcpOper.Bksv)) {
			#if 0//mark by keith
			/* wait 1 sec to retry */
			mod_timer(&hdcp_state_timer, jiffies+HZ);
			break;
			#else //add by keith
			DPRINTK("retry = %d\n", hdcpOper.retry);
 			if(hdcpOper.retry) {
 				hdcpOper.retry--;
				/* wait 1 sec to retry */
				mod_timer(&hdcp_state_timer, jiffies+HZ);
				break;
 			}
 			else {	
				/* hdcp_ReadBksv failed */
#ifdef HDCP_MSG		
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				printk("hdcp_ReadBksv failed: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
				hdcp_ForceUnauthentication();
				/* restart */
				hdcpOper.state = HDCP_XMT_AUTH_A0;
				mod_timer(&hdcp_state_timer, jiffies+HZ);	
				break;
 			}
			#endif
		}

		/* Trigger Rx to Authentication Part 1 */
		hdcp_TriggerRxAuth(hdmiDB.pk.ksv, hdcpOper.An);
		/* get REPEATER */
		if(hdcp_CheckRepeater()) {
			/* set to support repeater */
			// TBD
			hdcpOper.repeater = 1;
			HDCP_REG_CR = HDCP_REG_CR | HDCP_CR_DS_REPEATER;//add by keith	
		}
		else {
			/* set to support NO repeater */
			// TBD
			hdcpOper.repeater = 0;
			HDCP_REG_CR = HDCP_REG_CR & (~HDCP_CR_DS_REPEATER);//add by keith
		}
		
		DPRINTK("repeater = %x \n", hdcpOper.repeater);
		
		hdcpOper.state = HDCP_XMT_AUTH_A2;

	case HDCP_XMT_AUTH_A2:
		/* Computations */
		/* computes Km, Ks, M0 and R0 */
		DPRINTK("in hdmi A2 state \n");
		//DPRINTK("hdcp_AuthenticationSequence()\n");
		hdcp_AuthenticationSequence();
		
		//hdcpOper.retry = 10;//mark by keith
		hdcpOper.retry = 3;//add by keith
		
		/* if computed results are available */
		hdcpOper.state = HDCP_XMT_AUTH_A3;
		//wait for 100 msec to read R0p.
		mod_timer(&hdcp_state_timer, jiffies+HZ/10);//add by keith
		break;//add by keith
		
	case HDCP_XMT_AUTH_A3:
		/* Validate Receiver */
		/* computes Km, Ks, M0 and R0 */
		/* if computed results are available */
		DPRINTK("In hdmi A3 state \n");
		DPRINTK("Check Authentication\n");
		hdcp_ReadRip(hdcpOper.Rip);	

		printk("R0 = %02x%02x,  R0p = %02x%02x\n", 
			hdcpOper.Ri[1], hdcpOper.Ri[0],
			hdcpOper.Rip[1], hdcpOper.Rip[0]);
		
		if((hdcpOper.Ri[0] != hdcpOper.Rip[0]) || (hdcpOper.Ri[1] != hdcpOper.Rip[1]))
 		{
			DPRINTK("retry = %d\n", hdcpOper.retry);
 			if(hdcpOper.retry) {
 				hdcpOper.retry--;
				mod_timer(&hdcp_state_timer, jiffies+HZ/10);
 			}
 			else {				
				/* authentication part I failed */
#ifdef HDCP_MSG		
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				printk("R0!=R0p: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
				hdcp_ForceUnauthentication();
				//HDMI_REG_GCPCR = HDMI_GCP_ENABLE | HDMI_GCP_AVMUTE_ENABLE;
				#if 1 //add by keith, for solving the problem about R0!= R0p sometimes.
					#if 0
					/* disable HDCP */
					HDMI_REG_CR &=  ~HDMI_REG_CR_ENABLE_HDCP;
					//for(delay=0;delay<3000;delay++);

					/*disable Audio*/
					HDMI_REG_ICR &= ~HDMI_AUDIO_ENABLE;

					/*disable HDMI */
					HDMI_REG_CR &= ~0x00000001;	//disable HDMI
					
					/*set HDMI*/	
					HDMI_REG_CR |= HDMI_REG_CR_RESET;		 //reset
					while(HDMI_REG_CR & HDMI_REG_CR_RESET) ;// wait
					HDMI_REG_CR |= HDMI_REG_CR_ENABLE;	 //enable HDMI
					HDMI_REG_CR |= (1<<30);
					HDMI_REG_CR |= (1<<31);
					HDMI_REG_CR |= (0xf<<24);	
					
					/*config Video*/
					hdmi_setVideo(hdmiDB.generic.mode, &hdmiDB.video);
					hdmi_gen_avi_infoframe(&hdmiDB.video);

					/* Enable Audio FIFO input */
					HDMI_REG_ICR |= HDMI_AUDIO_ENABLE;

					/*config audio*/
					hdmi_setAudio(&hdmiDB.audio);
					hdmi_gen_audio_infoframe(&hdmiDB.audio);
					#endif

					//loadHDCPPrivateKeys_form_Flash();

	#if USE_HDCP_P_KEY_IN_FLASH
		loadHDCPPrivateKeys_form_Flash();
	#else
		INT32	i,j;
		UINT8 ksv[5] = {0x1e,0xc8,0x42,0xdd,0xd9};
		//struct _HDCP_PrivateKey pk;
		UINT8 pkey[40][7] = {
			{0x1b,0xfa,0x8a,0x61,0xf6,0x68,0x3d},
			{0x86,0x5c,0x88,0x5a,0x4d,0xb5,0x32},
			{0x18,0x13,0x2a,0xd4,0xaa,0x1a,0x99},
			{0xe8,0x44,0x46,0xc9,0x3b,0xc1,0x20},
			{0xbb,0xd4,0x64,0xa8,0x6e,0x85,0xac},
			{0xa9,0xa8,0x97,0x17,0xe5,0xed,0x90},
			{0x82,0x3c,0x1b,0xfa,0x11,0x0c,0xb9},
			{0x49,0x87,0x05,0x5f,0x36,0xca,0xd7},
			{0x02,0x08,0xf1,0x16,0xa5,0x6d,0xd7},
			{0xb8,0x26,0x0f,0x51,0x20,0xf2,0xbd},
			{0x43,0xfc,0x7f,0xc6,0xcc,0x5d,0x6f},
			{0x0e,0x22,0xdf,0x32,0xeb,0x5b,0x72},
			{0x15,0x82,0xf0,0x80,0x71,0x9d,0x58},
			{0xc7,0x57,0xca,0xbf,0x44,0xe9,0x59},
			{0x4a,0x97,0x23,0x2b,0x36,0x35,0xe0},
			{0x15,0x00,0x9a,0x38,0x8a,0xe8,0xae},
			{0x7f,0xa8,0x1b,0x95,0x4f,0x90,0x4f},
			{0x24,0x7c,0x36,0xd8,0xb2,0x01,0xf5},
			{0x95,0x00,0xec,0xfd,0x05,0x14,0x06},
			{0xa6,0x4b,0x6a,0xb3,0xa9,0x13,0x63},
			{0x3b,0xba,0x7e,0xa3,0x8c,0x2d,0xa2},
			{0xda,0x11,0xb2,0x18,0x6a,0x69,0xf8},
			{0x5b,0x5f,0x6f,0x0e,0xb9,0xad,0x2f},
			{0x07,0xf7,0x33,0x60,0xc8,0xe6,0x46},
			{0x69,0x2c,0x82,0x3c,0xb3,0x5a,0xd5},
			{0xcd,0xe3,0x81,0xb0,0x3f,0x0b,0x54},
			{0xb8,0x86,0x76,0x98,0x31,0x1e,0xbf},
			{0xd6,0xfb,0xf9,0xe3,0xb7,0x50,0x55},
			{0xb9,0xb8,0xa1,0xfc,0xef,0x71,0x88},
			{0xa6,0x65,0x53,0x99,0x9b,0x0f,0x2a},
			{0x7a,0x06,0x18,0xe0,0xa2,0x2c,0x0e},
			{0xcf,0xd9,0x52,0xf6,0x66,0xad,0x9f},
			{0x4f,0x78,0x96,0x7c,0xc0,0x37,0xc6},
			{0xa6,0xe2,0x59,0x64,0x7c,0x8e,0xea},
			{0x02,0x83,0x43,0x42,0xb4,0xc3,0x4b},
			{0xff,0xe4,0xf2,0x55,0x24,0xa9,0x18},
			{0x7f,0x46,0xf8,0x6b,0xd9,0x36,0x3c},
			{0xd1,0x12,0x0d,0x67,0xeb,0x01,0xe8},
			{0x53,0xc3,0xea,0xdc,0xbe,0x24,0xe8},
			{0xd5,0x07,0xd0,0xb7,0x39,0x85,0xdf},
		};

		/* set HDCP keys */
		hdmiDB.pk.pnt = 0x55;
		for(i=0; i<5; i++)
			hdmiDB.pk.ksv[i] = ksv[i];
		for(i=0; i<40; i++)
			for(j=0; j<7; j++)
				hdmiDB.pk.key[i][j] = pkey[i][j];
		//IOCTLDRV(DS_DEVICE_HDMI, HDCP_PRIVATE_KEYS_SET, (INT32)(&pk));
	#endif
					/*set An Influence Seed*/
					hdcp_loadAnInfluenceSeed(&hdmiDB.seed);

					/* enable HDCP *///add by keith
					HDMI_REG_CR |=  HDMI_REG_CR_ENABLE_HDCP;
				#endif
				/* restart */
				hdcpOper.state = HDCP_XMT_AUTH_A0;
				mod_timer(&hdcp_state_timer, jiffies+HZ);
			}
			break;//return;//break; keith
		}
		hdcpOper.retry = 0;

		#if 1
		/*add by keith*/
		hdcpOper.M0[0]=HDCP_REG_MILR&0xff;
		hdcpOper.M0[1]=(HDCP_REG_MILR>>8) & 0xff;
		hdcpOper.M0[2]=(HDCP_REG_MILR>>16) & 0xff;
		hdcpOper.M0[3]=(HDCP_REG_MILR>>24) & 0xff;
		hdcpOper.M0[4]=HDCP_REG_MIMR&0xff;
		hdcpOper.M0[5]=(HDCP_REG_MIMR>>8) & 0xff;
		hdcpOper.M0[6]=(HDCP_REG_MIMR>>16) & 0xff;
		hdcpOper.M0[7]=(HDCP_REG_MIMR>>24) & 0xff;
		DPRINTK("\n\n\n hdcpOper.M0[0] = 0x%2x \n", hdcpOper.M0[0]);
		#endif		
		
		/* authentication part I successful */
		HDMI_REG_SCHCR &= ~(0x4);
		hdcpOper.state = HDCP_XMT_AUTH_A6;
		
	case HDCP_XMT_AUTH_A6:
		/* Test for Repeater */
		DPRINTK("in hdmi A6 state \n");
		if(hdcpOper.repeater) {
			/* change to Authentication part II */
			hdcpOper.state = HDCP_XMT_AUTH_A8;
			/* wait 100msec */
			mod_timer(&hdcp_state_timer, jiffies+HZ/10);
			break;//return;//break; keith
		}
			

		/* NO repeater */
		/* change to Authentication part III */
		hdcpOper.state = HDCP_XMT_AUTH_A4;
		
	case HDCP_XMT_AUTH_A4:
		/* Authenticated */
		DPRINTK("in hdmi A4 state \n");
		#if 0
		/*add by ciwu*/
		hdcpOper.M0[0]=HDCP_REG_AnILR&0xff;
		hdcpOper.M0[1]=(HDCP_REG_AnILR&(0xff<<8))>>8;
		hdcpOper.M0[2]=(HDCP_REG_AnILR&(0xff<<16))>>16;
		hdcpOper.M0[3]=(HDCP_REG_AnILR&(0xff<<24))>>24;
		hdcpOper.M0[4]=HDCP_REG_AnIMR&0xff;
		hdcpOper.M0[5]=(HDCP_REG_AnIMR&(0xff<<8))>>8;
		hdcpOper.M0[6]=(HDCP_REG_AnIMR&(0xff<<16))>>16;
		hdcpOper.M0[7]=(HDCP_REG_AnIMR&(0xff<<24))>>24;
		#endif		
		hdcp_SetAuthenticated();
		hdcpOper.state = HDCP_XMT_AUTH_A5;
		hdcpOper.retry = 3;		
		/* enable Ri update check */
		HDCP_REG_SR |= HDCP_SR_RI_UPDATE;
		HDCP_REG_CR |= HDCP_CR_RI_INT;

		#if ENABLE_HDCP_EESS//add by krith
		if( hdcp_CheckRxSupportingEESS() ){
			HDCP_REG_SR |= HDCP_SR_PJ_UPDATE;
			HDCP_REG_CR |= HDCP_CR_PJ_INT;
		}
		#endif
		
		//mod_timer(&hdcp_state_timer, jiffies+HZ);
		//DPRINTK("in hdmi A4 state ... done\n");
		break;
		
	case HDCP_XMT_AUTH_A5:
		DPRINTK("in hdmi A5 state \n");
		/* Link Integrity Check */
		/* Interrupt and BH will do this job */
		//mod_timer(&hdcp_state_timer, jiffies+HZ);		
		
		break;
		
		
	case HDCP_XMT_AUTH_A8:
		DPRINTK("in hdmi A8 state \n");
	#if 1
		/* 2nd part authentication */
		/* Wait for Ready */
		/* set up 5 second timer poll for KSV list ready */
		if(!hdcp_CheckKsvListReady()) {
			DPRINTK("hdcp_CheckKsvListReady = 0 \n");
			#if 1 //add by keityh
			if(retry_times_for_set_up_5_second<50){ //100 msec * 50 = 5 sec
				retry_times_for_set_up_5_second++;
				hdcpOper.state = HDCP_XMT_AUTH_A8;
				mod_timer(&hdcp_state_timer, jiffies+HZ/10);/* wait 100msec */
				break;
			} else {
				/* restart */
				retry_times_for_set_up_5_second = 0;
				hdcpOper.state = HDCP_XMT_AUTH_A0;
#ifdef HDCP_MSG		
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				printk("hdcp_CheckKsvListReady: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
				hdcp_ForceUnauthentication();
				mod_timer(&hdcp_state_timer, jiffies+HZ/10);
				break;
			}
			#else//mark by keith 				
			/* wait 100msec */
			mod_timer(&hdcp_state_timer, jiffies+HZ/10);			
			//mod_timer(&hdcp_state_timer, jiffies+HZ);	// add by ciwu
			//hdcpOper.state = HDCP_XMT_AUTH_A0;		// add by ciwu
			break;			
			#endif
		}
		retry_times_for_set_up_5_second = 0;
		hdcpOper.state = HDCP_XMT_AUTH_A9;
		//mod_timer(&hdcp_state_timer, jiffies+HZ/10);	// add by keith
	#endif		
		//break;	// add by keith
	
	case HDCP_XMT_AUTH_A9:
		DPRINTK("in hdmi A9 state \n");
	#if 1
		//int cnt2;
		/* Read KSV List and Bstatus */
		//hdcp_ReadKsvList(&hdcpOper.Bstatus, hdcpOper.ksvList);//mark by keith
		hdcp_ReadKsvList(hdcpOper.Bstatus, hdcpOper.ksvList);//add by keith

		//cnt2=GET_BITS(hdcpOper.Bstatus[0], 6, 0)+1;	//add by ciwu
		//hdcpOper.llen=cnt2*10;						//add by ciwu

		/* get V' */
		//hdcp_ReadVprime();				//mark by ciwu
		hdcp_ReadVprime(hdcpOper.Vp);		//add by ciwu
		
		/* generate (KSV list||Bstatus||M0) */
		/* generate V = SHA-1(KSV list||Bstatus||M0) */
		generate_V(hdcpOper.V, hdcpOper.ksvList, hdcpOper.Bstatus, hdcpOper.M0);//ad by keith
		//do_sha_1(hdcpOper.V, hdcpOper.llen, hdcpOper.M0); // mark by keith

		/*INT32 pigi =0;
		for(pigi=0;pigi<20;pigi++) {			
			printk("V=%x\n", hdcpOper.V[pigi]);
		}*/
		
		/* compare with V' */
		if(memcmp(hdcpOper.V, hdcpOper.Vp, 20)) {
			/* authentication part II failed */
#ifdef HDCP_MSG		
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				printk("V!=Vp: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
			hdcp_ForceUnauthentication();
			hdcpOper.state = HDCP_XMT_AUTH_A0;
			mod_timer(&hdcp_state_timer, jiffies+HZ/10);
			break;				
		}

		/* KSV list correct , transit to Authentication Part III */
		hdcpOper.state = HDCP_XMT_AUTH_A4;
		mod_timer(&hdcp_state_timer, jiffies+HZ/10);
		break;				
	#endif	
	default:
		break;
	}
	
	return;
}


DECLARE_WAIT_QUEUE_HEAD(waitQueue_hdcpXmtState);
static int hdcpXmtState_thread_pid = -1; 
static int hdcpXmtState_thread_is_wakeup = 0;

void runHDCPXmtState(unsigned long tmp){
	DPRINTK("runHDCPXmtState .....................\n");
		mb();
		hdcpXmtState_thread_is_wakeup = 1;
		wake_up_interruptible(&waitQueue_hdcpXmtState);
		return;
}

static int thread_hdcpXmtState(void *data){
	DPRINTK("thread_hdcpXmtState .....................\n");
	
	/* main loop */
	while (1) {
		printk("thread_hdcpXmtState :  in while ..................\n");
		interruptible_sleep_on(&waitQueue_hdcpXmtState);
		if(hdcpXmtState_thread_is_wakeup == 0){
			DPRINTK("thread_hdcpXmtState stop!!\n");
			break;
		} else {
			hdcpXmtState_thread_is_wakeup = 0;
		}
		hdcp_XmtState(0);
	}

	DPRINTK("thread_hdcpXmtState ...................done\n");
	return 0;
}

INT32 hdcpXmtState_thread_init(void){
	int nothing =0;
	hdcpOper.HDCPAuthResult = HDCP_AUTH_UNKNOW_RESULT;//add by keith
	hdcpOper.HDCPAuthFailCounter =0;//add by keith
	/* create a kthread for invoking hdcpXmtState*/
	hdcpXmtState_thread_is_wakeup = 0;
	hdcpXmtState_thread_pid = kthread_run(thread_hdcpXmtState, (void*)(&nothing), "hdcpXmtState");
	printk("%s %d hdcpXmtState_thread_pid=0x%x\n",__FILE__,__LINE__,hdcpXmtState_thread_pid);
	if (!IS_ERR(hdcpXmtState_thread_pid)) {
		DPRINTK("ERROR: kernel_thread(thread_hdcpXmtState, nothing, 0) \n");
	}
	return 0;	
}

INT32 hdcpXmtState_thread_kill(void){
	/* shutdown the thread if there was one */
	if (hdcpXmtState_thread_pid) {
		
		//stop it, but it need time to its cpu turn.
		hdcpXmtState_thread_is_wakeup = 0;
		wake_up_interruptible(&waitQueue_hdcpXmtState);
		
		//kill it
		kthread_stop(hdcpXmtState_thread_pid);
		/*if (kill_proc(hdcpXmtState_thread_pid, 0, 1) == -ESRCH) 
		{
			DPRINTK("ERROR: hdcpXmtState_thread_kill thread PID %d already died\n", hdcpXmtState_thread_pid);
		} else {
			DPRINTK("SUCCESS: hdcpXmtState_thread_kill thread PID %d already died\n", hdcpXmtState_thread_pid);;
		}*/
	}
	return 0;	
}

INT32 hdcp_timer_init(void)
{
	init_timer(&hdcp_state_timer);
	//hdcp_state_timer.function = hdcp_XmtState;
	hdcp_state_timer.function = runHDCPXmtState;
	return 0;
}

INT32 hdcp_timer_stop(void)
{
	del_timer(&hdcp_state_timer);
	return 0;
}

BOOL hdcp_forceUnauthentication(BOOL isforceUnauthentication){
	DPRINTK("hdcp_forceUnauthentication %d \n", isforceUnauthentication);
	if(hdcpOper.isForceUnauthentication == isforceUnauthentication)
		return TRUE;

	hdcpOper.isForceUnauthentication = isforceUnauthentication;

	if(hdcpOper.isForceUnauthentication == FALSE){
		hdcpOper.state = HDCP_XMT_AUTH_A0;
		runHDCPXmtState(0);
		DPRINTK("hdcpOper.isForceUnauthentication == FALSE\n");
	}else{
		hdcp_ForceUnauthentication();
		hdcpOper.state = HDCP_XMT_LINK_H0;//add by keith
		DPRINTK("hdcpOper.isForceUnauthentication == TRUE\n");
	}

	return TRUE;
}
#endif


