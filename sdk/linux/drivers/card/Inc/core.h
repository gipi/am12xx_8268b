#ifndef CORE_H
#define CORE_H

#include "am_types.h"
#include "sys_cfg.h"

//#define PAGE_TRANS_N

/* card module  clock */
#define	CLOCK_SELECT_20		0x0
#define	CLOCK_SELECT_24		0x1
#define	CLOCK_SELECT_30		0x2			// default value
#define	CLOCK_SELECT_40		0x3
#define	CLOCK_SELECT_48		0x4
#define	CLOCK_SELECT_60		0x5
#define	CLOCK_SELECT_80		0x6
#define	CLOCK_SELECT_96		0x7
#define 	CLOCK_SELECT_120		0x8

/*stop specific module command */
#define 	STOP_CF				0x01
#define	STOP_SM				0x02//xd
#define 	STOP_SD				0x04
#define	STOP_MS				0x08

/*Rts5121 card reader specific command defination */
#define 	CARD_STOP			0x83  //stop command 
#define 	CARD_SWITCHCLOCK	0x0080

#define 	CF_REGISTERCHECK	0x0016
#define	SM_REGISTERCHECK	0x0033
#define 	SD_REGISTERCHECK	0x0052
#define	 MS_REGISTERCHECK	0x0074

#define 	GPIO_CONFIGURE		0x0014
#define 	GPIO_OUTPUT			0x0015


#define	 GPIO_INTENABLE		0x0019
#define 	GPIO_INTPENDING		0x001A
#define 	GPIO_INTCLEAR		0x001B


#define	 CF_RESET			0x0010
#define 	CF_READPORT			0x0002
#define 	CF_WRITEPORT		0x0003
#define 	CF_SETTIMING		0x0012
#define 	CF_READONESECTOR	0x0004
#define 	CF_AUTOREAD			0x0008
#define 	CF_AUTOWRITE		0x0009
#define 	CF_NORMALREAD		0x000C
#define 	CF_NORMALWRITE		0x000D
// { for CF card 8bit mode
#define 	CF_CONFIGURE		0x0013
// }
#define 	CF_ULTRADMAREAD				0x000A
#define 	CF_ULTRADMAWRITE			0x000B
#define 	CF_ULTRADMASETPARAMETER		0x0018
#define 	CF_ULTRADMANORMALREAD		0x000E
#define 	CF_ULTRADMANORMALWRITE		0x000F

#define	SM_SETTIMING		0x0031
#define 	SM_CONTROL			0x0032
#define 	SM_RESET			0x0020
#define 	SM_ERASE			0x0021
#define	 SM_READSTATUS		0x0022
#define 	SM_READID			0x0023
#define 	SM_READREDUNDANT	0x0024
#define 	SM_READPAGES		0x0025
#define 	SM_WRITEPAGES		0x0028
#define 	SM_WRITEREDUNDANT	0x002A
#define 	SM_SENDCOMMAND		0x0026

#define 	SM_COPYBACK			0x002C

#define	 SM_SETADDRESS		0x002B
#define 	SM_NORMALREAD		0x0027
#define 	SM_NORMALWRITE		0x0029


#define 	SD_SETPARAMETER		0x0051
#define 	SD_GETSTATUS		0x0052
#define 	SD_SENDCMDGETRSP	0x0048
#define 	SD_NORMALREAD		0x004C
#define 	SD_NORMALWRITE		0x0040
#define 	SD_AUTOREAD1		0x004D
#define 	SD_AUTOREAD2		0x004E
#define 	SD_AUTOREAD3		0x0045
#define 	SD_AUTOREAD4		0x0046
#define 	SD_AUTOWRITE1		0x0049
#define 	SD_AUTOWRITE2		0x004A
#define 	SD_AUTOWRITE3		0x0041
#define 	SD_AUTOWRITE4		0x0042


#define 	MS_SETPARAMETER		0x0071
#define 	MS_READBYTES		0x0060
#define 	MS_WRITEBYTES		0x0064
#define 	MS_AUTOREAD			0x0068
#define 	MS_AUTOWRITE		0x006C
#define 	MS_NORMALREAD		0x0061
#define	 MS_NORMALWRITE		0x0065
#define 	MS_GETSTATUS		0x0072

#define 	MS_GETSECTORNUM		0x0073
#define 	MS_SETCMD			0x0066
#define 	MS_MULTIREAD		0x0062
#define 	MS_MULTIWRITE		0x0063
#define 	MS_COPYPAGE		0x0067

/*data transfer direction*/
#define	CARD_DATA_IN		0
#define	CARD_DATA_OUT		1

/*error num definations*/
#define	CARD_GOOD			0
#define	CARD_STALLED		1
#define	CARD_TIMEOUT		2
#define	CARD_NOMEM			3
#define	CARD_FAILED			4
#define	CARD_ERROR			5
#define	CARD_OC_NOW		8
#define	CARD_OC_EVER		9
#define	CARD_PROGRAM_ERROR	15
#define	CARD_WP_ERROR		16

/*card type definations*/
typedef enum{
	TYPE_NONE =0,
	TYPE_SD=1,
	TYPE_MMC=2,
	TYPE_MS_STICK=3,
	TYPE_MS_PRO=4,
	TYPE_XD=5,
	TYPE_CF=6,
}card_type_t;


/*card reader timeout definations*/
#define 	CMD_TIMEOUT		10000
#define 	CRC_CMD_HZ    	10000*5000
//@fish add val 50000 delay 4ms  2010-10-19 delay timeout -->32ms
#define 	CRC_CMD_TIME    	(50000*8)  // (CMD_TIMEOUT*HZ)/1000 
#define 	CRC_HZ    		5000

/*card wp definations*/
#define	CARD_WRITE_NoProtect	0x00
#define 	CARD_WRITE_Protect	0x01


/*read write operation definations*/
#define 	READ_OP		0x00  //0x000000
#define 	WRITE_OP		0x01 //0x010000
/*
#define		Card_Cardtype		0	//(null)
#define		Card_CFCardtype		2	//(null)
#define		Card_CardMultiPin		4	//(null)
#define		Card_MSStandBUS		6	//(null)
*/
typedef struct EZTool_Card_Data{
	INT16U    CardType;
	INT16U    CFCardType;
	INT16U    SDMulitPin;
	INT16U    MSMulitPin;
	INT16U    XDMulitPin;
	INT16U    MSStandardBus;
	INT16U    CF_DataBus;	
}EZToolCardData;

////////////////////////////////////////////////////////////////////////
typedef struct card_Com_info{
        unsigned char  SDPower;
        unsigned char   SDDet;
        unsigned char   SDWP;
        unsigned char   XDDet;
        unsigned char   XDPower;
        unsigned char   CFDet;
        unsigned char   CFPower; 
        unsigned char   CF_OE;
        unsigned char   MS_Bus_1Bit;
        unsigned char   CF_Bus_Width;
        unsigned int   ChipType;
        void   *pri_data;    
        EZToolCardData   Data;
}card_common_info;



/***************************************************************/
/***************************************************************/
/*Debug info functions*/
#ifdef DEBUG_CARD
	#define	PERR()	do{ printk(KERN_ALERT"\nfile:%s\tline: %d\tfuntion:%s\n",\
				__FILE__,__LINE__,__FUNCTION__);}while(0)
#else
	#define 	PERR()	do{;}while(0)
#endif

#define DEBUG_SD
#ifdef DEBUG_SD
#define Trace_SD(x...)   printk(x)  //do{printk(x);}while(0
#else
#define Trace_SD(x...) do { } while (0)
#endif

#define MS_DEBUG
#ifdef MS_DEBUG
	#define Trace_MS(x...)	do{printk(x);}while(0)
#else
	#define Trace_MS(x...)	do{;}while(0)
#endif

#define CF_DEBUG
#ifdef CF_DEBUG
	#define Trace_CF(x...)	do{printk(x);}while(0)
#else
	#define Trace_CF(x...)	do{;}while(0)
#endif

#define XD_DEBUG
#ifdef XD_DEBUG
	#define Trace_XD(x...)	do{printk(x);}while(0)
#else
	#define Trace_XD(x...)	do{;}while(0)
#endif

#define DEBUG_DET
#ifdef 	DEBUG_DET
#define	Trace_Det(x...)			printk(x)
#else
#define    Trace_Det(x...)
#endif

#define DEBUG_CREADER
#ifdef    DEBUG_CREADER
#define   Trace_CRC(x...)			printk(x)
#else
#define   Trace_CRC(x...)		 
#endif

//#define __DEBUG_3__
#ifdef __DEBUG_3__
#define TraceMsg(x...) printk("Trace-->:" x)
#else
#define TraceMsg(x...) do { } while (0)
#endif

#define DetBUG(x...)	printk(x)



#define __INIT_DEBUG__
#ifdef __INIT_DEBUG__
#define INIT_BUG(x...) printk("INIT:" x)
#else
#define INIT_BUG(x...) do { } while (0)
#endif


#if 0
#define  Card_Bug(fmt,msg...)  printk(fmt,##msg)
#else
#define  Card_Bug(fmt,msg...)  
#endif

#define SD_GUG_ON
#ifdef  SD_GUG_ON
#define  SD_BUG(fmt,msg...)  printk(fmt,##msg)
#else
#define  SD_BUG(fmt,msg...)  
#endif

#if 0//#ifdef MS_DEBUG
#define msdbg(fmt,msg...)    printk(fmt,##msg)
#else
#define msdbg(...)     do { } while (0)
#endif
#include "linux/errno.h"
#define SD_ERR2_OS_ERR(err) (err?-EIO:err)


/***************************************************************/
/***************************************************************/

/*  AL1203 for 7531 CARDmulti pin
    CF_CD --> GPIO32, CF_OE --> GPIO56, CF_PWR -->GPIO29
    SD_CD --> GPIO31, SD_PWR-->GPIO55, SD_WP  --> GPIO14
    
    CF_D0,CF_D2~D7 [14:12],CF_D1[31:29]
    CF_WE[19:16],CF_RE[26:24],CF_CS1[28]           -->MF3
    CF_A0~A2 [2:0,5:4,9:8]                                          -->MF2
    CF_D8~D15[29:28]                                                 -->MF1
    
    xD_D0~xD_D7 xD_WE[19:16],XD_RE[22:20] ,
    xD_RB[10:8],XD_CE[6:4]],xD_CLE[2:0]              ---------->MF3
    xD_ALE[30:28]                                                      ---------->MF2
    
    MS_D0,D2,D3[14:12],MS_D1[31:29]    
    MS_BS,MS_CLK                                                    ---------->MF3
    
    SD_D0,D2,D3[14:12],SD_D1[31:29], 
    SD_CMD[19;16],MS_CLK[22;20]                             ---------->MF3
*/
#define SD_MF3_MSK_7531    ~((7<<12)|(15<<16)|(7<<20)|(7<<29))
#define SD_MF3_VAL_7531       ((1<<12)|(1<<16)|(1<<20)|(1<<29))

#define MS_MF3_MSK_7531    ~((7<<12)|(15<<16)|(7<<20)|(7<<29))
#define MS_MF3_VAL_7531       (3<<12)|(3<<16)|(3<<20)|(3<<29)

#define XD_MF3_MSK_7531    ~((7<<0)|(7<<4)|(7<<8)|(7<<12)|(15<<16)|(7<<20)|(7<<29))
#define XD_MF3_VAL_7531       (5<<0)|(6<<4)|(1<<8)|(4<<12)|(7<<16)|(6<<20)|(4<<29)
#define XD_MF2_MSK_7531    ~((7<<28))
#define XD_MF2_VAL_7531       (1<<28)

#define CF_MF3_MSK_7531     ~((7<<12)|(15<<16) |(7<<24)|(1<<28)|(7<<29))
#define CF_MF3_VAL_7531       ((5<<12)|(8<<16) |(5<<24)|(1<<28)|(5<<29))
#define CF_MF2_MSK_7531     ~((7<<0)|(3<<4)|(3<<8))
#define CF_MF2_VAL_7531        ((2<<0)|(2<<4)|(2<<8))
#define CF_MF1_MSK_7531    ~((3<<28))
#define CF_MF1_VAL_7531       (1<<28)

/*  AL1203 for 7331 CARDmulti pin
    SD_CD -->  MFP4 EXTINT1 GPIO31, SD_PWR-->CF_RESET,MFP3 GPIO26, 
    SD_WP-->MFP1 PLCD8 GPIO14    
    xD_D0~xD_D7 xD_WE,XD_RE ,
    xD_RB,XD_CE,xD_CLE              ---------->MF3    
    xD_ALE                                       ---------->MF2
    MS_D0~D7 MS_BS,MS_CLK     ---------->MF3
    SD_D0~D7 MS_BS,MS_CLK      ---------->MF3
*/
//#define MS_D1_SELF  

#define SD_MF3_MSK_7331    ~((7<<12)|(15<<16)|(7<<20)|(7<<29))
#define SD_MF3_VAL_7331       (1<<12)|(1<<16)|(1<<20)|(1<<29)


#define MS_MF3_MSK_7331    ~((7<<12)|(15<<16)|(7<<20)|(7<<29)) 
#define MS_MF3_VAL_7331       (3<<12)|(3<<16)|(3<<20)|(3<<29)

#define MS_MF3_MSK_7331A    ~((7<<12)|(15<<16)|(7<<20)|(7<<24)) 
#define MS_MF3_VAL_7331A       (3<<12)|(3<<16)|(3<<20)|(2<<24)


// xD_WE->2:0,XD_RE->6:4,XD_RB ->10:8  XD_CLE->19:16 XD_CE->22:20 
#define XD_MF3_MSK_7331    ~((7<<0)|(7<<4)|(7<<8)|(7<<12)|(15<<16)|(7<<20)|(7<<29))
#define XD_MF3_VAL_7331       (1<<0)|(1<<4)|(1<<8)|(4<<12)|(4<<16)|(4<<20)|(4<<29)
//XD_ALE->30:28
#define XD_MF2_MSK_7331     ~((7<<28))
#define XD_MF2_VAL_7331        ((1<<28))



//AL1203 75555
//MF  SD_D3,D2[2:0],SD_D1[6:4],SD_D0[10:8]
#define SD_MF5_MSK_7555    ~((7<<0)|(7<<4)|(7<<8))
#define SD_MF5_VAL_7555       ((2<<0)|(2<<4)|(2<<8))
#define SD_MF5_GPO_7555       ((7<<0)|(7<<4)|(7<<8))

#define SD_MF4_MSK_7555    ~((7<<12)|(7<<16))  // ~((7<<12)|(7<<16)|(0xf<<20)|(0xf<<24)|(0xf<<28))
#define SD_MF4_VAL_7555     ((1<<12)|(1<<16))   //   ((1<<12)|(1<<16)|(0x2<<20)|(0x2<<24)|(0x2<<28))
#define SD_MF4_GPO_7555     ((5<<12)|(5<<16))   //   ((1<<12)|(1<<16)|(0x2<<20)|(0x2<<24)|(0x2<<28))


#define SD_MF4_MSK_7555_    ~((7<<12)|(7<<16))
#define SD_MF4_VAL_7555_       ((1<<12)|(1<<16))

//Demo 7555 D3,D1,D0
#define SD_MF5_MSK_7555_    ~((7<<16)|(7<<20)|(7<<24))
#define SD_MF5_VAL_7555_       ((2<<16)|(2<<20)|(2<<24))
#define SD_MF5_GPO_7555_       ((0<<16)|(0<<20)|(0<<24))

//Demo 7555 D2
#define SD_MF8_MSK_7555_    ~((7<<12))
#define SD_MF8_VAL_7555_       ((2<<12))
#define SD_MF8_GPO_7555_       ((0<<12))


#define MS_MF5_MSK_7555_1Bit       ~((7<<0)|(7<<8))
#define MS_MF5_VAL_7555_1Bit       ((3<<0)|(3<<8))
#define MS_MF1_MSK_7555_1Bit       ~((7<<28))
#define MS_MF1_VAL_7555_1Bit        ((1<<28))

#define MS_MF5_MSK_7555    ~((7<<0)|(7<<4)|(7<<8))
#define MS_MF5_VAL_7555       ((3<<0)|(3<<4)|(3<<8))
#define MS_MF4_MSK_7555    ~((7<<12)|(7<<16))
#define MS_MF4_VAL_7555       ((3<<12)|(3<<16))


//#define MS_MF4_MSK_7555    ~((7<<12)|(7<<16) |(0xf<<20)|(0xf<<24)|(0xf<<28))
//#define MS_MF4_VAL_7555       ((3<<12)|(3<<16)|(0x3<<20)|(0x3<<24)|(0xf<<28))

/***for QC **********************
MF5:  
[2:0] P_NFD3 P_NFD2 XD_D3,XD_D2
[6:4] P_NFD1 XD_D1
[10:8] P_NFD0 XD_D0
MF4: 
[6:4]  P_NFCLE XD_CLE,
[10:8] P_NFALE XD_ALE
[14:12] XD_CE
[18:16] XD_WE
[23:20 ]P_NFD7,P_NFD6   XD_D7,XD_D6
[27:24] P_NFD5 XD_D5,
[31:28] P_NFD4 XD_D4
MF3:
[25:24]P_NFRB  XD_RB
[30:28]P_NFRE  XD_RE
***/
#define XD_MF5_MSK_7555    ~((7<<0)|(7<<4)|(7<<8))
#define XD_MF5_VAL_7555       ((4<<0)|(4<<4)|(4<<8))
#define XD_MF4_MSK_7555    ~((7<<4)|(7<<8)|(7<<12)|(7<<16)|(0xf<<20)|(0xf<<24)|(0xf<<28))
#define XD_MF4_VAL_7555       ((2<<4)|(2<<8)|(4<<12)|(4<<16)|(0x4<<20)|(0x4<<24)|(0x4<<28))
#define XD_MF3_MSK_7555    ~((3<<24)|(7<<28))
#define XD_MF3_VAL_7555       ((2<<24)|(3<<28))

/*** for QC CF 
MF2[26:24]P_CFA1:CF_A0
MF2[30:28]P_CFA0:CF_A1
MF3[19:16]P_XDCLE: CF_A0
MF3[30:28]P_NFRE: CF_A1
MF4[6:4]P_NFCLE: CF_A2
MF4[23:20]P_NFD7,D6: CF_D14,CF_D15
MF4[27:24]P_NFD5: CF_D13
MF4[31:28]P_NFD4: CF_D12
MF5[2:0] P_NFD3: D11,D10
MF5 [6:4] P_NFD1: CF_D9
MF5 [10:8] P_NFD0: CF_D8
MF5[14:12] P_CFD7:  CF_D7
MF5[18:16]P_CFD3:CF_D3
MF5[22:20]P_CFD1:CF_D1
MF5[26:24]P_CFD0:CF_D0
MF6[13:12]P_CFRE: CF_RE
MF6[18:16]P_CFWE:CF_WE
MF6[22:20]P_CFCS0:CF_CS0
MF7[2:0]P_CFD5: CF_D5
MF7[6:4]P_CFD4: CF_D4
MF7[22:20]P_LCD17:CF_CS1
MF8[10:8]P_CFD6: CF_D6
MF8[14:12]P_CFD2:CF_D2

***/
//#define CF_MF2_MSK_7555    ~((7<<24)|(7<<28))
//#define CF_MF2_VAL_7555       ((1<<24)|(1<<28))
//for CF_A1,CF_A0
#define CF_MF3_MSK_7555    ~((0xf<<16)|(7<<28))
#define CF_MF3_VAL_7555       ((6<<16)|(4<<28))
#define CF_MF4_MSK_7555    ~((7<<4)|(0xf<<20)|(0xf<<24)|(0xf<<28))
#define CF_MF4_VAL_7555       ((4<<4)|(0x6<<20)|(0x6<<24)|(0x6<<28))
#define CF_MF5_MSK_7555    ~((7<<0)|(7<<4)|(7<<8)|(7<<12)|(7<<16)|(7<<20)|(7<<24))
#define CF_MF5_VAL_7555       ((6<<0)|(6<<4)|(6<<8)|(1<<12)|(1<<16)|(1<<20)|(1<<24))
#define CF_MF6_MSK_7555    ~((3<<12)|(7<<16)|(7<<20))
#define CF_MF6_VAL_7555       ((1<<12)|(1<<16)|(1<<20))
#define CF_MF7_MSK_7555    ~((7<<0)|(7<<4)|(7<<20))
#define CF_MF7_VAL_7555       ((1<<0)|(1<<4)|(6<<20))
#define CF_MF8_MSK_7555    ~((7<<8)|(7<<12))
#define CF_MF8_VAL_7555       ((1<<8)|(1<<12))


//AL1220 AM5101t  zhoiujian
#define SD_MF4_MSK_5101    ~((3<<0)|(3<<3)|(3<<6)|(3<<9)|(3<<12)|(3<<15))
#define SD_MF4_VAL_5101     ((1<<0)|(1<<3)|(1<<6)|(1<<9)|(1<<12)|(1<<15))
#define MS_MF4_MSK_5101    ~((3<<0)|(3<<3)|(3<<6)|(3<<9)|(3<<12)|(3<<15))
#define MS_MF4_VAL_5101     ((2<<0)|(2<<3)|(2<<6)|(2<<9)|(2<<12)|(2<<15))

//AL1213 AM8250  lvjp
#define SD_MF2_MSK_8250    ~((1<<4)|(1<<7)|(1<<10)|(1<<13)|(1<<16)|(1<<19))
#define SD_MF2_VAL_8250     ((1<<4)|(1<<7)|(1<<10)|(1<<13)|(1<<16)|(1<<19))
#define SD_MF2_GPO_8250     ((8<<4)|(8<<7)|(8<<10)|(8<<13)|(8<<16)|(8<<19))

#define MS_MF2_MSK_8250    ~((2<<4)|(1<<7)|(2<<10)|(2<<13)|(2<<16)|(2<<19))
#define MS_MF2_VAL_8250     ((2<<4)|(1<<7)|(2<<10)|(2<<13)|(2<<16)|(2<<19))



//////////////////////////////////////////////

/*mem operations*/
#define LONG2CHAR(num,i)	*((unsigned char *)&(num) + (i))
#define SHORT2CHAR(num,i)	*((unsigned char *)&(num) + (i))

#define ASSIGN_SHORT(num,hbyte,lbyte)			*((unsigned char *)&(num)) = (lbyte); \
								*((unsigned char *)&(num)+1) = (hbyte); 
#define ASSIGN_LONG(num,byte3,byte2,byte1,byte0)	*((unsigned char *)&(num))  = (byte0); \
								*((unsigned char *)&(num)+1) = (byte1); \
								*((unsigned char *)&(num)+2) = (byte2); \
 								*((unsigned char *)&(num)+3) = (byte3);  

#define Little_LONG(num,byte3,byte2,byte1,byte0)	*((unsigned char *)&(num))  = (byte3); \
								*((unsigned char *)&(num)+1) = (byte2); \
								*((unsigned char *)&(num)+2) = (byte1); \
 								*((unsigned char *)&(num)+3) = (byte0);  		




//@fish add vaue :1  card && nand flash multi 
#define SDMUlTIPIN     Get_ComomInfo()->Data.SDMulitPin







/*basic data /command transfer between card and card module*/
INT32S card_send_msg(void * data, INT32S len, INT32S * actual_length, INT32S timeout);
INT32S card_recv_msg(void * data, INT32S len, INT32S * actual_length, INT32S timeout);
INT32S card_bulk_transfer(INT8U Card_DMA_NUM, INT32U direction, void * buf, INT32U length_left, INT32S * residual, INT32S timeout, INT8U bMode);

/*card module clock rate control  different card have to word on different clock rate*/
INT32S card_switchclock(INT8U  clock);

INT32U _Get_TimeOut2(INT8U  Flag );

/*stop specific card module*/
INT32S card_stop(INT32S card);

/*handle error */
INT32S card_handle_error(INT32S card, INT32U err);

/*group of gpio operations*/
void		setgpio(INT8U num,INT8U val);
INT8U	getgpio(INT8U num);
void 	power_on(INT8U num);
void	power_off(INT8U num);
INT8U	in_slot(INT8U num);

/*card read enable*/
void 	card_reader_on(void);
void 	card_reader_off(void);
void *malloc(INT32U num_bytes);
void free(void *p);
void Short_DelayUS(INT32U DelayTim);
INT32S card_switchclock(INT8U  clock_freq);
/*dump mem info*/
 void 	Card_Print_Buf( const INT8U    * pad, const INT8U * pData, INT16U inLen);
INT32S  card_get_dma_chan(INT8U Flag);
void  card_rel_dma_chan(  INT32S dmano);

void    Set_DMA_Mode(char Mode);
//Get Card Reader Read/Write Card type
void    Set_CardType(INT8U CardFlag);

  

#endif
