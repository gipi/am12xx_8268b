/*
 * aotg_regs.h -- for Actions ATJ213x AOTG 
 *
 */

#ifndef  __USDK130_AOTG_REGS_H
#define  __USDK130_AOTG_REGS_H

//CMU register address
//#define CMU_DEVCLKEN      		0xB0010000+0x00000080
//#define CMU_DEVRST				0xB0010000+0x00000084

//PMU register address
#define AOTG_PMU_CHG			0xB0000000 + 0x0008
#define AOTG_PMU_CHG_EN		(1 << 31)

#define AOTG_DMA_MAX_BYTE		0x7FFFF

/*PUBLIC PART*/
#define AOTG_BASE    				0xb0210000     /*The base address of AOTG  Module*/
#define AOTG_REG_OFFSET   		0x07ff
#define AOTG_USB_IRQ        			4

#define AOTG_REG(offset)            		(AOTG_BASE + (offset))

#define FIFO1DAT0    				AOTG_REG(0x084)
#define FIFO2DAT0    				AOTG_REG(0x088) 

#define SETUPDAT0       AOTG_REG(0x180) 
#define SETUPDAT1       AOTG_REG(0x181) 
#define SETUPDAT2       AOTG_REG(0x182) 
#define SETUPDAT3       AOTG_REG(0x183) 
#define SETUPDAT4       AOTG_REG(0x184) 
#define SETUPDAT5       AOTG_REG(0x185) 
#define SETUPDAT6       AOTG_REG(0x186) 
#define SETUPDAT7       AOTG_REG(0x187) 

#define USBIRQ              AOTG_REG(0x18c)
#define USBIEN              AOTG_REG(0x198)

#define IVECT             	AOTG_REG(0x1a0)
#define FIFOVECT          AOTG_REG(0x1a1)
 
#define ENDPRST           AOTG_REG(0x1a2) 
#define USBCS                 AOTG_REG(0x1a3)
#define FRMNRL            AOTG_REG(0x1a4)
#define FRMNFH             AOTG_REG(0x1a5) 
#define FNADDR             AOTG_REG(0x1a6) 
#define CLKGATE           AOTG_REG(0x1a7) 
#define FIFOCTRL          AOTG_REG(0x1a8) 

#define OTGIRQ            	AOTG_REG(0x1bc)
#define OTGSTATE          AOTG_REG(0x1bd)
#define OTGCTRL            AOTG_REG(0x1be) 
#define OTGSTATUS        AOTG_REG(0x1bf) 
#define OTGIEN            	AOTG_REG(0x1c0)

#define TAAIDLBDIS        AOTG_REG(0x1c1) 
#define TAWAITBCON        AOTG_REG(0x1c2) 
#define TBVBUSPLS         AOTG_REG(0x1c3) 
#define TBVBUSDISPLS    AOTG_REG(0x1c7)

#define VDCTRL						AOTG_REG(0x408)
#define VDSTAT						AOTG_REG(0x409)

#define OUT1STARTADDRESS   AOTG_REG(0x304) 
#define OUT2STARTADDRESS   AOTG_REG(0x308) 
#define OUT3STARTADDRESS   AOTG_REG(0x30C) 
#define OUT4STARTADDRESS   AOTG_REG(0x310) 

#define IN1STARTADDRESS    AOTG_REG(0x344)
#define IN2STARTADDRESS    AOTG_REG(0x348)
#define IN3STARTADDRESS    AOTG_REG(0x34C)
#define IN4STARTADDRESS    AOTG_REG(0x350)

#define USBEIRQ		AOTG_REG(0x400)
#define USBESTA		AOTG_REG(0x401)
#define USBEXTCTRL	AOTG_REG(0X403)
#define USBERES		AOTG_REG(0x404)
#define TSTCTRL		AOTG_REG(0x40a)
#define DMAEPSEL		AOTG_REG(0x40c)
#define VBUSDBCTIMER AOTG_REG(0X40e)	


/**********************************************************************************************/
/**********************************************************************************************/

/*mask for EP0CS*/
#      define       		EP0CS_HCSETTOOGLE  (1<<6)
#      define       		EP0CS_HCCLRTOOGLE  (1<<5)
#      define 			EP0CS_HCSET     (1<<4)
#	define			EP0CS_HCINBSY		(1<<3)
#	define			EP0CS_HCOUTBSY	(1<<2)
#	define			EP0CS_OUTBSY	(1<<3)
#	define			EP0CS_INBSY	(1<<2)
#	define			EP0CS_HSNAK	(1<<1)
#	define			EP0CS_STALL	(1<<0)

/*mask for EPXCON(Host & Device)*/

#	define	EPCON_VAL	        	(1<<7)
#	define	EPCON_STALL			(1<<6)
#	define	EPCON_TYPE_INT		0x0c
#	define	EPCON_TYPE_BULK	0x08
#	define	EPCON_TYPE_ISO		0x04

#	define	EPCON_BUF_QUAD	0x03
#	define	EPCON_BUF_TRIPLE	0x02
#	define	EPCON_BUF_DOUBLE	0x01
#	define	EPCON_BUF_SINGLE	0x00


/*mask for EPXCS(Host & Device)*/

#	define	EPCS_AUTO_IN		(1<<4)
#	define	EPCS_AUTO_OUT		(1<<4)
#	define	EPCS_BUSY			(1<<1)
#	define	EPCS_ERR				(1<<0)

/*mask for OTG clock*/
#define     USBOTG_EN				(1<<13)
#define  	USBPHY_EN				(1<<27)
/*mask for OTG2 clock*/
#define     USBOTG_EN2				(1<<1)
#define  	USBPHY_EN2				(1<<2)

/*mask for CMU DEVRESET USB*/
#define 	BIT_DEVRST_USB			(1<<9)

/*mask for USBIRQ*/
#	define	USBIRQ_HS	        	(1<<5)
#	define	USBIRQ_URES			(1<<4)
#	define	USBIRQ_SUSP			(1<<3)
#	define	USBIRQ_SUTOK		(1<<2)
#	define	USBIRQ_SOF			(1<<1)
#	define	USBIRQ_SUDAV		(1<<0)

/*mask for USBIEN*/
#	define	USBIEN_HS	        (1<<5)
#	define	USBIEN_URES	(1<<4)
#	define	USBIEN_SUSP	(1<<3)
#	define	USBIEN_SUTOK	(1<<2)
#	define	USBIEN_SOF	(1<<1)
#	define	USBIEN_SUDAV	(1<<0)

/*mask for USBIVECT*/
#	define	UIV_SUDAV           	0x00
#	define 	UIV_SOF            	 	0x04
#	define 	UIV_SUTOK           	0x08
#	define 	UIV_SUSPEND        	0x0c
#	define 	UIV_USBRESET        0x10
#	define 	UIV_HSPEED          	0x14
#	define	UIV_HCOUT0ERR	0x16
#	define 	UIV_EP0IN           	0x18
#	define 	UIV_HCIN0ERR		0x1a
#	define 	UIV_EP0OUT          	0x1c
#	define 	UIV_EP0PING         	0x20
#	define	UIV_EP1IN          	0x24
#	define 	UIV_EP1OUT          	0x28
#	define 	UIV_EP1PING         	0x2c
#	define 	UIV_EP2IN           	0x30
#	define 	UIV_EP2OUT          	0x34
#	define 	UIV_EP2PING         	0x38
#	define 	UIV_EP3IN           	0x3c
#	define 	UIV_EP3OUT          	0x40
#	define 	UIV_EP3PING         	0x44
#	define 	UIV_EP4IN           	0x48
#	define 	UIV_EP4OUT          	0x4c
#	define 	UIV_EP4PING         	0x50
/*bq:1213 add endpoint5/6*/
#define 	UIV_EP5IN           	0x54
#define 	UIV_EP5OUT          	0x58
#define 	UIV_EP5PING         	0x5c

#define 	UIV_EP6IN           	0x60
#define 	UIV_EP6OUT          	0x64
#define 	UIV_EP6PING         	0x68
#define 	UIV_OTGIRQ          	0xd8


#define 	UIV_HCOUT1ERR	0x22
#define 	UIV_HCOUT2ERR	0x2e
#define 	UIV_HCOUT3ERR	0x3a
#define 	UIV_HCOUT4ERR	0x46
//bq: add 
#define 	UIV_HCOUT5ERR	0x52  
#define 	UIV_HCOUT6ERR	0x5e

#define 	UIV_HCIN1ERR	0x26
#define	UIV_HCIN2ERR	0x32
#define	UIV_HCIN3ERR	0x3e
#define	UIV_HCIN4ERR	0x4a//kewen
#define	UIV_HCIN5ERR	0x56
#define	UIV_HCIN6ERR	0x62

/*mask for USBCS*/
#	define	USBCS_WAKESRC	        (1<<7)
#	define	USBCS_DISCONN		(1<<6)
#	define	USBCS_SIGRSUME		(1<<5)
#      define     USBCS_LSMODE              (1<<0)

/*mask for USBCLKGATE*/
#	define	CLKGATE_WUVUBSEN	(1<<2)
#	define	CLKGATE_WUDPEN		(1<<1)
#	define	CLKGATE_WUIDEN		(1<<0)

/*mask for FIFOCTRL*/
#	define	FIFOCTRL_FIFOAUTO	(1<<5)
#	define	FIFOCTRL_IO			(1<<4)

/*mask for OTGIRQ*/
#	define	OTGIRQ_PERIPH	        (1<<4)
#	define	OTGIRQ_VBUSEER		(1<<3)
#	define	OTGIRQ_LOCSOF	        (1<<2)
#	define	OTGIRQ_SRPDET		(1<<1)
#	define	OTGIRQ_IDLE			(1<<0)

/*mask for OTGSTATE*/
#	define	A_IDLE				0x00
#	define	A_WAIT_VRISE		0x01
#	define	A_WAIT_BCON		0x02
#	define	A_HOST				0x03
#	define	A_SUSPEND			0x04
#	define	A_PHERIPHERAL	0x05
#	define	A_VBUS_ERR		0x06
#	define	A_WAIT_VFAL		0x07
#	define	B_IDLE				0x08
#	define	B_PHERIPHERAL	0x09
#	define	B_WAIT_ACON		0x0a
#	define	B_HOST				0x0b
#	define	B_SRP_INIT1		0x0c
#	define	B_SRP_INIT2		0x0d
#	define	B_DISCHRG1		0x0e
#	define	B_DISCHRG2		0x0f

/*mask for OTGCTRL*/
#	define	OTGCTRL_FORCEBCONN	        	(1<<7)
#	define	OTGCTRL_SRPDATDETEN 		(1<<5)
#	define	OTGCTRL_SRPVBUSDETEN 		(1<<4)
#	define	OTGCTRL_BHNPEN				(1<<3)
#	define	OTGCTRL_ASETBHNPEN			(1<<2)
#	define	OTGCTRL_ABUSDROP				(1<<1)
#	define	OTGCTRL_BUSREQ				(1<<0)


/*mask for OTGSTATUS*/
#	define	OTGSTATUS_ID	        			(1<<6)
#	define	OTGSTATUS_AVBUSSVAL 			(1<<5)
#	define	OTGSTATUS_BSESSEND			(1<<4)
#	define	OTGSTATUS_ASESSVAL			(1<<3)
#	define	OTGSTATUS_BSESSVAL			(1<<2)
#	define	OTGSTATUS_CONN				(1<<1)
#	define	OTGSTATUS_BSE0SRP				(1<<0)

/*mask for OTGIEN*/
#	define	OTGIEN_PERIPH	        (1<<4)
#	define	OTGIEN_VBUSEER		(1<<3)
#	define	OTGIEN_LOCSOF	        (1<<2)
#	define	OTGIEN_SRPDET		(1<<1)
#	define	OTGIEN_IDLE			(1<<0)


/*mask for USBEIRQ*/
#	define	USBEIRQ_WUIRQEN	       		(1<<7)
#	define	USBEIRQ_USBIRQEN			(1<<6)
#	define	USBEIRQ_WUIRQ				(1<<5)
#	define	USBEIRQ_USBIRQ				(1<<4)
#	define	USBEIRQ_WUDPEN			(1<<2)
#	define	USBEIRQ_WUIDEN				(1<<1)
#	define	USBEIRQ_WUVBUSEN			(1<<0)


/*mask for USBESTA*/
#	define	USBESTA_ID	       				(1<<7)
#	define	USBESTA_VBUSVAL			(1<<6)
#	define	USBESTA_LINE1				(1<<5)
#	define	USBESTA_LINE0				(1<<4)
#	define	USBESTA_EWKIRQ				(1<<3)
#	define	USBESTA_WKDP				(1<<2)
#	define	USBESTA_WKID				(1<<1)
#	define	USBESTA_WKVBUS			(1<<0)



/*mask for USBERES*/
#	define	USBERES_USBREST		(1<<0)

/*mask for ENDPRST*/
#	define	ENDPRST_TOGSETQ	        (1<<7)
#	define	ENDPRST_FIFOREST	   	(1<<6)
#	define	ENDPRST_TOGRST		(1<<5)
#	define	ENDPRST_IO			(1<<4)

/*mask for EPXINIEN*/
#	define	EP1_IN_IEN	(1<<1)
#	define	EP0_IN_IEN	(1<<0)

/*mask for EPXOUTIEN*/
#	define	EP1_OUT_IEN	(1<<1)
#	define	EP0_OUT_IEN	(1<<0)


/*mask for EPXINIRQ*/
#define	EP6_IN_IRQ	(1<<6)
#define	EP5_IN_IRQ	(1<<5)
#	define	EP4_IN_IRQ	(1<<4)
#	define	EP3_IN_IRQ	(1<<3)
#	define	EP2_IN_IRQ	(1<<2)
#	define	EP1_IN_IRQ	(1<<1)
#	define	EP0_IN_IRQ	(1<<0)

/*mask for EPXOUTIEN*/
#define	EP6_OUT_IRQ	(1<<6)
#define	EP5_OUT_IRQ	(1<<5)
#	define	EP4_OUT_IRQ	(1<<4)
#	define	EP3_OUT_IRQ	(1<<3)
#	define	EP2_OUT_IRQ	(1<<2)
#	define	EP1_OUT_IRQ	(1<<1)
#	define	EP0_OUT_IRQ	(1<<0)

/*for VBUSDBCTIMER*/
#	define 	L_DBVAL	0xffff
#	define	S_DBVAL	0x2000

/*mask for exctrl*/
#	define 	BIT_FORCE_EN		(1<<0)
#	define	BIT_FORCE_ID			(1<<1)

#	define	EP4_PNGIRQ	(1<<4)
#	define	EP3_PNGIRQ	(1<<3)
#	define	EP2_PNGIRQ	(1<<2)
#	define	EP1_PNGIRQ	(1<<1)
#	define	EP0_PNGIRQ	(1<<0)
/*********************************************************************************/
/*                    Registers relating to USB PERIPHERAL                       */
/*********************************************************************************/
/*EP0*/
#define OUT0BC          AOTG_REG(0x000) 
#define IN0BC           AOTG_REG(0x001) 
#define EP0CS           AOTG_REG(0x002) 

/*EP1 OUT*/
#define OUT1BCL    AOTG_REG(0x008)
#define OUT1BCH    AOTG_REG(0x009)
#define OUT1CON    AOTG_REG(0x00a) 
#define OUT1CS        AOTG_REG(0x00b) 

/*EP1 IN*/
#define IN1BCL         AOTG_REG(0x00c) 
#define IN1BCH        AOTG_REG(0x00d) 
#define IN1CON        AOTG_REG(0x00e) 
#define IN1CS            AOTG_REG(0x00f)

#if 0 
/*EP2 IN*/  //use
#define IN2BCL         AOTG_REG(0x014) 
#define IN2BCH        AOTG_REG(0x015) 
#define IN2CON        AOTG_REG(0x016) 
#define IN2CS            AOTG_REG(0x017)
#endif


/*EP2 IN  */
#define IN2BCL         AOTG_REG(0x014) 
#define IN2BCH        AOTG_REG(0x015) 
#define IN2CON        AOTG_REG(0x016) 
#define IN2CS            AOTG_REG(0x017)




/*EP0 DATA FIFO*/
#define EP0INDAT0        AOTG_REG(0x100) 
#define EP0INDAT1        AOTG_REG(0x101) 
#define EP0INDAT2        AOTG_REG(0x102) 
#define EP0INDAT3        AOTG_REG(0x103) 
#define EP0INDAT4        AOTG_REG(0x104) 
#define EP0INDAT5        AOTG_REG(0x105) 
#define EP0INDAT6        AOTG_REG(0x106) 
#define EP0INDAT7        AOTG_REG(0x107) 
#define EP0INDAT8        AOTG_REG(0x108) 
#define EP0INDAT9        AOTG_REG(0x109) 
#define EP0INDAT10      AOTG_REG(0x10a) 
#define EP0INDAT11      AOTG_REG(0x10b) 
#define EP0INDAT12      AOTG_REG(0x10c) 
#define EP0INDAT13      AOTG_REG(0x10d) 
#define EP0INDAT14      AOTG_REG(0x10e) 
#define EP0INDAT15      AOTG_REG(0x10f) 
#define EP0INDAT16      AOTG_REG(0x110) 
#define EP0INDAT17      AOTG_REG(0x111) 
#define EP0INDAT18      AOTG_REG(0x112) 
#define EP0INDAT19      AOTG_REG(0x113) 
#define EP0INDAT20      AOTG_REG(0x114) 
#define EP0INDAT21      AOTG_REG(0x115) 
#define EP0INDAT22      AOTG_REG(0x116) 
#define EP0INDAT23      AOTG_REG(0x117) 
#define EP0INDAT24      AOTG_REG(0x118) 
#define EP0INDAT25      AOTG_REG(0x119) 
#define EP0INDAT26      AOTG_REG(0x11a) 
#define EP0INDAT27      AOTG_REG(0x11b) 
#define EP0INDAT28      AOTG_REG(0x11c) 
#define EP0INDAT29      AOTG_REG(0x11d) 
#define EP0INDAT30      AOTG_REG(0x11e) 
#define EP0INDAT31      AOTG_REG(0x11f) 
#define EP0INDAT32      AOTG_REG(0x120) 
#define EP0INDAT33      AOTG_REG(0x121) 
#define EP0INDAT34      AOTG_REG(0x122) 
#define EP0INDAT35      AOTG_REG(0x123) 
#define EP0INDAT36      AOTG_REG(0x124) 
#define EP0INDAT37      AOTG_REG(0x125) 
#define EP0INDAT38      AOTG_REG(0x126) 
#define EP0INDAT39      AOTG_REG(0x127) 
#define EP0INDAT40      AOTG_REG(0x128) 
#define EP0INDAT41      AOTG_REG(0x129) 
#define EP0INDAT42      AOTG_REG(0x12a) 
#define EP0INDAT43      AOTG_REG(0x12b) 
#define EP0INDAT44      AOTG_REG(0x12c) 
#define EP0INDAT45      AOTG_REG(0x12d) 
#define EP0INDAT46      AOTG_REG(0x12e) 
#define EP0INDAT47      AOTG_REG(0x12f) 
#define EP0INDAT48      AOTG_REG(0x130) 
#define EP0INDAT49      AOTG_REG(0x131) 
#define EP0INDAT50      AOTG_REG(0x132) 
#define EP0INDAT51      AOTG_REG(0x133) 
#define EP0INDAT52      AOTG_REG(0x134) 
#define EP0INDAT53      AOTG_REG(0x135) 
#define EP0INDAT54      AOTG_REG(0x136) 
#define EP0INDAT55      AOTG_REG(0x137) 
#define EP0INDAT56      AOTG_REG(0x138) 
#define EP0INDAT57      AOTG_REG(0x139) 
#define EP0INDAT58      AOTG_REG(0x13a) 
#define EP0INDAT59      AOTG_REG(0x13b)
#define EP0INDAT60      AOTG_REG(0x13c) 
#define EP0INDAT61      AOTG_REG(0x13d) 
#define EP0INDAT62      AOTG_REG(0x13e) 
#define EP0INDAT63      AOTG_REG(0x13f) 

#define EP0OUTDAT0        AOTG_REG(0x140) 
#define EP0OUTDAT1        AOTG_REG(0x141) 
#define EP0OUTDAT2        AOTG_REG(0x142) 
#define EP0OUTDAT3        AOTG_REG(0x143) 
#define EP0OUTDAT4        AOTG_REG(0x144) 
#define EP0OUTDAT5        AOTG_REG(0x145) 
#define EP0OUTDAT6        AOTG_REG(0x146) 
#define EP0OUTDAT7        AOTG_REG(0x147) 
#define EP0OUTDAT8        AOTG_REG(0x148) 
#define EP0OUTDAT9        AOTG_REG(0x149) 
#define EP0OUTDAT10      AOTG_REG(0x14a) 
#define EP0OUTDAT11      AOTG_REG(0x14b) 
#define EP0OUTDAT12      AOTG_REG(0x14c) 
#define EP0OUTDAT13      AOTG_REG(0x14d) 
#define EP0OUTDAT14      AOTG_REG(0x14e) 
#define EP0OUTDAT15      AOTG_REG(0x14f) 
#define EP0OUTDAT16      AOTG_REG(0x150) 
#define EP0OUTDAT17      AOTG_REG(0x151) 
#define EP0OUTDAT18      AOTG_REG(0x152) 
#define EP0OUTDAT19      AOTG_REG(0x153) 
#define EP0OUTDAT20      AOTG_REG(0x154) 
#define EP0OUTDAT21      AOTG_REG(0x155) 
#define EP0OUTDAT22      AOTG_REG(0x156) 
#define EP0OUTDAT23      AOTG_REG(0x157) 
#define EP0OUTDAT24      AOTG_REG(0x158) 
#define EP0OUTDAT25      AOTG_REG(0x159) 
#define EP0OUTDAT26      AOTG_REG(0x15a) 
#define EP0OUTDAT27      AOTG_REG(0x15b) 
#define EP0OUTDAT28      AOTG_REG(0x15c) 
#define EP0OUTDAT29      AOTG_REG(0x15d) 
#define EP0OUTDAT30      AOTG_REG(0x15e) 
#define EP0OUTDAT31      AOTG_REG(0x15f) 
#define EP0OUTDAT32      AOTG_REG(0x160) 
#define EP0OUTDAT33      AOTG_REG(0x161) 
#define EP0OUTDAT34      AOTG_REG(0x162) 
#define EP0OUTDAT35      AOTG_REG(0x163) 
#define EP0OUTDAT36      AOTG_REG(0x164) 
#define EP0OUTDAT37      AOTG_REG(0x165) 
#define EP0OUTDAT38      AOTG_REG(0x166) 
#define EP0OUTDAT39      AOTG_REG(0x167) 
#define EP0OUTDAT40      AOTG_REG(0x168) 
#define EP0OUTDAT41      AOTG_REG(0x169) 
#define EP0OUTDAT42      AOTG_REG(0x16a) 
#define EP0OUTDAT43      AOTG_REG(0x16b) 
#define EP0OUTDAT44      AOTG_REG(0x16c) 
#define EP0OUTDAT45      AOTG_REG(0x16d) 
#define EP0OUTDAT46      AOTG_REG(0x16e) 
#define EP0OUTDAT47      AOTG_REG(0x16f) 
#define EP0OUTDAT48      AOTG_REG(0x170) 
#define EP0OUTDAT49      AOTG_REG(0x171) 
#define EP0OUTDAT50      AOTG_REG(0x172) 
#define EP0OUTDAT51      AOTG_REG(0x173) 
#define EP0OUTDAT52      AOTG_REG(0x174) 
#define EP0OUTDAT53      AOTG_REG(0x175) 
#define EP0OUTDAT54      AOTG_REG(0x176) 
#define EP0OUTDAT55      AOTG_REG(0x177) 
#define EP0OUTDAT56      AOTG_REG(0x178) 
#define EP0OUTDAT57      AOTG_REG(0x179) 
#define EP0OUTDAT58      AOTG_REG(0x17a) 
#define EP0OUTDAT59      AOTG_REG(0x17b)
#define EP0OUTDAT60      AOTG_REG(0x17c) 
#define EP0OUTDAT61      AOTG_REG(0x17d) 
#define EP0OUTDAT62      AOTG_REG(0x17e) 
#define EP0OUTDAT63      AOTG_REG(0x17f) 

/*EP0 IRQ*/
#define IN07IRQ             AOTG_REG(0x188)     
#define OUT07IRQ         AOTG_REG(0x18a)    


#define OUT04PNGIRQ     AOTG_REG(0x18e)
#define OUT04PNGIEN      AOTG_REG(0x19a) 
/*EP0~7 IEN*/ 
#define IN07IEN           AOTG_REG(0x194)
#define OUT07IEN        AOTG_REG(0x196)
#define OUT07PNGIEN       AOTG_REG(0x19a) 

/*EP maxpacket*/
#define OUT0MAXPCK      AOTG_REG(0x1e0) 
#define OUT1MAXPCKL     AOTG_REG(0x1e2) 
#define OUT1MAXPCKH      AOTG_REG(0x1e3) 
#define IN0MAXPCK      AOTG_REG(0x3e0)
#define IN1MAXPCKL   AOTG_REG(0x3e2)
#define IN1MAXPCKH   AOTG_REG(0x3e3)

#define IN2MAXPCKL   AOTG_REG(0x3e4)
#define IN2MAXPCKH   AOTG_REG(0x3e5)

/************************************************************************************/
/*                    Registers relating to USB HOST                                */
/************************************************************************************/

/*HCEP0*/
#define HCIN0BC          AOTG_REG(0x000) 
#define HCOUT0BC      AOTG_REG(0x001) 
#define HCEP0CS          AOTG_REG(0x002) 

/*HCEP CTRL*/
#define HCEP0CTRL    AOTG_REG(0x0c0)
#define HCOUT1CTRL AOTG_REG(0x0c4)
#define HCIN1CTRL    AOTG_REG(0x0c6)

/*HC IN/OUT ERR*/
#define HCIN0ERR    	AOTG_REG(0x0c3)
#define HCIN1ERR    	AOTG_REG(0x0c7)
#define HCIN2ERR    	AOTG_REG(0x0cb)
#define HCIN3ERR    	AOTG_REG(0x0cf)
#define HCIN4ERR    	AOTG_REG(0x0d3)
#define HCIN5ERR    	AOTG_REG(0x0d7)
#define HCIN6ERR    	AOTG_REG(0x0db)

#define HCOUT0ERR    	AOTG_REG(0x0c1)
#define HCOUT1ERR    	AOTG_REG(0x0c5)
#define HCOUT2ERR    	AOTG_REG(0x0c9)
#define HCOUT3ERR    	AOTG_REG(0x0cd)
#define HCOUT4ERR    	AOTG_REG(0x0d3)
#define HCOUT5ERR    	AOTG_REG(0x0d5)
#define HCOUT6ERR    	AOTG_REG(0x0d9)

/*HCEP1 IN*/
#define HCIN1BCL    		AOTG_REG(0x008)
#define HCIN1BCH    	AOTG_REG(0x009) 
#define HCIN1CON    	AOTG_REG(0x00a) 
#define HCIN1CS     		AOTG_REG(0x00b) 

/*HCEP1 OUT*/
#define HCOUT1BCL		AOTG_REG(0x00c) 
#define HCOUT1BCH       AOTG_REG(0x00d) 
#define HCOUT1CON	AOTG_REG(0x00e) 
#define HCOUT1CS         	AOTG_REG(0x00f)

/*HCEP2 IN*/
#define HCIN2BCL    		AOTG_REG(0x010)
#define HCIN2BCH    	AOTG_REG(0x011) 
#define HCIN2CON    	AOTG_REG(0x012) 
#define HCIN2CS     		AOTG_REG(0x013) 

/*HCEP2 OUT*/
#define HCOUT2BCL		AOTG_REG(0x014) 
#define HCOUT2BCH       	AOTG_REG(0x015) 
#define HCOUT2CON	AOTG_REG(0x016) 
#define HCOUT2CS         	AOTG_REG(0x017)

/*HCEP3	IN*/
#define HCIN3BCL    		AOTG_REG(0x018)
#define HCIN3BCH    	AOTG_REG(0x019) 
#define HCIN3CON    	AOTG_REG(0x01a) 
#define HCIN3CS     		AOTG_REG(0x01b) 

/*HCEP3 OUT*/
#define HCOUT3BCL    	AOTG_REG(0x01c)
#define HCOUT3BCH    	AOTG_REG(0x01d) 
#define HCOUT3CON    	AOTG_REG(0x01e) 
#define HCOUT3CS     	AOTG_REG(0x01f) 

/*HCEP4	IN*/
#define HCIN4BCL    		AOTG_REG(0x020)
#define HCIN4BCH    	AOTG_REG(0x021) 
#define HCIN4CON    	AOTG_REG(0x022) 
#define HCIN4CS     		AOTG_REG(0x023) 

/*HCEP4 OUT*/
#define HCOUT4BCL    	AOTG_REG(0x024)
#define HCOUT4BCH    	AOTG_REG(0x025) 
#define HCOUT4CON    	AOTG_REG(0x026) 
#define HCOUT4CS     	AOTG_REG(0x027) 

/**1213 add***/
/*HCEP5	IN*/
#define HCIN5BCL    		AOTG_REG(0x028)
#define HCIN5BCH    	AOTG_REG(0x029) 
#define HCIN5CON    	AOTG_REG(0x02a) 
#define HCIN5CS     		AOTG_REG(0x02b) 

/*HCEP5 OUT*/
#define HCOUT5BCL    	AOTG_REG(0x02c)
#define HCOUT5BCH    	AOTG_REG(0x02d) 
#define HCOUT5CON    	AOTG_REG(0x02e) 
#define HCOUT5CS     	AOTG_REG(0x02f) 

/*HCEP6	IN*/
#define HCIN6BCL    		AOTG_REG(0x030)
#define HCIN6BCH    	AOTG_REG(0x031) 
#define HCIN6CON    	AOTG_REG(0x032) 
#define HCIN6CS     		AOTG_REG(0x033) 

/*HCEP6 OUT*/
#define HCOUT6BCL    	AOTG_REG(0x034)
#define HCOUT6BCH    	AOTG_REG(0x035) 
#define HCOUT6CON    	AOTG_REG(0x036) 
#define HCOUT6CS     	AOTG_REG(0x037) 
/*HC Frame Remain Register Low/high*/
#define HCFRMREMAINL       AOTG_REG(0x1ae) 
#define HCFRMREMAINH       AOTG_REG(0x1af) 


/*HCEP0 DATA FIFO*/
#define HCEP0OUTDAT0        AOTG_REG(0x100) 
#define HCEP0OUTDAT1        AOTG_REG(0x101) 
#define HCEP0OUTDAT2        AOTG_REG(0x102) 
#define HCEP0OUTDAT3        AOTG_REG(0x103) 
#define HCEP0OUTDAT4        AOTG_REG(0x104) 
#define HCEP0OUTDAT5        AOTG_REG(0x105) 
#define HCEP0OUTDAT6        AOTG_REG(0x106) 
#define HCEP0OUTDAT7        AOTG_REG(0x107) 
#define HCEP0OUTDAT8        AOTG_REG(0x108) 
#define HCEP0OUTDAT9        AOTG_REG(0x109) 
#define HCEP0OUTDAT10      AOTG_REG(0x10a) 
#define HCEP0OUTDAT11      AOTG_REG(0x10b) 
#define HCEP0OUTDAT12      AOTG_REG(0x10c) 
#define HCEP0OUTDAT13      AOTG_REG(0x10d) 
#define HCEP0OUTDAT14      AOTG_REG(0x10e) 
#define HCEP0OUTDAT15      AOTG_REG(0x10f) 
#define HCEP0OUTDAT16      AOTG_REG(0x110) 
#define HCEP0OUTDAT17      AOTG_REG(0x111) 
#define HCEP0OUTDAT18      AOTG_REG(0x112) 
#define HCEP0OUTDAT19      AOTG_REG(0x113) 
#define HCEP0OUTDAT20      AOTG_REG(0x114) 
#define HCEP0OUTDAT21      AOTG_REG(0x115) 
#define HCEP0OUTDAT22      AOTG_REG(0x116) 
#define HCEP0OUTDAT23      AOTG_REG(0x117) 
#define HCEP0OUTDAT24      AOTG_REG(0x118) 
#define HCEP0OUTDAT25      AOTG_REG(0x119) 
#define HCEP0OUTDAT26      AOTG_REG(0x11a) 
#define HCEP0OUTDAT27      AOTG_REG(0x11b) 
#define HCEP0OUTDAT28      AOTG_REG(0x11c) 
#define HCEP0OUTDAT29      AOTG_REG(0x11d) 
#define HCEP0OUTDAT30      AOTG_REG(0x11e) 
#define HCEP0OUTDAT31      AOTG_REG(0x11f) 
#define HCEP0OUTDAT32      AOTG_REG(0x120) 
#define HCEP0OUTDAT33      AOTG_REG(0x121) 
#define HCEP0OUTDAT34      AOTG_REG(0x122) 
#define HCEP0OUTDAT35      AOTG_REG(0x123) 
#define HCEP0OUTDAT36      AOTG_REG(0x124) 
#define HCEP0OUTDAT37      AOTG_REG(0x125) 
#define HCEP0OUTDAT38      AOTG_REG(0x126) 
#define HCEP0OUTDAT39      AOTG_REG(0x127) 
#define HCEP0OUTDAT40      AOTG_REG(0x128) 
#define HCEP0OUTDAT41      AOTG_REG(0x129) 
#define HCEP0OUTDAT42      AOTG_REG(0x12a) 
#define HCEP0OUTDAT43      AOTG_REG(0x12b) 
#define HCEP0OUTDAT44      AOTG_REG(0x12c) 
#define HCEP0OUTDAT45      AOTG_REG(0x12d) 
#define HCEP0OUTDAT46      AOTG_REG(0x12e) 
#define HCEP0OUTDAT47      AOTG_REG(0x12f) 
#define HCEP0OUTDAT48      AOTG_REG(0x130) 
#define HCEP0OUTDAT49      AOTG_REG(0x131) 
#define HCEP0OUTDAT50      AOTG_REG(0x132) 
#define HCEP0OUTDAT51      AOTG_REG(0x133) 
#define HCEP0OUTDAT52      AOTG_REG(0x134) 
#define HCEP0OUTDAT53      AOTG_REG(0x135) 
#define HCEP0OUTDAT54      AOTG_REG(0x136) 
#define HCEP0OUTDAT55      AOTG_REG(0x137) 
#define HCEP0OUTDAT56      AOTG_REG(0x138) 
#define HCEP0OUTDAT57      AOTG_REG(0x139) 
#define HCEP0OUTDAT58      AOTG_REG(0x13a) 
#define HCEP0OUTDAT59      AOTG_REG(0x13b)
#define HCEP0OUTDAT60      AOTG_REG(0x13c) 
#define HCEP0OUTDAT61      AOTG_REG(0x13d) 
#define HCEP0OUTDAT62      AOTG_REG(0x13e) 
#define HCEP0OUTDAT63      AOTG_REG(0x13f) 


#define HCEP0INDAT0        AOTG_REG(0x140) 
#define HCEP0INDAT1        AOTG_REG(0x141) 
#define HCEP0INDAT2        AOTG_REG(0x142) 
#define HCEP0INDAT3        AOTG_REG(0x143) 
#define HCEP0INDAT4        AOTG_REG(0x144) 
#define HCEP0INDAT5        AOTG_REG(0x145) 
#define HCEP0INDAT6        AOTG_REG(0x146) 
#define HCEP0INDAT7        AOTG_REG(0x147) 
#define HCEP0INDAT8        AOTG_REG(0x148) 
#define HCEP0INDAT9        AOTG_REG(0x149) 
#define HCEP0INDAT10      AOTG_REG(0x14a) 
#define HCEP0INDAT11      AOTG_REG(0x14b) 
#define HCEP0INDAT12      AOTG_REG(0x14c) 
#define HCEP0INDAT13      AOTG_REG(0x14d) 
#define HCEP0INDAT14      AOTG_REG(0x14e) 
#define HCEP0INDAT15      AOTG_REG(0x14f) 
#define HCEP0INDAT16      AOTG_REG(0x150) 
#define HCEP0INDAT17      AOTG_REG(0x151) 
#define HCEP0INDAT18      AOTG_REG(0x152) 
#define HCEP0INDAT19      AOTG_REG(0x153) 
#define HCEP0INDAT20      AOTG_REG(0x154) 
#define HCEP0INDAT21      AOTG_REG(0x155) 
#define HCEP0INDAT22      AOTG_REG(0x156) 
#define HCEP0INDAT23      AOTG_REG(0x157) 
#define HCEP0INDAT24      AOTG_REG(0x158) 
#define HCEP0INDAT25      AOTG_REG(0x159) 
#define HCEP0INDAT26      AOTG_REG(0x15a) 
#define HCEP0INDAT27      AOTG_REG(0x15b) 
#define HCEP0INDAT28      AOTG_REG(0x15c) 
#define HCEP0INDAT29      AOTG_REG(0x15d) 
#define HCEP0INDAT30      AOTG_REG(0x15e) 
#define HCEP0INDAT31      AOTG_REG(0x15f) 
#define HCEP0INDAT32      AOTG_REG(0x160) 
#define HCEP0INDAT33      AOTG_REG(0x161) 
#define HCEP0INDAT34      AOTG_REG(0x162) 
#define HCEP0INDAT35      AOTG_REG(0x163) 
#define HCEP0INDAT36      AOTG_REG(0x164) 
#define HCEP0INDAT37      AOTG_REG(0x165) 
#define HCEP0INDAT38      AOTG_REG(0x166) 
#define HCEP0INDAT39      AOTG_REG(0x167) 
#define HCEP0INDAT40      AOTG_REG(0x168) 
#define HCEP0INDAT41      AOTG_REG(0x169) 
#define HCEP0INDAT42      AOTG_REG(0x16a) 
#define HCEP0INDAT43      AOTG_REG(0x16b) 
#define HCEP0INDAT44      AOTG_REG(0x16c) 
#define HCEP0INDAT45      AOTG_REG(0x16d) 
#define HCEP0INDAT46      AOTG_REG(0x16e) 
#define HCEP0INDAT47      AOTG_REG(0x16f) 
#define HCEP0INDAT48      AOTG_REG(0x170) 
#define HCEP0INDAT49      AOTG_REG(0x171) 
#define HCEP0INDAT50      AOTG_REG(0x172) 
#define HCEP0INDAT51      AOTG_REG(0x173) 
#define HCEP0INDAT52      AOTG_REG(0x174) 
#define HCEP0INDAT53      AOTG_REG(0x175) 
#define HCEP0INDAT54      AOTG_REG(0x176) 
#define HCEP0INDAT55      AOTG_REG(0x177) 
#define HCEP0INDAT56      AOTG_REG(0x178) 
#define HCEP0INDAT57      AOTG_REG(0x179) 
#define HCEP0INDAT58      AOTG_REG(0x17a) 
#define HCEP0INDAT59      AOTG_REG(0x17b)
#define HCEP0INDAT60      AOTG_REG(0x17c) 
#define HCEP0INDAT61      AOTG_REG(0x17d) 
#define HCEP0INDAT62      AOTG_REG(0x17e) 
#define HCEP0INDAT63      AOTG_REG(0x17f) 

/*HCEP0~4  IRQ*/
#define HCOUT04IRQ          AOTG_REG(0x188)     
#define HCIN04IRQ           AOTG_REG(0x18a)    



/*HCEP0~4 IEN*/ 
#define HCOUT04IEN           AOTG_REG(0x194)
#define HCIN04IEN            AOTG_REG(0x196)

/*HCEP0~4 ERR IRQ*/
#define HCOUT04ERRIRQ           AOTG_REG(0x1b6)
#define HCIN04ERRIRQ               AOTG_REG(0x1b4)

/*HCEP0~4 ERR IEN*/
#define HCOUT04ERRIEN           AOTG_REG(0x1bA)
#define HCIN04ERRIEN               AOTG_REG(0x1b8)

/*HCPORTCTRL*/
#define HCPORTCTRL                 AOTG_REG(0x1ab)

/*HCEP maxpacket*/
#define HCIN0MAXPCK       AOTG_REG(0x1e0) 
#define HCIN1MAXPCKL      AOTG_REG(0x1e2) 
#define HCIN1MAXPCKH      AOTG_REG(0x1e3) 
#define HCOUT0MAXPACK      AOTG_REG(0x3e0)
#define HCOUT1MAXPACKL   AOTG_REG(0x3e2)
#define HCOUT1MAXPACKH   AOTG_REG(0x3e3)

/*1203 add*/
#define HCIN1AUTOCTLUP		AOTG_REG(0x504)
#define HCIN2AUTOCTLUP		AOTG_REG(0x508)
#define HCIN3AUTOCTLUP		AOTG_REG(0x50C)
#define HCIN4AUTOCTLUP		AOTG_REG(0x510)

#define HCIN1AUTOCTLUSB		AOTG_REG(0x506)
#define HCIN2AUTOCTLUSB		AOTG_REG(0x50a)
#define HCIN3AUTOCTLUSB		AOTG_REG(0x50e)
#define HCIN4AUTOCTLUSB		AOTG_REG(0x512)

#define HCIN1STARTFRM		AOTG_REG(0x544)
#define HCIN2STARTFRM		AOTG_REG(0x548)
#define HCIN3STARTFRM		AOTG_REG(0x54C)
#define HCIN4STARTFRM		AOTG_REG(0x550)

#define HCIN1FRMINTERVAL	AOTG_REG(0x546)
#define HCIN2FRMINTERVAL	AOTG_REG(0x54a)
#define HCIN3FRMINTERVAL	AOTG_REG(0x54e)
#define HCIN4FRMINTERVAL	AOTG_REG(0x552)

#define HCIN1FRMPCKNB		AOTG_REG(0x584)
#define HCIN2FRMPCKNB		AOTG_REG(0x588)
#define HCIN3FRMPCKNB		AOTG_REG(0x58C)
#define HCIN4FRMPCKNB		AOTG_REG(0x590)

#define HCIN1SUMPCKNB		AOTG_REG(0x5c4)
#define HCIN2SUMPCKNB		AOTG_REG(0x5c8)
#define HCIN3SUMPCKNB		AOTG_REG(0x5cc)
#define HCIN4SUMPCKNB		AOTG_REG(0x5d0)

#define HCIN1SUMPCKRCNT		AOTG_REG(0x5c6)
#define HCIN2SUMPCKRCNT		AOTG_REG(0x5ca)
#define HCIN3SUMPCKRCNT		AOTG_REG(0x5ce)
#define HCIN4SUMPCKRCNT		AOTG_REG(0x5d2)
#endif  /* __USDK130_AOTG_REGS_H */
 

