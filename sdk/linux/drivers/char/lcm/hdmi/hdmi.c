//720p
#include "rtd_types.h"
#include "hdmi_drv.h"
#include "hdmi_reg.h"
#include <actions_regs.h>
//#include "misc.h"
#include "typedef.h"
#include "hdmi.h"
#include "linux/string.h"
//#include   	 "dac_drv.h"  			//for DA controller

#if CONFIG_AM_CHIP_ID == 1213
#include <linux/module.h>
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>//add by keith
#include "am7x_board.h"
#endif

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif

#define hdmi_copy_from_user	!memcpy
#define hdmi_copy_to_user	!memcpy
#define hdmi_get_user(x,y)	x=*y;
#define hdmi_put_user(x,y)	*y=x;

#define ESPIPE			-2
#define ENOTTY			-3
#define EFAULT			-4
#if CONFIG_AM_CHIP_MINOR == 8268
#define SSL_S_END_BIT 7
#define SSL_S_START_BIT 1
#else
#define SSL_S_END_BIT 8
#define SSL_S_START_BIT 2
#endif

#if 0
#define DPRINTK(format, args...) \
	printk("HDMI TX: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif
#define ACT_SAMPLE_44K		0x9
void static inline act_writel(unsigned int val, int reg)
{
    *(volatile unsigned int *)(reg) = val;
}
static inline unsigned int act_readl(unsigned long port)
{
    return (*(volatile unsigned int *)port);
}

static void RegBitSet(int val,int reg,int msb,int lsb){
	unsigned int valr;
	
	valr=act_readl(reg);
	if((msb==31)&&(lsb==0))
		act_writel(0|(val<<lsb)|0,reg);
	else if(msb==31)
		act_writel(0|(val<<lsb)|(valr<<(32-lsb)>>(32-lsb)),reg);
	else if(lsb==0)
		act_writel((valr>>(msb+1)<<(msb+1))|(val<<lsb)|0,reg);
	else
		act_writel((valr>>(msb+1)<<(msb+1))|(val<<lsb)|(valr<<(32-lsb)>>(32-lsb)),reg);
}


/* If wish to dump HDMI InfoFrame, set 1. */
#define DUMP_HDMI_INFOFRAME 0

/* If wish to support fail RGB Rx  and auto set color FMT, define it*/
//#define AUTO_SET_COLOR_FMT 1

#define ENABLE	1
#define DISABLE	0
//////////hdmi_drv

/* HDMI Tx local include File */
#include "hdmi_opr.h"
#include "hdmi_reg.h"
//#include "island.h"
#if MODULE_CONFIG_HDMI_HDCP_ENABLE
#include "hdcp_drv.h"
#include "hdmi_ddc.h"

#define HDCP_MSG 0
static struct tasklet_struct hdmi_irq_tasklet;
extern struct hdcp_oper hdcpOper;
#endif



#define HDMI_TX_MINOR		6
#define HDMI_TX_DEVICE_NODE_ID	10

//#ifdef AUTO_SET_COLOR_FORMAT
typedef enum _HDMI_COLOR_FMT{
	COLOR_FMT_RGB = 0x0,
	COLOR_FMT_YCBCR_422 = 0x1,
	COLOR_FMT_YCBCR_444 = 0x2,
	COLOR_FMT_AUTO_DETECT = 0x20
} HDMI_COLOR_FMT;
//#endif


unsigned int outstanding_HDMI_CR = 0;
unsigned int outstanding_HDCP_SR = 0;

INT32 tmds_SerializerSet(INT32 setting);

struct _timing_format {
	INT8 name[32];		// name
	INT32 clock;		// pixel clock
	INT32 interlace;		// interlaced/prograsive
	INT32 h_max;			// total horizontal pixels
	INT32 v_max;			// total vertical lines
	INT32 video_h_max;	// active horizontal pixels
	INT32 video_v_max;	// active vertical lines
	INT32 level;			// timing polarity
	INT32 repeat;			// video data repetetion
};


/* RTD2880 Video Timing and Waveform Definetion */
struct _timing_format t_format[] = {
	{"1920x1080i @60Hz", 7425, 1, 2200, 1125, 1920, 1080, 1, 0},	// 0
	{"1920x1080i @50Hz", 7425, 1, 2640, 1125, 1920, 1080, 1, 0},	// 1
	{"1920x1080p @30Hz", 7425, 0, 2200, 1125, 1920, 1080, 1, 0},	// 2
	{"1920x1080p @25Hz", 7425, 0, 2640, 1125, 1920, 1080, 1, 0},	// 3

	{"1920x1080p @24Hz", 7425, 0, 2750, 1125, 1920, 1080, 1, 0},	// 4
	{"1280x720p @60Hz", 7425, 0, 1650, 750, 1280, 720, 1, 0},		// 5
	{"1280x720p @30Hz", 7425, 0, 3300, 750, 1280, 720, 1, 0},		// 6
	{"1280x720p @50Hz", 7425, 0, 1980, 750, 1280, 720, 1, 0},		// 7

	{"1280x720p @25Hz", 7425, 0, 3960, 750, 1280, 720, 1, 0},		// 8
	{"1280x720p @24Hz", 7425, 0, 4125, 750, 1280, 720, 1, 0},		// 9
	{"1920x1125i @50Hz", 7200, 1, 2304, 1250, 1920, 1125, 1, 0},	// 10
	{"1440x480i @60Hz", 2700, 1, 1716, 525, 1440, 480, 0, 1},		// 11

	{"1440x576i @50Hz", 2700, 1, 1728, 625, 1440, 576, 0, 1},		// 12
	{"720x480p @60Hz", 2700, 0, 858, 525, 720, 480, 0, 0},		// 13
	{"720x576p @50Hz", 2700, 0, 864, 625, 720, 576, 0, 0},		// 14
	{"1920x1080p @60Hz", 14850, 0, 2200, 1125, 1920, 1080, 1, 0},  //15
	{"1920x1080p @50Hz", 14850, 0, 2200, 1125, 1920, 1080, 1, 0},  //16
};

//static INT32 timing_format = 13;

struct vid_table {
	INT32 vid;						// CEA861B Video Identification Codes
	struct _timing_format *timing;	// RTD2880 Display Interface Timing Definetion
	INT32 aspect;						// Aspect Ratio
};

/*
	CEA-861B Table 13, Video Identification Codes
*/
static struct vid_table VideoID[] = {
	{VIDEO_ID_720x480_60P_4_3, &t_format[13], 0},		// 720x480p		@60Hz	4:3
	{VIDEO_ID_720x480_60P_16_9, &t_format[13], 1},		// 720x480p		@60Hz	16:9
	{VIDEO_ID_1280x720_60P_16_9, &t_format[5], 1},		// 1280x720p 	@60Hz	16:9
	{VIDEO_ID_1920x1080_60I_16_9, &t_format[0], 1},		// 1920x1080i 	@60Hz	16:9
	{VIDEO_ID_720x480_60I_4_3, &t_format[11], 0},		// 1440x480i 	@60Hz	4:3
	{VIDEO_ID_720x480_60I_16_9, &t_format[11], 1},		// 1440x480i 	@60Hz	16:9
	{VIDEO_ID_720x576_50P_4_3, &t_format[14], 0},		// 720x576p		@50Hz	4:3
	{VIDEO_ID_720x576_50P_16_9, &t_format[14], 1},		// 720x576p		@50Hz	16:9
	{VIDEO_ID_1280x720_50P_16_9, &t_format[7], 1},		// 1280x720p	@50Hz	16:9
	{VIDEO_ID_1920x1080_50I_16_9, &t_format[1], 1},		// 1920x1080i	@50Hz	16:9
	{VIDEO_ID_720x576_50I_4_3, &t_format[12], 0},		// 1440x576i	@50Hz	4:3
	{VIDEO_ID_720x576_50I_16_9, &t_format[12], 1},		// 1440x576i	@50Hz	16:9
	{VIDEO_ID_1920x1080_24P_16_9, &t_format[4], 1},		// 1920x1080p	@24Hz	16:9
	{VIDEO_ID_1920x1080_25P_16_9, &t_format[3], 1},		// 1920x1080p	@25hz	16:9
	{VIDEO_ID_1920x1080_30P_16_9, &t_format[2], 1},		// 1920x1080p	@30Hz	16:9
	{VIDEO_ID_1920x1080_60P_16_9, &t_format[15], 1},	// 1920x1080p	@60Hz	16:9
	{VIDEO_ID_1920x1080_50P_16_9, &t_format[16], 1},	// 1920x1080p	@50Hz	16:9
	{0, 0, 0},					// End

};


/* Wait queue used when Ri Update Interrupt */
DECLARE_WAIT_QUEUE_HEAD(verify_RiRip_thread_queue);
/*verify_RiRip_thread pid */
static int verify_RiRip_thread_pid = -1; 
static int verify_RiRip_thread_is_wakeup = 0;

/* Wait queue used when HDP Interrupt */
DECLARE_WAIT_QUEUE_HEAD(do_HDP_thread_queue);
/*do_HDP_thread pid */
static int do_HDP_thread_pid = -1; 
static int do_HDP_thread_is_wakeup = 0;


struct _HDMI_Oper	hdmiDB;
struct _DVI_Config	dviCfg;

static void * hdmi_get_vid_table(UINT32 vid)
{
INT32 i;
	for(i=0; VideoID[i].vid; i++)
		if(VideoID[i].vid == vid)
			return ((void*)&VideoID[i]);
	return 0;
}

UINT32 hdmi_GetVersion(void)
{
	return HDMI_REG_VR;
}



INT32 hdmi_Reset(void)
{
	HDMI_REG_CR |= (1<<1);
	while(HDMI_REG_CR&(1<<1));
	return 0;
}

INT32 hdmi_Enable(void)
{
	do {
		HDMI_REG_CR |= 0x01;
	} while ((HDMI_REG_CR & 0x01)==0);
	return 0;
}



/* Packet Command */

int hdmi_EnableWriteRamPacket(void)
{
int i;

	HDMI_REG_OPCR |= (1<<31);
	while (HDMI_REG_OPCR & (1<<31))
	{}	//for(i=0; i<10000; i++)
			/* do nothing */;
	return 0;

}

int hdmi_SetRamPacketPeriod(unsigned int no, int period)
{
	if(no>5 || no<0)
		return -1;
	if(period>0xf || period<0)
		return -1;

	// disable
	HDMI_REG_RPCR &= ~(1<<no);
	HDMI_REG_RPCR &= ~(0xf<<(no*4+8));

	if(period) {	// enable and set period
		HDMI_REG_RPCR |= (period<<(no*4+8));
		HDMI_REG_RPCR |= (1<<no);
	}

	return 0;
}

/*
	convert readable Data Island packet to RAM packet format,
	and write to RAM packet area
*/

int hdmi_SetRamPacket(unsigned int no, unsigned char *pkt)
{
unsigned char tpkt[36];
unsigned long *reg = (unsigned long *)tpkt;
#if 0 //for 2881
unsigned int addr = 70 + no*14;
#else//for 2885 -->20080331 kist
unsigned int addr = 98 + no*14;
#endif

	if(no>5 || no<0)
		return -1;
#if 0

	tpkt[0] = 0;
	tpkt[1] = pkt[2];
	tpkt[2] = pkt[1];
	tpkt[3] = pkt[0];

	tpkt[4] = pkt[6];
	tpkt[5] = pkt[5];
	tpkt[6] = pkt[4];
	tpkt[7] = pkt[3];
	tpkt[8] = 0;
	tpkt[9] = pkt[9];
	tpkt[10] = pkt[8];
	tpkt[11] = pkt[7];

	tpkt[12] = pkt[13];
	tpkt[13] = pkt[12];
	tpkt[14] = pkt[11];
	tpkt[15] = pkt[10];
	tpkt[16] = 0;
	tpkt[17] = pkt[16];
	tpkt[18] = pkt[15];
	tpkt[19] = pkt[14];

	tpkt[20] = pkt[20];
	tpkt[21] = pkt[19];
	tpkt[22] = pkt[18];
	tpkt[23] = pkt[17];
	tpkt[24] = 0;
	tpkt[25] = pkt[23];
	tpkt[26] = pkt[22];
	tpkt[27] = pkt[21];

	tpkt[28] = pkt[27];
	tpkt[29] = pkt[26];
	tpkt[30] = pkt[25];
	tpkt[31] = pkt[24];
	tpkt[32] = 0;
	tpkt[33] = pkt[30];
	tpkt[34] = pkt[29];
	tpkt[35] = pkt[28];
#else //may be wrong
	tpkt[0] =  pkt[0];
	tpkt[1] =pkt[1];
	tpkt[2] = pkt[2];
	tpkt[3] = 0;

	tpkt[4] = pkt[3];
	tpkt[5] = pkt[4];
	tpkt[6] = pkt[5];
	tpkt[7] = pkt[6];
	tpkt[8] = pkt[7];
	tpkt[9] = pkt[8];
	tpkt[10] = pkt[9];
	tpkt[11] = 0;

	tpkt[12] = pkt[10];
	tpkt[13] = pkt[11];
	tpkt[14] = pkt[12];
	tpkt[15] = pkt[13];
	tpkt[16] = pkt[14];
	tpkt[17] = pkt[15];
	tpkt[18] = pkt[16];
	tpkt[19] = 0;

	tpkt[20] = pkt[17];
	tpkt[21] = pkt[18];
	tpkt[22] = pkt[19];
	tpkt[23] = pkt[20];
	tpkt[24] = pkt[21];
	tpkt[25] = pkt[22];
	tpkt[26] = pkt[23];
	tpkt[27] = 0;

	tpkt[28] = pkt[24];
	tpkt[29] = pkt[25];
	tpkt[30] = pkt[26];
	tpkt[31] = pkt[27];
	tpkt[32] = pkt[28];
	tpkt[33] = pkt[29];
	tpkt[34] = pkt[30];
	tpkt[35] = 0;
#endif
	/* write mode */
	HDMI_REG_OPCR = (1<<8) | (addr&0xff);
	HDMI_REG_ORP6PH = reg[0];
	HDMI_REG_ORSP6W0 = reg[1];
	HDMI_REG_ORSP6W1 = reg[2];
	HDMI_REG_ORSP6W2 = reg[3];
	HDMI_REG_ORSP6W3 = reg[4];
	HDMI_REG_ORSP6W4 = reg[5];
	HDMI_REG_ORSP6W5 = reg[6];
	HDMI_REG_ORSP6W6 = reg[7];
	HDMI_REG_ORSP6W7 = reg[8];

	// Casey, 20051018
	hdmi_EnableWriteRamPacket();

	return 0;
}



INT32 hdmi_gen_MPEG_infoframe(struct _MPEG_InfoFrame *mpeg) 
{
UINT8 pkt[32];
UINT32 checksum=0;
INT32 i;

	/* clear buffer */
	memset(pkt, 0, 32);
	/* header */
	pkt[0] = 0x80 | 0x05;
	pkt[1] = 1;			/* version = 1 */
	pkt[2] = 0x1f & 10;	/* len = 10 */
	pkt[3] = 0x00;	/* checksum = 0 */

	/* data */
	/* bit rate, 4 bytes */
	memcpy(&pkt[4], mpeg->bit_rate, 4);
	/* Source Device Information */
	pkt[8] = (mpeg->repeat <<4) | mpeg->Frame;
	
	/* count checksum */
	for(i=0; i<31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1)  & 0xff;

	/* set to RAM Packet */
	hdmi_SetRamPacket(HDMI_RAMPKT_MPEG_SLOT, pkt);
	hdmi_SetRamPacketPeriod(HDMI_RAMPKT_MPEG_SLOT, HDMI_RAMPKT_PERIOD);	
	return 0;

}

INT32 hdmi_gen_spd_infoframe(struct _SPD_InfoFrame *spd)
{
UINT8 pkt[32];
UINT32 checksum=0;
INT32 i;

	/* clear buffer */
	memset(pkt, 0, 32);
	/* header */
	pkt[0] = 0x80 | 0x03;
	pkt[1] = 1;			/* version = 1 */
	pkt[2] = 0x1f & 25;	/* len = 10 */
	pkt[3] = 0x00;	/* checksum = 0 */

	/* data */
	/* Vendor Name, 8 bytes */
	memcpy(&pkt[4], spd->name, 8);
	/* Product Description, 16 bytes */
	memcpy(&pkt[12], spd->desc, 16);
	/* Source Device Information */
	pkt[28] = spd->device;

	/* count checksum */
	for(i=0; i<31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1)  & 0xff;

	/* set to RAM Packet */
	hdmi_SetRamPacket(HDMI_RAMPKT_SPD_SLOT, pkt);
	hdmi_SetRamPacketPeriod(HDMI_RAMPKT_SPD_SLOT, HDMI_RAMPKT_PERIOD);
	return 0;

}




INT32 hdmi_gen_audio_infoframe(struct _Audio_InfoFrame *audio)
{
UINT8 pkt[32];
UINT32 checksum=0;
INT32 i;

	/* clear buffer */
	memset(pkt, 0, 32);
	/* header */
	pkt[0] = 0x80 | 0x04;
	pkt[1] = 1;			/* version = 1 */
	pkt[2] = 0x1f & 10;	/* len = 10 */
	pkt[3] = 0x00;	/* checksum = 0 */
	/* data */

	/* speaker */
	pkt[7] = audio->speaker;
	/* Down-mix Inhibits Flag and level shift value */
	pkt[8] =	(audio->dm_inh<<7) | (audio->lsv<<3);

	/* count checksum */
	for(i=0; i<31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1)  & 0xff;

	/* set to RAM Packet */
	hdmi_SetRamPacket(HDMI_RAMPKT_AUDIO_SLOT, pkt);
	hdmi_SetRamPacketPeriod(HDMI_RAMPKT_AUDIO_SLOT, HDMI_RAMPKT_PERIOD);	
	return 0;

}


/*

		Sampling Rate	Value		N
 	--------------------------------------------
		to header		0			x
		32k				1			4096
		44.1k			2			6272
		48k				3			6144
		88.2k			4			12544
		96k				5			12288
		176.4k			6			25088
		192k			7			24576
 	--------------------------------------------
*/

UINT32 sf_to_N[] = {0, 4096, 6272, 6144, 12544, 12288, 25088, 24576};


INT32 hdmi_setAudio(struct _Audio_InfoFrame *audio)
{

	/* change range */
	if(audio->rate<1 || audio->rate>7)
		return -1;
	if(audio->ch<1 || audio->ch>7)
		return -1;

	/* Disable CRP */
	HDMI_REG_ACRPCR = (1<<31);

	/* clear 20 bits for N */
	HDMI_REG_ACRPCR &= ~0xfffff;
	/* set N */
	HDMI_REG_ACRPCR |= sf_to_N[audio->rate];

	/* let HW counting CTS */
    	//HDMI_REG_ACRPTCSR = 27000;

	/*
		Audio
		Channel
		Count		Value
 	--------------------------
		to header	0
		2CH			1
		3CH			2
		4CH			3
		5CH			4
		6CH			5
		7CH			6
		8CH			7
 	--------------------------
	*/
	if (audio->type >= 2) {  // Non-PCM
			/* 2 channel */
			HDMI_REG_ASPCR = (0xf<<27) | (0xf<<23) | ((1<<7) | (1<<3));
//			HDMI_REG_ASPCR = (0xf<<27) | (0xf<<23) |((1<<7) | (1<<3));
			/* 2CH layout (default) */
		    HDMI_REG_ACACR = (0<<0)|(1<<3)|(2<<6)|(3<<9)|(4<<12)|(5<<15)|(6<<18)|(7<<21);	
		    
	} else {
		if(audio->ch >= 2) { // PCM
			/* 6 channel */
			HDMI_REG_ASPCR = (0x7<<15) | (1<<14)  | (0x7<<10) | (1<<9) | (1<<8) | 0x77;

			/* 6CH layout by CEA-861B table 22 */
			/* 	1	2	3	4	5	6	*/
			/* 	FL	FR	LFE	FC	RL	RR	*/
	    	HDMI_REG_ACACR = (0<<0)|(4<<3)|(6<<6)|(2<<9)|(1<<12)|(5<<15)|(3<<18)|(7<<21);
			//HDMI_REG_ACACR = (0<<0)|(1<<3)|(2<<6)|(3<<9)|(4<<12)|(5<<15)|(6<<18)|(7<<21);//add by keith, not yet prove
		} else {
			/* 2 channel */
			HDMI_REG_ASPCR = ((1<<4) | (1<<0));
			/* 2CH layout (default) */
    		HDMI_REG_ACACR = (0<<0)|(1<<3)|(2<<6)|(3<<9)|(4<<12)|(5<<15)|(6<<18)|(7<<21);
		}
	}

	/* connect Audio data path */

	/* Enable CRP */
	HDMI_REG_ACRPCR &= ~(1<<31);

	return 0;

}


INT32 hdmi_gen_avi_infoframe(struct _AVI_Infoframe *video)
{
UINT8 pkt[32];
UINT32 checksum=0;
INT32 i;
	
	/* clear buffer */
	memset(pkt, 0, 32);
	/* header */
	pkt[0] = 0x80 | 0x02;
	pkt[1] = video->version;
	pkt[2] = 0x1f & 13;	/* len = 13 */
	pkt[3] = 0x00;	/* checksum = 0 */
	/* data */
	pkt[4] =	(video->cformat<<5) | \
				((video->act_pic_ratio==0x8?0:1)<<4) | \
				(video->bar_info<<2) | \
				(video->scan_info);
	pkt[5] =	(video->colorimetry<<6) | \
				(video->pic_ratio<<4) | \
				(video->act_pic_ratio);
	pkt[6] =	(video->quan_range<<2) | \
				(video->scaling);

	if(video->version == 2) {
		pkt[7] = video->video_id;
		pkt[8] = video->repeat;
	}
	else {
		pkt[7] = 0;
		pkt[8] = 0;
	}

	/* bar info */
	if(video->bar_info) {
		pkt[9] = video->top_bar_end & 0xff;
		pkt[10] = video->top_bar_end>>8;
		pkt[11] = video->bottom_bar_start & 0xff;
		pkt[12] = video->bottom_bar_start>>8;
		pkt[13] = video->left_bar_end & 0xff;
		pkt[14] = video->left_bar_end>>8;
		pkt[15] = video->right_bar_start & 0xff;
		pkt[16] = video->right_bar_start>>8;
	}
	else {
		pkt[9] = 0;
		pkt[10] = 0;
		pkt[11] = 0;
		pkt[12] = 0;
		pkt[13] = 0;
		pkt[14] = 0;
		pkt[15] = 0;
		pkt[16] = 0;
	}

	/* count checksum */
	for(i=0; i<31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1)  & 0xff;

	/* set to RAM Packet */
	hdmi_SetRamPacket(HDMI_RAMPKT_AVI_SLOT, pkt);
	hdmi_SetRamPacketPeriod(HDMI_RAMPKT_AVI_SLOT, HDMI_RAMPKT_PERIOD);

	/*send RAM Packet more, debug for lossing AVI InfoFrame on 576p50/480p60 */
	if(0){//(video->video_id == 2 || video->video_id == 3 || video->video_id == 17 || video->video_id == 18){//fred
		hdmi_SetRamPacket(HDMI_RAMPKT_MPEG_SLOT, pkt);
		hdmi_SetRamPacketPeriod(HDMI_RAMPKT_MPEG_SLOT, HDMI_RAMPKT_PERIOD);
	}

	/*debug fail on 480p60 in test item:7-25: H_ACTIVE=719 pixels using Panasonic HDMI analyzer */
	if(0){//(video->video_id == 2 || video->video_id == 3){//fred
		//for 2885
		HDMI_REG_CR=HDMI_REG_CR|0x04;
		//HDMI_REG_DIPCCR =(((8-1)<<8) |2);
		//for 2881
		//HDMI_REG_DIPCCR = 	HDMI_REG_DIPCCR & ~(0xf);//HBIDIPCNT = 0. //kist test
	}
	return 0;

}



/***************************************************************************
 NAME
		dvi_Set
 DESCRIPTION
		This function set DVI/HDMI Tx digital part
 PARAMETERS
		none
 RETURN VALUES
		-1				input value out of range
		0				OK
*****************************************************************************/
INT32 hdmi_setVideo(INT8 mode, struct _AVI_Infoframe *video)
{
struct vid_table *vidP;

	/* get Video Timing and waveform setting */
	vidP = (struct vid_table *)hdmi_get_vid_table(video->video_id);
	if(!vidP)
		return -1;	/* No Such Video Timing */

	/* set TMDS serializer */
	tmds_SerializerSet(vidP->timing->clock);

	/* Enable DVI/HDMI Tx digital output */
	hdmi_Enable();

	if(mode == HDMI_MODE_DVI) {
		/* set scheduler for DVI */
		HDMI_REG_SCHCR = HDMI_REG_SCHCR_DVI;
	}
	else if(mode == HDMI_MODE_HDMI) {
		/* set scheduler */
		HDMI_REG_SCHCR = HDMI_REG_SCHCR_HDMI;
		/* keep-out windows */
		#if 0	//original code , next is modify to TVIA
		HDCP_REG_KOWR = HDCP_REG_KOWR_KO_H(57) | \
						HDCP_REG_KOWR_KO_START(509) | \
						HDCP_REG_KOWR_KO_END(655);
		#endif
		HDCP_REG_KOWR = HDCP_REG_KOWR_KO_H(58) | \
						HDCP_REG_KOWR_KO_START(509) | \
						HDCP_REG_KOWR_KO_END(655);
		/* opportunity windows */
		HDCP_REG_OWR =	HDCP_REG_OWR_OPP_START(513) | \
						HDCP_REG_OWR_OPP_END(529);
		
		/* set data island packet number for Video data active line */
		if(vidP->timing->interlace) {
			// interlaced
			HDMI_REG_DIPCCR = HDMI_REG_DIPCCR_WIDTH;
		}
		else {
			 // prograssive
            if(video->video_id==VIDEO_ID_720x480_60P_4_3 || video->video_id==VIDEO_ID_720x480_60P_16_9)
                HDMI_REG_DIPCCR = (((9-1)<<8) | (1-1));
            else
                HDMI_REG_DIPCCR = HDMI_REG_DIPCCR_NARROW;
		}
	}
	else
		return -1;


	/* setup active level */
	if(vidP->timing->level)
		HDMI_REG_SCHCR &= ~HDMI_REG_SCHCR_LOW_ACTIVE;
	else
		HDMI_REG_SCHCR |= HDMI_REG_SCHCR_LOW_ACTIVE;

	/* set up video color format */
	//HDMI_REG_SCHCR |= (video->cformat << 4);
	/*AVI value => HDMI reg value*/
	if(video->cformat == COLOR_FMT_RGB)
		HDMI_REG_SCHCR |= (0 << 4);
	else if(video->cformat == COLOR_FMT_YCBCR_422)
		HDMI_REG_SCHCR |= (3 << 4);
	else if(video->cformat == COLOR_FMT_YCBCR_444)
		HDMI_REG_SCHCR |= (2 << 4);
	else
		HDMI_REG_SCHCR |= (1 << 4);
	
	/* set YCbCr422 color format schedule */
	//if(video->cformat == 3)	//	YCbCr422
	if(video->cformat == COLOR_FMT_YCBCR_422)
		HDMI_REG_SCHCR |= HDMI_REG_SCHCR_REP422;
	else
		HDMI_REG_SCHCR &= ~HDMI_REG_SCHCR_REP422;

	/* setup video data repetetion */
	if(vidP->timing->repeat) {
		HDMI_REG_SCHCR |= HDMI_REG_SCHCR_REP_ACT_PXL;
		HDMI_REG_SCHCR |= HDMI_REG_SCHCR_REPEAT;//add by keith
	}
	else {
		HDMI_REG_SCHCR |= HDMI_REG_SCHCR_ACT_PXL;
		HDMI_REG_SCHCR &= ~HDMI_REG_SCHCR_REPEAT;//add by keith
	}

	HDMI_REG_SCR = 0;
	/* setup HDCP */
	/* enable HDCP */
	HDCP_REG_CR = (1<<31);
	/* Ri is updated every 128 frames, Pj is updated every 16 frame */
	HDCP_REG_ICR = ((128-1) << 8) | (16-1);

	/* enable TMDS output*/
	TMDS_REG_EODR0 |= TMDS_OUTPUT_ENABLE;

	return 0;
}


/***************************************************************************
 NAME
		dvi_Set
 DESCRIPTION
		This function set DVI Tx digital part
 PARAMETERS
		vformat			Video Timing Format,
						values check RTD2880 display interface in detail
		cformat			Color Format:
							RGB			0
							YCbCr444	2
							YCbCr422	3
 RETURN VALUES
		-1				input value out of range
		0				OK
*****************************************************************************/

/***************************************************************************
 NAME
		tmds_SerializerSet
 DESCRIPTION
		This function set DVI Tx analog part
 PARAMETERS
		setting			verified value for the following setting:
						0:	Video Pixel Clock 27MHz
						0:	Video Pixel Clock 74.25MHz
 RETURN VALUES
		-1				input value out of range
		0				OK
*****************************************************************************/
INT32 tmds_SerializerSet(INT32 setting)
{
	switch(setting) {
	default:
	case 2500:
	case 2700:
		/* value verified for 27MHz */
		HDMI_REG_CR  = HDMI_REG_CR |HDMI_REG_CR_FIFO_FILL;
		TMDS_REG_SCR0 = 0xc0c260f0;//0xC0c660f0;  //strong
		TMDS_REG_SCR1 = 0x000c8768;//0x000c876b;  //strong
		break;
	case 7425:
	case 7200:
		/* value verified for 74.25MHz and 72Mhz */
		HDMI_REG_CR = HDMI_REG_CR & (~(HDMI_REG_CR_BIT_REPE_AT_EN))|HDMI_REG_CR_FIFO_FILL;
#if 0 //CONFIG_AM_CHIP_MINOR == 8268
		TMDS_REG_SCR0 = 0xc04660f0;
#else		
		TMDS_REG_SCR0 = 0xc0c660f0;//0xC0c660f0;  //strong
#endif		
		TMDS_REG_SCR1 = 0x000c8768;//0x000c876b;  //strong
		break;
	case 14850:
		DPRINTK("for 148.5Mhz Clock\n");
		HDMI_REG_CR = HDMI_REG_CR & (~(HDMI_REG_CR_BIT_REPE_AT_EN))|HDMI_REG_CR_FIFO_FILL;
#if CONFIG_AM_CHIP_MINOR == 8268
		TMDS_REG_SCR0 = 0x40c660f0;//0xC0c660f0;  //strong
		TMDS_REG_SCR1 = 0x000c4363;//0x000c876b;  //strong
#else
		TMDS_REG_SCR0 = 0x70c660f0;//0xC0c660f0;  //strong
		TMDS_REG_SCR1 = 0x000c876b;//0x000c876b;  //strong
#endif
		break;
	}
	return 0;
}

static void setDispInferfaceForHDMI(void){
	
		//dispIH_setDigitalInterfaceHdmi(1);  //need lcd config fred
		//DPRINTK("dispIH_setDigitalInterfaceHdmi(1)\n");
	
}



#define _Start_Point_CEA_EXtension_Data 128




/* for DEBUG */
static unsigned int ll_reg[2];

int hdmi_tx_ioctl(unsigned int cmd, unsigned long arg)
{
unsigned int value;
unsigned char buf[256];
UINT8 isAudioMute;
UINT8 isHotPlug;
UINT8 isForceUnAuthHDCP;
UINT8 edid_data[256];
HDMI_COLOR_FMT color_fmt;	
UINT8 isColorFMTChanging = FALSE;
//volatile int dealy;
	//DPRINTK("start hdmi_tx_ioctl  \n");
	switch (cmd) {
		default:
			//DPRINTK("default return in hdmi_tx_ioctl  \n");
			return 0;//return -ENOTTY;



		case HDMI_MODE_SET:
			DPRINTK("HDMI_MODE_SET\n");
			/* HDMI Generic Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.generic.mode, (INT8 *)arg, sizeof(hdmiDB.generic.mode)))
				return -EFAULT;
			/* enable HDMI state */
			DPRINTK("mode = %d\n", hdmiDB.generic.mode);
			if(hdmiDB.generic.mode) {
				HDMI_REG_CR |= HDMI_REG_CR_RESET;		 //reset
				while(HDMI_REG_CR & HDMI_REG_CR_RESET) ;// wait
				HDMI_REG_CR |= HDMI_REG_CR_ENABLE;	 //enable HDMI
				/* set HPD */
				HDMI_REG_CR |= (1<<30);
				HDMI_REG_CR |= (1<<31);
				/* set debounce */
				HDMI_REG_CR |= (0xf<<24);
				//hdcpOper.state = HDCP_XMT_LINK_H0;
			//	hdcp_timer_init();
				//hdcp_XmtState(0);
				//runHDCPXmtState(0);//mark by keith
				
			}
			else{
				HDMI_REG_CR &= ~0x00000001;	//disable HDMI
			//	hdcp_timer_stop();
			}
			break;

		case HDMI_MODE_GET:
			DPRINTK("HDMI_MODE_GET\n");
			/* HDMI Generic Configuration Status*/
			if(hdmi_copy_to_user((char *)arg, (INT8 *)&hdmiDB.generic.mode, sizeof(hdmiDB.generic.mode)))
				return -EFAULT;
			break;

		case HDMI_AUDIO_SET:
			DPRINTK("HDMI_AUDIO_SET\n");
			/* HDMI Generic Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.generic.audio, (INT8 *)arg, sizeof(hdmiDB.generic.audio)))
				return -EFAULT;
			if(hdmiDB.generic.audio)
				/* Enable Audio FIFO input */
				HDMI_REG_ICR |= HDMI_AUDIO_ENABLE;
			else
				/* Disable Audio FIFO input */
				HDMI_REG_ICR &= ~HDMI_AUDIO_ENABLE;
			break;

		case HDMI_AUDIO_GET:
			DPRINTK("HDMI_AUDIO_GET\n");
			/* HDMI Generic Configuration Status*/
			if(hdmi_copy_to_user((char *)arg, (INT8 *)&hdmiDB.generic.audio, sizeof hdmiDB.generic.audio))
				return -EFAULT;
			break;

		case HDMI_AUDIO_MUTE_USING_ASPCR:
			DPRINTK("HDMI_AUDIO_MUTE_USING_ASPCR\n");
   			if(hdmi_copy_from_user((INT8 *)&isAudioMute, (INT8 *)arg, sizeof(UINT8)))
   			 return -EFAULT;

			if(hdmiDB.generic.mode == HDMI_MODE_HDMI) {

   				if(isAudioMute){
					HDMI_REG_ASPCR |= (0xf <<19);
					HDMI_REG_ASPCR &= ~(0xf <<15);
					HDMI_REG_ASPCR &= ~(0xff <<23);
					HDMI_REG_ASPCR &= ~(0xff);
				} else {
					if(hdmiDB.audio.ch >= 2)// 6 channel
						HDMI_REG_ASPCR = (0xf<<27) | (0xf<<23) | (0x7<<15) | (1<<14)  | (0x7<<10) | (1<<9) | (1<<8) | 0x77;
					else
						HDMI_REG_ASPCR = (0xf<<27) | (0xf<<23) | ((1<<4) | (1<<0));
				}
			}
   			break;

		case HDMI_AVMUTE_SET:
			DPRINTK("HDMI_AVMUTE_SET\n");
			/* HDMI Generic Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.generic.avmute, (INT8 *)arg, sizeof(hdmiDB.generic.avmute)))
				return -EFAULT;

			printk("driver PIG: hdmiDB.generic.avmute = %d\n", hdmiDB.generic.avmute);
			if(hdmiDB.generic.avmute){
				/* set GCP sent ENABLE AVMUTE every field */
				HDMI_REG_GCPCR = HDMI_GCP_ENABLE | HDMI_GCP_AVMUTE_ENABLE;					
				//If AVMute is changed, we call hdcp_XmtState to stop HDCP's Auth.
				//hdcp_XmtState(0);//add by keith
			//	runHDCPXmtState(0);		fred
			} else {
				/* set GCP sent DISABLE AVMUTE field */
				HDMI_REG_GCPCR = HDMI_GCP_ENABLE | HDMI_GCP_AVMUTE_DISABLE;
				//If AVMute is changed, we call hdcp_XmtState to re-start HDCP's Auth.
				//hdcp_XmtState(0);//add by keith
			//	runHDCPXmtState(0);    fred
			}
			break;

		case HDMI_AVMUTE_GET:
			DPRINTK("HDMI_AVMUTE_GET\n");
			/* HDMI Generic Configuration Status*/
			if(hdmi_copy_to_user((char *)arg, (INT8 *)&hdmiDB.generic.avmute, sizeof hdmiDB.generic.avmute))
				return -EFAULT;
			break;

		case HDMI_VIDEO_CONFIG_SET:
			DPRINTK("HDMI_VIDEO_CONFIG_SET\n");
			/* Video Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.video, (INT8 *)arg, sizeof(struct _AVI_Infoframe)))
				return -EFAULT;

			if(hdmiDB.video.video_id == VIDEO_ID_1280x720_60P_16_9) {//720P_60Hz				
				setDispInferfaceForHDMI();
			}
			
			
				hdmi_setVideo(hdmiDB.generic.mode, &hdmiDB.video);
				hdmi_gen_avi_infoframe(&hdmiDB.video);
			
			
			break;

		case HDMI_VIDEO_CONFIG_GET:
			DPRINTK("HDMI_VIDEO_CONFIG_GET\n");
			/* Video Configuration */
			if(hdmi_copy_to_user((INT8 *)arg, (INT8 *)&hdmiDB.video, sizeof(struct _AVI_Infoframe)))
				return -EFAULT;
			break;

		case HDMI_AUDIO_CONFIG_SET:
			DPRINTK("HDMI_AUDIO_CONFIG_SET\n");
			/* Audio Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.audio, (INT8 *)arg, sizeof(struct _Audio_InfoFrame)))
				return -EFAULT;
			if(hdmiDB.generic.mode == HDMI_MODE_HDMI) {
				hdmi_setAudio(&hdmiDB.audio);
				hdmi_gen_audio_infoframe(&hdmiDB.audio);
			}
			break;

		case HDMI_AUDIO_CONFIG_GET:
			DPRINTK("HDMI_AUDIO_CONFIG_GET\n");
			/* Audio Configuration */
			if(hdmi_copy_to_user((INT8 *)arg, (INT8 *)&hdmiDB.audio, sizeof(struct _Audio_InfoFrame)))
				return -EFAULT;
			break;
		case HDMI_PRODUCT_DESCRIPTION_SET:
			DPRINTK("HDMI_PRODUCT_DESCRIPTION_SET\n");
			/* Source Product Descriptor Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.spd, (INT8 *)arg, sizeof(struct _SPD_InfoFrame)))
				return -EFAULT;
			if(hdmiDB.generic.mode == HDMI_MODE_HDMI)
				hdmi_gen_spd_infoframe(&hdmiDB.spd);
			break;

		case HDMI_PRODUCT_DESCRIPTION_GET:
			DPRINTK("HDMI_PRODUCT_DESCRIPTION_GET\n");
			/* Source Product Descriptor Configuration */
			if(hdmi_copy_to_user((INT8 *)arg, (INT8 *)&hdmiDB.spd, sizeof(struct _SPD_InfoFrame)))
				return -EFAULT;
			break;
			
		case HDMI_MPEG_INFOFRAME_SET:
			DPRINTK("HDMI_MPEG_INFOFRAME_SET\n");
			/* MPEG_INFOFRAME Descriptor Configuration */
			if(hdmi_copy_from_user((INT8 *)&hdmiDB.mpeg, (INT8 *)arg, sizeof(struct _MPEG_InfoFrame)))
				return -EFAULT;
			if(hdmiDB.generic.mode == HDMI_MODE_HDMI)				
				hdmi_gen_MPEG_infoframe(&hdmiDB.mpeg);
			break;

		case HDMI_MPEG_INFOFRAME_GET:
			DPRINTK("HDMI_MPEG_INFOFRAME_GET\n");
			/* MPEG_INFOFRAME Descriptor Configuration */
			if(hdmi_copy_to_user((INT8 *)arg, (INT8 *)&hdmiDB.mpeg, sizeof(struct _MPEG_InfoFrame)))
				return -EFAULT;
			break;
			


		


  		
		

	}	/* end of switch */

	//DPRINTK("End hdmi_tx_ioctl  \n");
	//return -ENOTTY;
	return 0;
}






//////////edn of hdmi_drv
#ifdef AUTO_SET_COLOR_FMT
typedef enum _DS_HDMI_COLOR_FMT{
	HDMI_COLOR_FMT_RGB = 0x0,
	HDMI_COLOR_FMT_YCBCR_422 = 0x1,
	HDMI_COLOR_FMT_YCBCR_444 = 0x2
} DS_HDMI_COLOR_FMT;
#endif



static int dsDriverInterface_SetHDMIVideoTx(DS_VIDEO_TIMING_FMT	videoFmt){

	struct _AVI_Infoframe video;

	/* configure Video */
	video.version = 2;


	video.cformat = COLOR_FMT_RGB;		// RGB


	video.scan_info = 0;	// no scan info data
	video.quan_range =0x01;	//Limited Range
	video.scaling = 0;		// no known non-uniform scaling
	video.repeat = 0;		// no pixel data repetetion
	video.act_pic_ratio = 8;//8 for Sample as picture aspect.
	video.bar_info = 0;		// no bar info data
	video.top_bar_end = 0;
	video.bottom_bar_start = 0;
	video.left_bar_end = 0;
	video.right_bar_start = 0;
	switch(videoFmt) {
		case FMT_858x525_720x480_60P:
			video.video_id = VIDEO_ID_720x480_60P_4_3;	// 2 for 4:3, 3 for 16:9
			video.pic_ratio = ASPECT_RATIO_4_3;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU601;	// 1 for ITU601
			break;

		case FMT_1650x750_1280x720_60P:
		case FMT_3300x750_1280x720_30P:
		case FMT_3960x750_1280x720_25P:
		case FMT_4125x750_1280x720_24P:
		case FMT_1280x800_60P:
		case FMT_800x600_60P:
		case FMT_1250x810_1024x768_60P:
			video.video_id = VIDEO_ID_1280x720_60P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;
		case FMT_2200x1125_1920x1080_60I:
			video.video_id = VIDEO_ID_1920x1080_60I_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_858x525_720x480_60I:
			video.video_id = VIDEO_ID_720x480_60I_4_3;//6 for 4:3, 7 for 16:9
			video.pic_ratio = ASPECT_RATIO_4_3;	// 1 for 4:3, 2 for 16:9
			video.repeat = 1;	// 1 for pixel sent 2 times.
			video.colorimetry = COLORIMETRY_ITU601;	// 1 for ITU601
			break;

		case FMT_864x625_720x576_50P:
			video.video_id = VIDEO_ID_720x576_50P_4_3;//17 for 4:3, 18 for 16:9
			video.pic_ratio = ASPECT_RATIO_4_3;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU601;	// 1 for ITU601
			break;

		case FMT_1980x750_1280x720_50P:
			video.video_id = VIDEO_ID_1280x720_50P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_2640x1125_1920x1080_50I:
			video.video_id = VIDEO_ID_1920x1080_50I_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_864x625_720x576_50I:
			video.video_id = VIDEO_ID_720x576_50I_4_3;	//21 for 4:3, 22 for 16:9
			video.pic_ratio = ASPECT_RATIO_4_3;	// 1 for 4:3, 2 for 16:9
			video.repeat = 1;	// 1 for pixel sent 2 times.
			video.colorimetry = COLORIMETRY_ITU601;	// 1 for ITU601
			break;

		case FMT_2750x1125_1920x1080_24P:
			video.video_id = VIDEO_ID_1920x1080_24P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_2640x1125_1920x1080_25P:
			video.video_id = VIDEO_ID_1920x1080_25P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_2200x1125_1920x1080_30P:
			video.video_id = VIDEO_ID_1920x1080_30P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;

		case FMT_1920x1080_60P:
			video.video_id = VIDEO_ID_1920x1080_60P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;
		case FMT_2080x741_1920x720_59P:
			video.video_id = VIDEO_ID_1920x1080_60P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;
		case FMT_854x480_50P:
			video.video_id = VIDEO_ID_1920x1080_60P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;
		case FMT_1920x1080_50P:
			video.video_id = VIDEO_ID_1920x1080_50P_16_9;	//16:9
			video.pic_ratio = ASPECT_RATIO_16_9;	// 1 for 4:3, 2 for 16:9
			video.colorimetry = COLORIMETRY_ITU709;	// 2 for ITU709
			break;	
			
		case FMT_2304x1250_1920x1152_50I:
		default:
			video.video_id = VIDEO_ID_VOID;	//0 for no video code available
			video.pic_ratio = ASPECT_RATIO_VOID;	//0 for no data
			video.colorimetry = COLORIMETRY_VOID;	// no Colorimetry data
			break;
	}
	hdmi_tx_ioctl(HDMI_VIDEO_CONFIG_SET, (INT32)(&video));
	return 0;
}
#if MODULE_CONFIG_HDMI_HDCP_ENABLE
/***************************************************************************
 NAME
		hdmi_hot_plug_test
 DESCRIPTION
		This function check HDMI/DVI plug status
 PARAMETERS
		none
RETURN VALUES
		1				HDMI Rx is plug
		0				HDMI Rx is not plug
*****************************************************************************/

INT32 hdmi_hot_plug_test(void)
{
	if(HDMI_REG_CR & HDMI_REG_CR_HOTPLUG_STATUS)
		return 1;
	else
		return 0;
}

static int doHDP(void *data){
	DPRINTK("doHDP .....................\n");
	UINT8 edid_data[256];
	
	//#ifdef AUTO_SET_COLOR_FORMAT
	HDMI_COLOR_FMT color_fmt;	
	UINT8 isModeChanging = FALSE;
	UINT8 isColorFMTChanging = FALSE;
	//#endif

	/* main loop */
	while (1) {
		DPRINTK("doHDP :  in while ..................\n");
		interruptible_sleep_on(&do_HDP_thread_queue);
		if(do_HDP_thread_is_wakeup == 0){
			DPRINTK("do_HDP_thread stop!!\n");
			break;
		} else {
			do_HDP_thread_is_wakeup = 0;
		}

		//for match the AN-ACY1 of SHARP Repeater and SHARP TV at Tarzan.
		HDMI_REG_CR &= ~0x00000001;	//disable HDMI
		mdelay(100);
		HDMI_REG_CR |= 0x00000001;	//enable HDMI

		/*setup Polarity */
		HDMI_REG_SCHCR &= ~HDMI_REG_SCHCR_SYNC_INV;	
#if 0
		if(hdmiDB.video.video_id == 4) {//720P_60Hz
			setDispInferfaceForHDMI();
		}		

		if( hdmi_hot_plug_test() ){
			#ifdef HDCP_MSG			
			hdcp_sendHDCPAuthResultMessageToAP(HDMI_HPD_PLUG);
			#endif

			if( hdmi_get_edid(edid_data) ){
				//error handler
				memset(edid_data, 0xff, 256);
			}		
			#if 0
			INT32 i;
			printk("\n\n\n============ EDID in doHDP() start====================");
			for(i=0;i<256;i++) {	
				if(!(i%10)) printk("\n%2d: ", i/10);
				printk("%2x ", edid_data[i]);		
			}
			printk("\n============== EDID in doHDP() end==================\n\n\n");
			#endif

			//#ifdef AUTO_SET_COLOR_FORMAT
			if(hdmiDB.generic.pixelColorFormatByUser == COLOR_FMT_AUTO_DETECT) {
				color_fmt = _checkColorFMT(edid_data);//check color FMT, 20070420
			} else {
				color_fmt = hdmiDB.generic.pixelColorFormatByUser;
			}
			if(color_fmt != hdmiDB.video.cformat){
				hdmiDB.video.cformat = color_fmt;
				isColorFMTChanging = TRUE;				
			}else{
				isColorFMTChanging = FALSE;				
			}
			
			if( _checkHDMIMode(edid_data) ){ //HDMI mode
				printk("checkHDMIMode is HDMI mode \n");
				if(  hdmiDB.generic.mode != HDMI_MODE_HDMI ){
					hdmiDB.generic.mode = HDMI_MODE_HDMI;
					isModeChanging = TRUE;
				}else
					isModeChanging = FALSE;
			} else {//DVI mode
				printk("checkHDMIMode is DVI mode \n");
				if(hdmiDB.generic.mode != HDMI_MODE_DVI ){
					hdmiDB.generic.mode = HDMI_MODE_DVI;
					isModeChanging = TRUE;
				}else
					isModeChanging = FALSE;
			}

			#ifdef TURN_OFF_DATA_SEND_TO_RX_WHEN_UN_HOT_PLUG
				isColorFMTChanging = TRUE;isModeChanging=TRUE;
			#endif
			if(isColorFMTChanging || isModeChanging)
				setInterfaceMode(hdmiDB.generic.mode);
			
			/*#else
			if( _checkHDMIMode(edid_data) ){ //HDMI mode
				printk("checkHDMIMode is HDMI mode \n");
				#ifdef TURN_OFF_DATA_SEND_TO_RX_WHEN_UN_HOT_PLUG
					setInterfaceMode(hdmiDB.generic.mode);
				#else
				if(  hdmiDB.generic.mode != HDMI_MODE_HDMI){
					hdmiDB.generic.mode = HDMI_MODE_HDMI;
					setInterfaceMode(hdmiDB.generic.mode);
				}
				#endif
			} else {//DVI mode
				printk("checkHDMIMode is DVI mode \n");
				#ifdef TURN_OFF_DATA_SEND_TO_RX_WHEN_UN_HOT_PLUG
					setInterfaceMode(hdmiDB.generic.mode);
				#else
				if(hdmiDB.generic.mode != HDMI_MODE_DVI){
					hdmiDB.generic.mode = HDMI_MODE_DVI;
					setInterfaceMode(hdmiDB.generic.mode);
				}
				#endif
			}
			#endif*/		
	
		} else {
			#ifdef TURN_OFF_DATA_SEND_TO_RX_WHEN_UN_HOT_PLUG
			HDMI_REG_CR = HDMI_REG_CR & (~HDMI_REG_CR_ENABLE);
			#endif
			
			#ifdef HDCP_MSG	
			hdcp_sendHDCPAuthResultMessageToAP(HDMI_HPD_UNPLUG);
			#endif
		}
#endif		
		runHDCPXmtState(0);

	}
	
	DPRINTK("doHDP ................  Done \n");
	return 0;
}


static int verifyRiRip(void *data){
	DPRINTK("verifyRiRip .....................\n");
	/* main loop */
	while (1) {
		
		DPRINTK("verifyRiRip :  in while ..................\n");
		//interruptible_sleep_on_timeout(&verify_RiRip_thread_queue, 100*2);
		interruptible_sleep_on(&verify_RiRip_thread_queue);
		if(verify_RiRip_thread_is_wakeup == 0){
			DPRINTK("verify_RiRip_thread stop!!\n");
			break;
		} else {
			verify_RiRip_thread_is_wakeup = 0;
		}
		DPRINTK("verifyRiRip :  in hdcp_ReadRip ..................\n");
#if 0
		//volatile int delay = 0;
		while(1){
			/* read Rip */
			hdcp_ReadRip(hdcpOper.Rip);

			//mark by keith. If use many printk in this ISR, linux OS will not be fast to ack mvd.
			//DPRINTK("R0 = %02x%02x, ", hdcpOper.Ri[1], hdcpOper.Ri[0]);
			//DPRINTK("R0p = %02x%02x\n", hdcpOper.Rip[1], hdcpOper.Rip[0]);

			//if printk this, the probability of always re-try state is more higher. 
			DPRINTK("Ri = %02x%02x, Rip = %02x%02x\n", 
				hdcpOper.Ri[1], hdcpOper.Ri[0],  hdcpOper.Rip[1], hdcpOper.Rip[0]);//add by keith 
			
			if(hdcp_CheckAuthentication()) {//0 for fail
				//DPRINTK("NO PASS: hdcp_CheckAuthentication = %d \n", hdcp_CheckAuthentication() ); //mark by keith due to video frash.
				hdcpOper.retry--;
				if(hdcpOper.retry <= 0){
#ifdef HDCP_MSG
					hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
					printk("Ri!=Rip: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
					hdcp_ForceUnauthentication();
					HDMI_REG_CR &= ~0x00000001;	//disable HDMI			add by ciwu
					HDMI_REG_CR |= 0x00000001;	//enable HDMI			add by ciwu
					//hdmi_Reset();//add by keith
					//hdcp_XmtState(0);
					runHDCPXmtState(0);
					break;
				}
			} else {//-1 for success
				//DPRINTK("PASS: hdcp_CheckAuthentication = %d \n", hdcp_CheckAuthentication() ); //mark by keith due to video frash.
#ifdef HDCP_MSG
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_SUCCESS);
#endif			
				hdcpOper.retry = 3;
				break;
			}
			//for(delay=0;delay<3000;delay++);
			mdelay(100);
		}
#else
			/* read Rip */
			hdcp_ReadRip(hdcpOper.Rip);

			//mark by keith. If use many printk in this ISR, linux OS will not be fast to ack mvd.
			//DPRINTK("R0 = %02x%02x, ", hdcpOper.Ri[1], hdcpOper.Ri[0]);
			//DPRINTK("R0p = %02x%02x\n", hdcpOper.Rip[1], hdcpOper.Rip[0]);

			//if printk this, the probability of always re-try state is more higher. 
			//printk("Ri = %02x%02x, Rip = %02x%02x\n", 
				//hdcpOper.Ri[1], hdcpOper.Ri[0],  hdcpOper.Rip[1], hdcpOper.Rip[0]);//add by keith 
			
			if(hdcp_CheckAuthentication()) {//0 for fail
				//DPRINTK("NO PASS: hdcp_CheckAuthentication = %d \n", hdcp_CheckAuthentication() ); //mark by keith due to video frash.
#ifdef HDCP_MSG
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				DPRINTK("Ri!=Rip: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif
				hdcp_ForceUnauthentication();
				HDMI_REG_CR &= ~0x00000001;	//disable HDMI			add by ciwu
				HDMI_REG_CR |= 0x00000001;	//enable HDMI			add by ciwu
				//hdmi_Reset();//add by keith
				//hdcp_XmtState(0);
				runHDCPXmtState(0);							
			} else {//-1 for success
				//DPRINTK("PASS: hdcp_CheckAuthentication = %d \n", hdcp_CheckAuthentication() ); //mark by keith due to video frash.
#ifdef HDCP_MSG
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_SUCCESS);
#endif			
			}			
	
#endif

	}
	
	DPRINTK("verifyRiRip ................  Done \n");
	return 0;
}

static void hdcp_checkProcess(unsigned long arg)
{
 //Note that: In hdcp_checkProcess, it will make the video frash to use printk too much.

//volatile int i=0;

	DPRINTK("\nhdcp_checkProcess.........\n");
	DPRINTK("link = %d \n", hdmi_hot_plug_test());

	/* check HPD status change */
	if (outstanding_HDMI_CR & HDMI_REG_CR_HOTPLUG_PENDING) {
		outstanding_HDMI_CR &= ~HDMI_REG_CR_HOTPLUG_PENDING;
		DPRINTK(" HPD status change \n");
		mb();
		do_HDP_thread_is_wakeup = 1;
		wake_up_interruptible(&do_HDP_thread_queue);
	}


	/* check Ri update */
	if (outstanding_HDCP_SR & HDCP_SR_RI_UPDATE) {
		DPRINTK("... hdcp_CheckRiUpdate...\n");
		/* check Ri update and will clear pending bit */
		if( hdcp_CheckRiUpdate() ){
			DPRINTK(" hdcp_CheckRiUpdate = 1 \n");
		} else {
			DPRINTK(" hdcp_CheckRiUpdate = 0 \n");
		}				
		mb();
		verify_RiRip_thread_is_wakeup = 1;
		wake_up_interruptible(&verify_RiRip_thread_queue);
		DPRINTK("... hdcp_CheckRiUpdate....done\n");
	}


#if 0
	/* check Pj update */
	if((outstanding_HDCP_SR & HDCP_SR_PJ_UPDATE)) {
		/* check Pj update and will clear pending bit */
		hdcp_CheckPjUpdate();
		/* read Pjp */
		hdcp_ReadPjp(&hdcpOper.Pjp);
		/* TBD */
		/* Now, there is no HDMI Rx support Pj */
	#if 1//add by krith
		DPRINTK("Pj = %02x, Pjp = %02x \n", hdcpOper.Pj, hdcpOper.Pjp);//add by keith
		if(hdcp_CheckAuthentication_Pj()) {//0 for fail
#ifdef HDCP_MSG
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_FAIL);
				printk("Pj!=Pjp: hdcp_sendHDCPAuthResultMessageToAP\n");
#endif			
				hdcp_ForceUnauthentication();
				HDMI_REG_CR &= ~0x00000001;	//disable HDMI	
				HDMI_REG_CR |= 0x00000001;	//enable HDMI	
				//hdcp_XmtState(0);
				runHDCPXmtState(0);
		} else {//-1 for success	
#ifdef HDCP_MSG
				hdcp_sendHDCPAuthResultMessageToAP(HDCP_AUTH_SUCCESS);
#endif				
			hdcpOper.retry = 3;
		}		
	#endif
	}
#endif
}


/*
 * This function is for HDMI Tx interrupt handler.
 *
 * @param irq: [input] The irq number.
 * @param dev_id: [input] The device id or data.
 * @param regs: [input]  The structure of registers.
 *
 * @return none
 * @ingroup hdmi
 */

#define HDMI_REG_CEC_TxCR0			HDMI_REG(0x0A8)
#define HDMI_REG_CEC_RxCR0			HDMI_REG(0x0A4)
irqreturn_t hdmi_intHandler(INT32 irq, void *dev_id, struct pt_regs *regs)
{
	int hdcp_update = 0;

	/* check HPD status change */
	if((HDMI_REG_CR & HDMI_REG_CR_HOTPLUG_INT) && (HDMI_REG_CR & (1<<30))) {
		// Re-schedule the CI tasklet
		outstanding_HDMI_CR = HDMI_REG_CR;
		HDMI_REG_CR |= HDMI_REG_CR_HOTPLUG_PENDING;
		tasklet_schedule(&hdmi_irq_tasklet);
		return IRQ_HANDLED;
	}
	/* check Ri update */ 	/* check Pj update */
	else if((HDCP_REG_SR & HDCP_SR_RI_UPDATE) || (HDCP_REG_SR & HDCP_SR_PJ_UPDATE)) {
		// Re-schedule the CI tasklet
		outstanding_HDCP_SR = HDCP_REG_SR;
		tasklet_schedule(&hdmi_irq_tasklet);
		/* clear pending */
                #if 1 //modify by YT
		HDCP_REG_SR |= (HDCP_SR_RI_UPDATE | HDCP_SR_PJ_UPDATE);
                #else
                HDCP_REG_SR |= HDCP_SR_PJ_UPDATE;
                #endif
		return IRQ_HANDLED;
	}
	else if ((HDMI_REG_CEC_TxCR0 & (1<<6))||(HDMI_REG_CEC_RxCR0 & (1<<6)))
	{
		printk("HDMI TX: other hdmi interrupter,fix me CEC_TxCR0:0x%x CEC_RxCR0:0x%x\n",HDMI_REG_CEC_TxCR0,HDMI_REG_CEC_RxCR0);
		HDMI_REG_CEC_TxCR0 &= (1<<6);
		HDMI_REG_CEC_RxCR0 &= (1<<6);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

int loadHDCPPrivateKeys_form_Flash()
{
	INT32	i,j;
	int ret=0;
	struct _HDCP_PrivateKey pk;
	
	ret = am_get_config(LCM_HDCP_PKEY_PATH,(char*)&pk.ksv, DATA_OFFSET, sizeof(UINT8)*5);
	if(ret < 0)
	{
		printk(KERN_ERR"NOTICE:READ HDCP PKEY CONFIG FILE ERROR\n");
		return ret;
	}

	ret = am_get_config(LCM_HDCP_PKEY_PATH,(char*)&pk.key, DATA_OFFSET+sizeof(UINT8)*8, sizeof(UINT8)*280);
	if(ret < 0)
	{
		printk(KERN_ERR"NOTICE:READ HDCP PKEY CONFIG FILE ERROR\n");
		return ret;
	}
	
	/* set HDCP keys */
	hdmiDB.pk.pnt = 0x55;
	printk("\n");
	for(i=0; i<5; i++)
	{
		hdmiDB.pk.ksv[i] = pk.ksv[i];
		printk("0x%x ",pk.ksv[i]);
	}
	printk("\n\n");
	for(i=0; i<40; i++)
	{
		for(j=0; j<7; j++)
		{
			hdmiDB.pk.key[i][j] = pk.key[i][j];	
			printk("0x%x ",pk.key[i][j]);
		}
		printk("\n");
	}
	return ret;	
}

static int hdmi_tx_init(void)
{
	int ret;
	int nothing=0;
	//struct irqaction irq_hdmi;
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
	#endif		
	/* init HDMI interrupt */
	RegBitSet(2,GPIO_MFCTL4,20,19);
	HDMI_REG_CR |= HDMI_REG_CR_HOTPLUG_INT;
	HDMI_REG_CR |=  HDMI_REG_CR_ENABLE_HDCP;
	tasklet_init(&hdmi_irq_tasklet, hdcp_checkProcess, (unsigned long) HDMI_TX_DEVICE_NODE_ID);
	ret = request_irq(IRQ_HDMI, hdmi_intHandler,IRQF_SHARED, "hdmi_tx", (void *)HDMI_TX_DEVICE_NODE_ID);//add by keith
	if(ret == -EINVAL)
	{
		printk(KERN_ERR "hdmi_tx_init: Bad irq number or handler\n");
		return ret;
	}
	else if(ret == -EBUSY)
	{
		printk(KERN_ERR "hdmi_tx_init: IRQ %d busy, change your config\n",IRQ_HDMI);
		return ret;
	}
#if 0	
	/* Set interrupt mask register */
#define HDMI_IE 	(1<<4)
	rtd_outl( SYSCTRL_SIC_M_GIE0, rtd_inl(SYSCTRL_SIC_M_GIE0) | HDMI_IE);
	/* Set interrupt routing register */
#define HDMI_IRS_SHIFT	16
	rtd_outl( SYSCTRL_SIC_M_GIR3, (rtd_inl(SYSCTRL_SIC_M_GIR3) & ~(0x0f<< HDMI_IRS_SHIFT)) | (HDMI_IRQ<<HDMI_IRS_SHIFT));
#endif

	/* create a kthread for verify RiRip*/
	verify_RiRip_thread_is_wakeup = 0;
	verify_RiRip_thread_pid = kthread_run(verifyRiRip, (void*)(&nothing), "verify_RiRip");
	DPRINTK("%s %d hdcpXmtState_thread_pid=0x%x\n",__FILE__,__LINE__,verify_RiRip_thread_pid);
	if (!IS_ERR(verify_RiRip_thread_pid)){	
		printk("ERROR: kernel_thread(verifyRiRip, nothing, 0) \n");
	}

	/* create a kthread for do HDP*/
	do_HDP_thread_is_wakeup = 0;
	do_HDP_thread_pid = kthread_run(doHDP, (void*)(&nothing), "do_HDP");
	DPRINTK("%s %d hdcpXmtState_thread_pid=0x%x\n",__FILE__,__LINE__,do_HDP_thread_pid);
	if (!IS_ERR(do_HDP_thread_pid)) {
		printk("ERROR: kernel_thread(doHDP, nothing, 0) \n");
	}	
	hdcpXmtState_thread_init();


	hdcp_timer_init();
	//video_frash_timer_init();//add by keith

	
	//hdmi_set_pinmux();//2885 only , add by YT
	//loadHDCPPrivateKeys_form_Flash();

	hdmiDB.generic.hdcp = 1;

	if (HDMI_REG_CR & HDMI_REG_CR_HOTPLUG_STATUS) {
		runHDCPXmtState(0);
	}
	return 0;
}

static void hdmi_tx_exit(void)
{	
	DPRINTK("hdmi_tx_exit. \n");

	free_irq(IRQ_HDMI,(void *)HDMI_TX_DEVICE_NODE_ID);
	
	/* shutdown the thread if there was one */
	if (verify_RiRip_thread_pid) {
		//stop it, but it need time to its cpu turn.
		verify_RiRip_thread_is_wakeup = 0;
		wake_up_interruptible(&verify_RiRip_thread_queue);
		
		//kill it
		kthread_stop(verify_RiRip_thread_pid);
		/*if (kill_proc(verify_RiRip_thread_pid, 0, 1) == -ESRCH)
		{
			printk("ERROR: hdmi_tx_release thread PID %d already died\n", verify_RiRip_thread_pid);
		} else {
			printk("SUCCESS: hdmi_tx_release thread PID %d already died\n", verify_RiRip_thread_pid);;
		}*/
	}	
	
	if (do_HDP_thread_pid) {
		//stop it, but it need time to its cpu turn.
		do_HDP_thread_is_wakeup = 0;
		wake_up_interruptible(&do_HDP_thread_queue);
		
		//kill it
		kthread_stop(do_HDP_thread_pid);
		/*if (kill_proc(do_HDP_thread_pid, 0, 1) == -ESRCH) 
		{
			printk("ERROR: hdmi_tx_release thread PID %d already died\n", do_HDP_thread_pid);
		} else {
			printk("SUCCESS: hdmi_tx_release thread PID %d already died\n", do_HDP_thread_pid);;
		}*/
	}

	hdcpXmtState_thread_kill();

	
	tasklet_kill(&hdmi_irq_tasklet);//add by keith
	hdcp_timer_stop();

	//video_frash_timer_stop();//add by keith
	
	//misc_deregister(&hdmi_tx_miscdev);
}
#endif


#define Start_Point_CEA_EXtension_Data 128


int Ds_DriverInterface_HdmiInit(DS_VIDEO_TIMING_FMT videoFmt, BOOL isAudioInfoFromAvp, struct _SPD_InfoFrame* pSpd) {

	UINT8	val8;
	struct _Audio_InfoFrame audio;
	struct _SPD_InfoFrame spd;

	//hdmi_tx_init(); //需要设置ri中断，执行hdmi_intHandler
	val8 = hdmiDB.generic.mode;
	 hdmi_tx_ioctl(HDMI_MODE_SET, (INT32)(&val8)) ;
	dsDriverInterface_SetHDMIVideoTx(videoFmt);
#if 1	// enable audio
	val8 = ENABLE;
//hdmi_tx_ioctl(HDMI_AUDIO_SET, (INT32)(&val8));//modified by jjf
//act_writel(0x111,HDMI_ACTL1);
	// configure auido
	
		audio.version = 1;
		audio.type= 0x01;//PCM
		audio.speaker = 0;//FR,FL
		audio.ch = 1;	/*
						Audio
						Channel
						Count		Value
				 	--------------------------
						to header	0
						2CH			1
						3CH			2
						4CH			3
						5CH			4
						6CH			5
						7CH			6
						8CH			7
				 	--------------------------
				 	*/
		audio.rate = 0x2;	//0x2 : 44.1khz  0x3:48k
		audio.bits = 0x3;   //BIT_NO_24
		audio.dm_inh = 0;//permitted or no information
		audio.lsv = 0;//0dB
		hdmi_tx_ioctl(HDMI_AUDIO_CONFIG_SET, (INT32)(&audio)) ;
	
#endif
	hdmi_tx_ioctl(HDMI_PRODUCT_DESCRIPTION_SET, (INT32)(pSpd)) ;
#if MODULE_CONFIG_HDMI_HDCP_ENABLE
		hdmi_tx_init(); //需要设置ri中断，执行hdmi_intHandler
#endif		

///add for debug fred

//HDMI_REG_ACRPCR=0x80000000;
///////fred	
	return 0;
}


int hdmi_init(DS_VIDEO_TIMING_FMT videoFmt,unsigned char hdmi_mode,struct _SPD_InfoFrame* pSpd){

	if(videoFmt == FMT_USER_DEFINED)
		videoFmt = FMT_1650x750_1280x720_60P;  
	RegBitSet(7,CMU_DISPLAYCLK2,20,18);//new add by fred
	act_writel(act_readl(CMU_DEVCLKEN)|0x34012,CMU_DEVCLKEN);
	switch (videoFmt)
	{
		case FMT_1280x800_60P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0x1bd/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)	
			 RegBitSet(0x1bd,CMU_LCDPLL,8,0);
#else
			 act_writel(0x10d521bd,CMU_LCDPLL);	//60hz
#endif			 
#endif
			 act_writel(0x00000081,CMU_DISPLAYCLK);
			 break;
		case FMT_800x600_60P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0x141/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else		
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)	
			 RegBitSet(0x141,CMU_LCDPLL,8,0);
#else
			 act_writel(0x10d521bd,CMU_LCDPLL);	//60hz
#endif			 
#endif			
			 act_writel(0x00000082,CMU_DISPLAYCLK);
			 break;	 
		case FMT_1920x1080_60P:
		case FMT_1920x1080_50P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else			
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)	
			 RegBitSet(0x18d,CMU_LCDPLL,8,0);
#else
			 act_writel(0x10d5218d,CMU_LCDPLL);	//60hz
#endif	
#endif	
		     act_writel(0x80,CMU_DISPLAYCLK);
			 break;
		case FMT_854x480_50P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0xa5/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)	
			 RegBitSet(0xa5,CMU_LCDPLL,8,0);
#else
			 act_writel(0x10d520a5,CMU_LCDPLL);	//60hz
#endif				
#endif				
		     act_writel(0x1081,CMU_DISPLAYCLK);
			 break;
		case FMT_2200x1125_1920x1080_60I:
		case FMT_2640x1125_1920x1080_50I:	
		case FMT_2200x1125_1920x1080_30P:
		case FMT_2640x1125_1920x1080_25P:
		case FMT_2750x1125_1920x1080_24P:	
		case FMT_1650x750_1280x720_60P:
		case FMT_3300x750_1280x720_30P:
		case FMT_1980x750_1280x720_50P:
		case FMT_3960x750_1280x720_25P:
		case FMT_4125x750_1280x720_24P:			
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)				
			 RegBitSet(0x18d,CMU_LCDPLL,8,0);
#else
			 act_writel(0x10d5218d,CMU_LCDPLL); 
#endif
#endif
			 act_writel(0x81,CMU_DISPLAYCLK); 
			 break;
		case FMT_2304x1250_1920x1152_50I:
		case FMT_858x525_720x480_60I:	
		case FMT_864x625_720x576_50I:
		case FMT_858x525_720x480_60P:
		case FMT_864x625_720x576_50P:	
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0x169/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)	
			 RegBitSet(0x169,CMU_LCDPLL,8,0);;
#else
			 act_writel(0x10d52169,CMU_LCDPLL);
#endif
#endif
			 act_writel(0x84,CMU_DISPLAYCLK); 
			 break;
		case FMT_1250x810_1024x768_60P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0xad/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
			RegBitSet(0xad,CMU_LCDPLL,8,0);
#else
			act_writel(0x13d520ad,CMU_LCDPLL);	//60hz
#endif		
#endif		
			act_writel(0x80,CMU_DISPLAYCLK);
			break;
		case FMT_2080x741_1920x720_59P:
#if CONFIG_AM_CHIP_MINOR == 8268
			 RegBitSet(0xf9/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#else
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
			RegBitSet(0xf9,CMU_LCDPLL,8,0);
#else
			act_writel(0x13d520f9,CMU_LCDPLL);	//60hz
#endif		
#endif		
			act_writel(0x80,CMU_DISPLAYCLK);
			break;
		default:
			 break;				
	}
	hdmiDB.generic.mode = hdmi_mode;
	Ds_DriverInterface_HdmiInit(videoFmt, 0,pSpd);
	return 0;
}

int hdmi_close(){
#if MODULE_CONFIG_HDMI_HDCP_ENABLE
	if (hdmiDB.generic.hdcp ==1)
		hdmi_tx_exit();
#endif	
	HDMI_REG_CR &= ~0x00000001;	//disable HDMI
	act_writel(act_readl(CMU_DEVCLKEN)&(~0x10010),CMU_DEVCLKEN);
	return 0;
}
//int main(){}
