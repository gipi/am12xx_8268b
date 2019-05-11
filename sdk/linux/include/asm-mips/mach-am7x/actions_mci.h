#ifndef __ACTIONS_MCI_H__
#define __ACTIONS_MCI_H__


#define SDIO_IRQ_EN (1<<2)



#define SD_MF2_MSK_8250    ~((1<<4)|(1<<7)|(1<<10)|(1<<13)|(1<<16)|(1<<19))
#define SD_MF2_VAL_8250     ((1<<4)|(1<<7)|(1<<10)|(1<<13)|(1<<16)|(1<<19))
#define SD_MF2_GPO_8250     ((8<<4)|(8<<7)|(8<<10)|(8<<13)|(8<<16)|(8<<19))

#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3





/* mmc definition */
//ERROR TYPE
#define SD_GOOD		0
#define SD_STALLED		-1
#define SD_NOMEM		-2
#define SD_FAILED		-3
#define SD_ERROR		-4

//SD_CTL REG
#define SD_CTL_WR		(1 << 27)
#define SD_CTL_RD		(0 << 27)
#define SD_CTL_CMD		(1 << 28)
#define SD_CTL_DATA		(0 << 28)
#define SD_CTL_STATUS   (1 << 31)


#define ClearStatus     ((1<<29)|(1<<30)|(1<<31))


#define SD_CTL_MAX_LEN	(0xFFFFFF)

//SD STATUS_REG
#define SD_CRC_END		    (1 << 0)
#define SD_CRC_READY		(1 << 1)
#define SD_TRN_ACTIVE		(1 << 3)
#define SD_TRN_END		(1 << 4)
#define SD_RX_FIFO_EMPTY	(1 << 6)

//SD IRQ EN/CLR REG
#define SD_TX_FULL_IRQ	(1 << 7)
#define SD_RX_FULL_IRQ	(1 << 6)
#define SD_CARE_ERR_IRQ	(1 << 5)
#define SD_TRANS_END_IRQ	(1 << 4)

//SD DRQ REG
#define SD_DRQ_R_DEMAND	(1 << 0)	//Demand mode doesn't refer to the burst length. It just use 1.
#define SD_DRQ_R_BLOCK	(0 << 0)
#define SD_DRQ_W_DEMAND	(1 << 4)	//Demand mode doesn't refer to the burst length. It just use 1.
#define SD_DRQ_W_BLOCK	(0 << 4)
#define SD_DRQ_R_TL_4		(3 << 1)
#define SD_DRQ_W_TL_4		(3 << 5)
#define SD_DRQ_R_TL_8		(7 << 1)
#define SD_DRQ_W_TL_8		(7 << 5)
#define SD_DRQ_4_MOD		SD_DRQ_R_BLOCK | SD_DRQ_R_TL_4 | SD_DRQ_W_BLOCK | SD_DRQ_W_TL_4
#define SD_DRQ_8_MOD		SD_DRQ_R_BLOCK | SD_DRQ_R_TL_8 | SD_DRQ_W_BLOCK | SD_DRQ_W_TL_8
#define SD_DRQ_1_MOD		SD_DRQ_R_DEMAND | SD_DRQ_W_DEMAND

//SD CMD for actions controllr
#define SD_SEND_CMD		    0x0048
#define SD_SET_PARA		    0x0051
#define SDIO_ENABLE_INT		0x0053
#define SDIO_CLR_INT_REG    0x0054



#define SD_SWITCH_CTL_CLK	0x0080
#define SD_NORMALREAD		0x004C
#define SD_NORMALWRITE	0x0040
#define SD_AUTOREAD1		0x004D  //Read command + Data + Stop command
#define SD_AUTOREAD2		0x004E  //Read command + Data
#define SD_AUTOREAD3		0x0045  //Data
#define SD_AUTOREAD4		0x0046  //Data + Stop command
#define SD_AUTOWRITE1		0x0049  //Write command + Data + Stop command
#define SD_AUTOWRITE2		0x004A  //Write command + Data
#define SD_AUTOWRITE3		0x0041  //Data
#define SD_AUTOWRITE4		0x0042  //Data + Stop command
#define SD_REG_CHK		0x0052
#define SD_CARD_STOP		0x0483

//SD respnse type for actions controllr
#define SD_RSP_MODE_R0	0x04
#define SD_RSP_MODE_R1	0x05
#define SD_RSP_MODE_R1b	0x0D
#define SD_RSP_MODE_R2	0x06
#define SD_RSP_MODE_R3	0x01
#define SD_RSP_MODE_R4	0x01
#define SD_RSP_MODE_R5	0x05
#define SD_RSP_MODE_R6	0x05
#define SD_RSP_MASK		0x0F
#define DIVIDE_SCHEME0		0x10
#define DIVIDE_SCHEME1		0x20
#define CHECK_CRC16		0x40

//SD control clock
#define SD_CTL_CLOCK_20	0x0
#define SD_CTL_CLOCK_24	0x1
#define SD_CTL_CLOCK_30	0x2     //default vlaue
#define SD_CTL_CLOCK_40	0x3
#define SD_CTL_CLOCK_48	0x4
#define SD_CTL_CLOCK_60	0x5
#define SD_CTL_CLOCK_80	0x6
#define SD_CTL_CLOCK_96	0x7

//SD bus mode
#define SD_1BIT_MODE		(0 << 4)
#define SD_4BIT_MODE		(1 << 4)
#define SD_8BIT_MODE		(2 << 4)

//SD set para
#define WAIT_BUSY		(1 << 4)

//SD cmd start bit and transmission bit
#define SD_CMD_HDR		0x40
#define SD_DFT_WAIT_T		2

/*SD CMU definiation*/
#define SD_DEV_RST_BIT	(1 << 6)

/* SD definiation */
#define SD_120M_EN		(1 << 4)
#define SD_60M_EN		(1 << 2)
#define SD_80M_EN		(1 << 1)
#define SD_96M_EN		(1 << 0)
#define SD_HCLK_EN		(1 << 3)

//SD register define
#define SD_REG_BASE   	0x100c0000
#define SD_CTL_REG		0x00
#define SD_STATUS_REG		0x04
#define SD_DATA_REG		0x08
#define SD_IRQ_EN_REG		0x0C
#define SD_IRQ_CLR_REG	0x10
#define SD_DRQ_SET_REG	0x14
#define SD_XD_CD_REG		0x18
#define SD_SM_REG		0x1C
#define SD_CRC_MBIST_INT_REG	0x20

//SD Command
#define CMD0			0
#define CMD1			1
#define CMD2			2
#define CMD3			3
#define CMD5			5
#define CMD6			6
#define CMD7			7
#define CMD8			8

//DMA DEFINE
#define DMA_DST_BST_SINGLE      (0 << 29)
#define DMA_DST_BST_INCR4       (3 << 29)
#define DMA_DST_BST_INCR8       (5 << 29)
#define DMA_DST_FIX_ADDR        (1 << 24)
#define DMA_DST_INC_ADDR        (0 << 24)
#define DMA_DST_DRQ_BIT         (19)
#define DMA_SRC_BST_SINGLE      (0 << 13)
#define DMA_SRC_BST_INCR4       (3 << 13)
#define DMA_SRC_BST_INCR8       (5 << 13)
#define DMA_SRC_FIX_ADDR        (1 << 8)
#define DMA_SRC_INC_ADDR        (0 << 8)
#define DMA_SRC_DRQ_BIT         (3)

//DMA Trig-Soure
#define DMA_TRIG_SDRAM          16
#define DMA_TRIG_SD           25

#endif
