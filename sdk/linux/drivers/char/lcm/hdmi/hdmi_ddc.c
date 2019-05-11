/***************************************************************************
	hdmi_reg.h
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 *
 * All rights reserved.                                       *
 *============================================================*/

/***************************************************************************
                          hdmi_ddc.c  -  description
                             -------------------
    begin                : Monday January 18, 2005
    copyright            : (C) 2004 by Kao, Kuo-Chun
    email                : kckao@realtek.com.tw
 ***************************************************************************/

/**
 * @file hdmi_ddc.c
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
  $Header: /cvsroot/dtv/linux-2.6/drivers/rtd2880/hdmi/hdmi_ddc.c,v 1.4 2005/12/23 07:53:13 kckao Exp $

  $Log: hdmi_ddc.c,v $
  Revision 1.4  2005/12/23 07:53:13  kckao
  *:modified for HDCP auto link state machine

  Revision 1.3  2005/11/25 03:33:36  kckao
  HDMI Tx Driver tested with RTD2880 Demo Board

  Revision 1.2  2005/05/03 11:37:12  kckao
  no message

  Revision 1.1  2005/05/03 03:05:16  kckao
  no message




 *=============================================================*/


#include <linux/module.h>
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <asm/io.h>


/* RTD2880 header file */
#include "rtd_types.h"
#include "hdmi_reg.h"

#if CONFIG_AM_CHIP_ID == 1213
/* external I2C function */
#define ErrCode	INT32

#define GET_BITS(r,s,e)	((r>>e)&~(0xffffffff<<(s-e+1)))
#define GET_BIT(r,b)	((r>>b)&1)
#define swaps(x)	((((x)&0xff00)>>8)+(((x)&0x00ff)<<8))
#define swapl(x)	((((x)&0x000000ff)<<24)+(((x)&0x0000ff00)<<8)+(((x)&0x00ff0000)>>8)+(((x)&0xff000000)>>24))

INT32 hdcp_WriteAn(const UINT8 *An);
INT32 hdcp_WriteAksv(const UINT8 *Aksv);
INT32 hdcp_WriteAinfo(const UINT8 *Ainfo);
INT32 hdcp_ReadBcaps(UINT8 *Bcaps);
INT32 hdcp_ReadBstatus(UINT8 *Bstatus);
INT32 hdcp_ReadBksv(UINT8 *Bksv);
INT32 hdcp_ReadREPEATER(UINT8 *pepeater);
INT32 hdcp_ReadRip(UINT8 *rip);
INT32 hdcp_ReadPjp(UINT8 *pjp);
INT32 hdcp_PollKSVListReady(void);
INT32 hdcp_ReadKSVList(UINT8 *ksvlist);


#if defined(CONFIG_RTD2880_IDTV_v2)
#define HDCP_DDC_DEVICE			0
//#elif defined(CONFIG_DTV_MODULE_BOARD) || defined(CONFIG_RTD2870_IDTV_v1) || defined(CONFIG_RTD2881_DEMO_BOARD_v1)
//#define HDCP_DDC_DEVICE			2
#else
#define HDCP_DDC_DEVICE			2
#endif

#define HDCP_DDC_PRIMARY_WRITE	0x74
#define HDCP_DDC_PRIMARY_READ		0x75
#define HDCP_DDC_SECOND_WRITE	0x76
#define HDCP_DDC_SECOND_READ		0x77
#define EDID_SEG				0x60
#define EDID_WRITE_ADDR			0xa0
#define EDID_READ_ADDR			0xa1

extern int hdcp_ddc_read(UINT8 offset, UINT8 r_len, UINT8* read_buf);
extern int hdcp_ddc_write(UINT8 offset, UINT8 w_len, UINT8* write_buf);

#if 0
INT32 hdcp_ddc_init(void)
{
	//I2C_init(HDCP_DDC_DEVICE, 0);
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01041012);	
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01041315);
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01041518);			//slow
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x00441012);			//more slow  <-- ok
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01042125);			//more more slow
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01042226);			//more more more slow
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01042328);			//more more more slow
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x01046064);
	I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x00441012);
	rtd_outl(0xb8007120, rtd_inl(0xb8007120) & (~0x0000FF00));
	//I2C_setSCLTiming(HDCP_DDC_DEVICE, 0x00930910);			//ok for 189MHZ in Tarzan
	return 0;
}


INT32 hdmi_get_edid(unsigned char *dbuf)
{
	unsigned char cbuf[5];
	int segment = 0;
	int r_len = 128;
	int i, j, k;
	int ret_val;
	int extension_number, start;
	
	/* init I2C */
	hdcp_ddc_init();

	/* read 1 byte from I2C slave, write 3 bytes for address, offset */
#if 0
	for(i=0; i<256; i+=16) {
		cbuf[0] = EDID_SEG;
		cbuf[1] = segment;
		cbuf[2] = EDID_WRITE_ADDR;
		cbuf[3] = i;
		cbuf[4] = EDID_READ_ADDR;
		ret_val = I2C_randomRead(HDCP_DDC_DEVICE, 5, 16, cbuf, &dbuf[i]);
		//if(ret_val)
		//	return ret_val;
		
		if(ret_val != R_ERR_SUCCESS){//add by keith
			printk("I2C_randomRead ERROR!!\n");
			return ret_val;
		}
	}

#else //20080516 kist modify 	

	for(i=0; i<r_len; i+=16) {
		cbuf[0] = EDID_WRITE_ADDR;
		cbuf[1] = i;
		cbuf[2] = EDID_READ_ADDR;
		ret_val = I2C_randomRead(HDCP_DDC_DEVICE, 3, 16, cbuf, &dbuf[i]);

		if(ret_val != R_ERR_SUCCESS){//add by keith
			printk("I2C_randomRead ERROR!!\n");
			return ret_val;
		}
		for (k=0; k<16; k++) {
			printk("%02x ", dbuf[i+k]);
		}
		printk("\n");
	}
	
	extension_number = dbuf[126];
	printk("extension_number =%d\n",extension_number);
	
	if (extension_number > 3) extension_number = 3;

	for (j=0;j<extension_number;j++) {
		
		if ( j & 0x01) {
			start = 0;
		} else {
			start = r_len;
		}

		for(i=0; i<r_len; i+=16) {	
			if (j==0){
				//need not to read segment
				cbuf[0] = EDID_WRITE_ADDR;
				cbuf[1] = start + i;
				cbuf[2] = EDID_READ_ADDR;
				ret_val = I2C_randomRead(HDCP_DDC_DEVICE, 3, 16, cbuf, &dbuf[i+r_len]);
			}
			else if (j==1){
				cbuf[0] = EDID_SEG;
				cbuf[1] = (j + 1) >> 1;//Block 2,3-->segment=1 
				cbuf[2] = EDID_WRITE_ADDR;
				cbuf[3] = start + i;
				cbuf[4] = EDID_READ_ADDR;
				ret_val = I2C_randomRead(HDCP_DDC_DEVICE, 5, 16, cbuf, &dbuf[i+r_len]);			
			}
			
			if(ret_val != R_ERR_SUCCESS){//add by keith
				printk("I2C_randomRead ERROR_01!!\n");
				return ret_val;
			}					
			for (k=0; k<16; k++) {
				printk("%02x ", dbuf[i+k+r_len]);
			}
			printk("\n");				
		}	

		if (dbuf[128] == 0x02  && dbuf[129] == 0x03)  {
			printk("Find CEA Extension Version 3\n"); 	// Find CEA Extension Version 3
			return 0;				
		}		
	}
	
	memset(&dbuf[128], 0xFF, 128);

#endif
	return 0;
}


INT32 hdcp_ddc_write(UINT8 offset, UINT8 w_len, UINT8* write_buf)
{
UINT8 buf[3];
INT32 i;
INT32 retVal;
	for(i = 0; i<w_len; i++) {
		buf[0] = HDCP_DDC_PRIMARY_WRITE;
		buf[1] = offset + i;
		buf[2] = write_buf[i];
		retVal = I2C_write(HDCP_DDC_DEVICE, 3, buf);
		if(retVal<0)
			break;
	}
	return i;
}

#if 0
INT32 hdcp_ddc_cur_read(UINT8 offset, UINT8 r_len, UINT8* read_buf) //add by ciwu
{
UINT8 buf[3];
INT32 i;
volatile j;
INT32 retVal;
	
		buf[0] = HDCP_DDC_PRIMARY_WRITE;
		buf[1] = offset;
		buf[2] = HDCP_DDC_PRIMARY_READ;
		/* read 1 byte from I2C slave, write 3 bytes for address, offset */
		retVal = I2C_currRead(HDCP_DDC_DEVICE, r_len, buf, read_buf);
		//for(j=0;j<10000;j++);

	return i;
}
#endif

INT32 hdcp_ddc_read(UINT8 offset, UINT8 r_len, UINT8* read_buf)
{
UINT8 buf[3];
INT32 i;
//volatile INT32 j;
INT32 retVal;
	for(i = 0; i<r_len; i++) {
		buf[0] = HDCP_DDC_PRIMARY_WRITE;
		buf[1] = offset + i;
		buf[2] = HDCP_DDC_PRIMARY_READ;
		/* read 1 byte from I2C slave, write 3 bytes for address, offset */
		retVal = I2C_randomRead(HDCP_DDC_DEVICE, 3, 1, buf, read_buf+i);
		//if(retVal != R_ERR_SUCCESS) printk("I2C_randomRead is ERROR!\n");
		//for(j=0;j<10000;j++);
		if(retVal<0)
			break;
	}
	return i;
}
#endif

INT32 hdcp_ddc_Bksv_read(UINT8 offset, UINT8 r_len, UINT8* read_buf)//add by keith
{
#if 0
	UINT8 buf[3];
	INT32 retVal;

	buf[0] = HDCP_DDC_PRIMARY_WRITE;
	buf[1] = offset;
	buf[2] = HDCP_DDC_PRIMARY_READ;
	/* read 1 byte from I2C slave, write 3 bytes for address, offset */
	retVal = I2C_randomRead(HDCP_DDC_DEVICE, 3, r_len, buf, read_buf);
	
	return r_len;
#endif	
	return hdcp_ddc_read(offset,r_len,read_buf);
}

INT32 hdcp_ReadRip(UINT8 *rip)
{
	UINT8 offset = 0x08;
#if 0	
	UINT8 buf[3];

	buf[0] = HDCP_DDC_PRIMARY_WRITE;
	buf[1] = offset;
	buf[2] = HDCP_DDC_PRIMARY_READ;
	
	/* read 2 byte from I2C slave, write 3 bytes for address, offset */
	return I2C_randomRead(HDCP_DDC_DEVICE, 3, 2, buf, rip);
#endif	

	return hdcp_ddc_read(offset,2,rip);
}


INT32 hdcp_ReadPjp(UINT8 *pjp)
{
	UINT8 offset = 0x0A;
#if 0
	UINT8 buf[3];

	buf[0] = HDCP_DDC_PRIMARY_WRITE;
	buf[1] = offset;
	buf[2] = HDCP_DDC_PRIMARY_READ;
	/* read 2 byte from I2C slave, write 3 bytes for address, offset */
	return I2C_randomRead(HDCP_DDC_DEVICE, 3, 1, buf, pjp);
#endif	
	return hdcp_ddc_read(offset,1,pjp);
}

INT32 hdcp_ReadBcaps(UINT8 *Bcaps)
{
	/* read Bcaps */
	return hdcp_ddc_read(0x40, 1, Bcaps);
}

INT32 hdcp_CheckRepeater(void)
{
UINT8 Bcaps;
	/* read Bcaps */
	hdcp_ddc_read(0x40, 1, &Bcaps);
	printk("bcaps=%x\n",Bcaps);
	if(Bcaps & (1<<6))
		return 1;
	return 0;
}

INT32 hdcp_CheckKsvListReady(void)
{
UINT8 Bcaps;
	/* read Bcaps */
	hdcp_ddc_read(0x40, 1, &Bcaps);
	printk("bcaps=%x\n",Bcaps);
	if(Bcaps & (1<<5))
		return 1;
	return 0;
}

//add by keith
INT32 hdcp_CheckRxSupportingEESS(void)
{
UINT8 Bcaps;
	/* read Bcaps */
	hdcp_ddc_read(0x40, 1, &Bcaps);
	if(Bcaps & (1<<1))
		return 1;
	return 0;
}

INT32 ksvCheckBits(UINT8 *ksv)
{
int n = 0;
int i, j;
	for(i=0; i<5; i++)
		for(j=0; j<8; j++)
			if(ksv[i]&(1<<j))
				n++;
	return n;
}

INT32 hdcp_ReadBksv(UINT8 *Bksv)
{

#if 0
	/* write An */
	hdcp_ddc_write(0x18, 8, An);
	/* write Aksv */
	hdcp_ddc_write(0x10, 5, Aksv);
#endif

	/* read Bksv */
	hdcp_ddc_read(0x00, 5, Bksv);
	printk("Bksv = 0x%2x %2x %2x %2x %2x\n", Bksv[0], Bksv[1], Bksv[2], Bksv[3], Bksv[4]);
	if(ksvCheckBits(Bksv) != 20)
		return -1;
	return 0;
}


INT32 hdcp_TriggerRxAuth(UINT8 *Aksv, UINT8 *An)
{
	/* write An */
	hdcp_ddc_write(0x18, 8, An);
	/* write Aksv */
	hdcp_ddc_write(0x10, 5, Aksv);

	return 0;
}


INT32 hdcp_ReadKsvList(UINT8 *Bstatus, UINT8 *ksvlist)
{
INT32 cnt;
UINT32 i;

	/* get device count in Bstatus [6:0] */
	//hdcp_ddc_read(0x41, 1, Bstatus);//mark by keith
	hdcp_ddc_read( 0x41, 1, (UINT8*)(Bstatus+0) );//add by keith

	hdcp_ddc_read( 0x42, 1, (UINT8*)(Bstatus+1) );//add by keith
	
	printk(" Bstatus=0x%2x %2x, ", *(Bstatus+1), *(Bstatus+0) );
	//cnt = GET_BITS(*Bstatus, 6, 0);//mark by keith
	cnt = GET_BITS(Bstatus[0], 6, 0);//add by keith
	printk(" cnt=%d, ", cnt);

	//for(i=0; i<cnt; i++) { //add by keith		
	//	hdcp_ddc_Bksv_read(0x43, 5, ksvlist+(i*5) );
	//}	
	hdcp_ddc_Bksv_read(0x43, 5*cnt, ksvlist );//add by keith
	
	//hdcp_ddc_cur_read(0x43, 5,ksvlist);//add by ciwu
	for(i=0; i<cnt; i++){		
		printk("hdcp_ReadKsvList[%d]: ksvlist: 0x%02x%02x%02x%02x%02x\n", i,
			*(ksvlist+i*5), *(ksvlist+1+i*5), *(ksvlist+2+i*5), *(ksvlist+3+i*5), *(ksvlist+4+i*5) );
	}
	return 0;
}


INT32 hdcp_ReadVprime(UINT8 *Vp)
{
	int i;
	/* read Vp */
	for(i=0;i<20;i++) {
		hdcp_ddc_read(0x20+i, 1, Vp+i);
		//printk("Vp=%x\n",*(Vp+i));
	}
	return 0;
}
#endif


