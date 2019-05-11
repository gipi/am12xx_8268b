///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <it6681.h>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2013/12/06
//   @fileversion: ITE_IT6682_1.01
//******************************************/
#ifndef _IT6681_H_
#define _IT6681_H_

#define IT6681_HDMI_RX_ADDR 0x90
#define IT6681_HDMI_TX_ADDR 0x98
#define IT6681_MHL_ADDR 0xC8

#ifdef __cplusplus
extern "C" {
#endif

int it6681_fwinit(void);
void it6681_irq(void);
void it6681_poll(void);

int HDMITX_SetAVIInfoFrame(void *p);
//void HDMITX_SET_SignalType(unsigned char DynRange,unsigned char colorcoef,unsigned char pixrep);
void HDMITX_SetVideoOutput(int mode);
void HDMITX_change_audio(unsigned char AudType,unsigned char AudFs,unsigned char AudCh);
void it6681_set_packed_pixel_mode(unsigned char mode);
void it6681_set_hdcp(unsigned char mode);

void DumpHDMITXReg(void);
void DumpHDMIRXReg(void);

int it6681_read_edid( void *it6681_dev_data, void *pedid, unsigned short max_length);
int it6681_i2c_poweroff(void);
struct it6681_platform_data
{
    struct i2c_client *hdmi_tx_client;
    struct i2c_client *hdmi_rx_client;
    struct i2c_client *mhl_client;
};


#ifdef __cplusplus
}
#endif


#endif
