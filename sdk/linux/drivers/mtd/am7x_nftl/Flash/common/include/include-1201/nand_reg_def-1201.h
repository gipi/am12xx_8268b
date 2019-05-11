/*
********************************************************************************
*                       USDK213X
*
*   Nand Flash Base Module
*
*File:  nand_flash.h
*By:    mengzh
*Date:  2006-8-3 11:33
*ver:   0.1
********************************************************************************
*/
#ifndef __NAND_FLASH_REG__
#define __NAND_FLASH_REG__

#include "nand_flash_driver_type.h"
#include "syscfg.h"
#include "actions_regs.h"
#include "nand_regs.h"
#define DMA_IOMAP_BASE              (0xB0060000)
//=============================================================================

#define WATCH_DOG_CONTROL           (0x0014)    /* RTC_Watchdog control, base is 0xb0018000*/

//NAND_CMD: (NAND_CMD Offset: 0x24) Nand Flash COMMAND Register
#define NAND_CMD_NORMAL_MODE        (0<<24)
#define NAND_CMD_SPARE_MODE         (1<<24)
#define	NAND_CMD_SCOL_ADDR		(1<<23)	/*send column address ctl*/

#define NAND_CMD_SEQU               (1<<31)     /* Sequent CMD                                         */
#define NAND_CMD_DMAS               (1<<19)     /* DMA Sent Control. Writing 1 to this bit State Machine
                                                   sent the DMA. IF RBI=0, State Machine sent the DMA no
                                                   wait the RBI ready; if RBI=1, State Machine sent the
                                                   DMA must be wait the RBI ready.                     */
#define NAND_CMD_RBI                (1<<18)     /* RBI CTL. 0: State Machine sent DMA no wait RBI ready;
                                                   1: State Machine sent DMA wait RBI ready.           */
#define NAND_CMD_ROW_ADDRCYCLE          (3<<20)     /* bit 22:20, Address Cycle Selection                  */
#define NAND_CMD_ROW_ADDRCYCLE_0        (0<<20)	/*no send ROW address*/
#define NAND_CMD_ROW_ADDRCYCLE_1        (1<<20)	/*send 1 cycle ROW address*/
#define NAND_CMD_ROW_ADDRCYCLE_2        (2<<20)
#define NAND_CMD_ROW_ADDRCYCLE_3        (3<<20)
#define NAND_CMD_ROW_ADDRCYCLE_4        (4<<20)

#define NAND_CMD_WCMD               (1<<16)     /* Write Command                                       */
#define NAND_CMD_RWDI               (1<<17)     /* RW Direction,0:read,1:write                         */
#define NAND_CMD_RW_READ            (0<<17)
#define NAND_CMD_RW_WRITE           (1<<17)

//================================= Define Nand Flash Controller Command ===============================
/* read command */
#define NAND_CMD_READ_00 \
(0x00 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_SCOL_ADDR | NAND_CMD_SEQU)
#define NAND_CMD_READ_60 \
(0x60 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3  | NAND_CMD_SEQU)
#define NAND_CMD_READ_30 \
(0x30 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RBI | NAND_CMD_RW_READ | NAND_CMD_SEQU)
#define NAND_CMD_SPARE_READ_00 \
(0x00 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_SCOL_ADDR | NAND_CMD_SEQU)
#define NAND_CMD_COPYBACK_READ_00 \
(0x00 | NAND_CMD_NORMAL_MODE |NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_SCOL_ADDR | NAND_CMD_SEQU)
#define NAND_CMD_COPY_READ_60 \
(0x60 | NAND_CMD_NORMAL_MODE |NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3)
#define NAND_CMD_SPARE_READ_30 \
(0x30 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_RBI | NAND_CMD_DMAS | NAND_CMD_RW_READ | NAND_CMD_SEQU)
#define NAND_CMD_COPYBACK_READ_35 \
(0x35 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)// | NAND_CMD_RW_READ
#define NAND_CMD_RANDOM_READ_05 \
(0x05 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_SCOL_ADDR | NAND_CMD_SEQU)
#define NAND_CMD_RANDOM_READ_E0 \
(0xE0 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS | NAND_CMD_RW_READ)
#define NAND_CMD_SPARE_READ_05 \
(0x05 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_RW_READ | NAND_CMD_SCOL_ADDR | NAND_CMD_SEQU)
#define NAND_CMD_SPARE_READ_E0 \
(0xE0 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_RW_READ | NAND_CMD_DMAS)

/* write command */
#define NAND_CMD_WRITE_80 \
(0x80 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_SCOL_ADDR | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_DMAS)
#define NAND_CMD_RANDOM_WRITE_85 \
(0x85 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_SCOL_ADDR | NAND_CMD_DMAS  | NAND_CMD_SEQU)
#define NAND_CMD_COPYBACK_WRITE_85 \
(0x85 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_SCOL_ADDR | NAND_CMD_ROW_ADDRCYCLE_3)
#define NAND_CMD_WRITE_81 \
(0x81 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_SCOL_ADDR | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_DMAS)
#define NAND_CMD_COPYBACK_WRITE_81 \
(0x81 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3)

#define NAND_CMD_SPARE_WRITE_85 \
(0x85 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_SCOL_ADDR |NAND_CMD_RW_WRITE | NAND_CMD_DMAS)
#define NAND_CMD_WRITE_10 \
(0x10 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)
#define NAND_CMD_WRITE_15 \
(0x15 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)
#define NAND_CMD_WRITE_11 \
(0x11 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* block erase command */
#define NAND_CMD_ERASE_60 \
(0x60 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_3 | NAND_CMD_SEQU)
#define NAND_CMD_ERASE_D0	\
(0xd0 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* read status command */
#define NAND_CMD_STATUS_70	\
(0x70 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)
#define NAND_CMD_STATUS_71	\
(0x71 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)

/* read nand chip ID command */
#define NAND_CMD_READID_90	\
(0x90 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_1 | NAND_CMD_DMAS)

/* reset nand command */
#define NAND_CMD_RESET_FF \
(0xFF | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* read interleave status command */
#define NAND_CMD_READSTATUS_F1	\
(0xf1 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)
#define NAND_CMD_READSTATUS_F2	\
(0xf2 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)



#define NAND_CMD_SB_WMAIN_POINT_00 \
	(0x00 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* write to flash */
#define NAND_CMD_SB_WRITE_80 \
	(0x80 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_4 | NAND_CMD_RW_WRITE | NAND_CMD_DMAS) 

#define NAND_CMD_SB_WRITE_CONFIRM_10 \
	(0x10 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

#define NAND_CMD_SB_WRITE_SPARE \
	(0x00 | NAND_CMD_SPARE_MODE | NAND_CMD_RW_WRITE | NAND_CMD_DMAS)

/* 0x004d0000 */
#define NAND_CMD_SB_READ_MAIN_00 \
	(0x00 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_4 | NAND_CMD_RW_READ | NAND_CMD_DMAS | NAND_CMD_RBI)

/* 0x014d0050 */
#define NAND_CMD_SB_READ_SPARE_50 \
	(0x50 | NAND_CMD_SPARE_MODE | NAND_CMD_WCMD | NAND_CMD_ROW_ADDRCYCLE_4 | NAND_CMD_RW_READ | NAND_CMD_DMAS | NAND_CMD_RBI)

//======================================define controlling and status bit==============================

/*** NAND_CTL: (NAND_CTL Offset: 0x00) Nand Flash Contorller Enable Register ***/
#define NAND_CTL_EN                 (1<<0)  /* NANDFLASH_IF_ENB */
#define NAND_CTL_RST                (1<<1)  /* FSM_RST */
#define NAND_CTL_CECT               (1<<2)  /* CE CTL bit */
#define NAND_CTL_CE0E               (1<<3)  /* CE0- Enable */
#define NAND_CTL_CE1E               (1<<4)  /* CE1- Enable */
#define NAND_CTL_CE2E               (1<<5)  /* CE2- Enable */
#define NAND_CTL_CE3E               (1<<6)  /* CE3- Enable */
#define NAND_CTL_MODE               (1<<8)  /* bit[8], Access Mode Select */
#define	NAND_CTL_MODE_CONV          (0<<8)  /* Conventional Serial Access Mode */
#define	NAND_CTL_MODE_EDO           (1<<8)  /* EDO type Serial Access Mode */
#define NAND_CTL_BWID               (1<<9) /* Bus Width Select. 0 Ext. memory bus is 8bit, 1 Ext. Memory bus is 16bit. */
#define NAND_CTL_BWID_8BIT          (0<<9) /* Ext. memory bus is 8bit */
#define NAND_CTL_BWID_16BIT         (1<<9) /* Ext. memory bus is 16bit */
#define NAND_BUS_SLT			(1<<10)	/*bus selection, 0: system bus, 1:special bus*/
#define	NAND_Specail_BUS		(1<<10)
#define	NAND_AHB_BUS			(0<<10)
#define NAND_TADL_WAIT               (3<<11)	/*TADL wait*/
#define NAND_TADL_WAIT_4CLK      (0<<11)	/*TADL wait, 4 clk*/
#define NAND_TADL_WAIT_8CLK      (1<<11)	/*TADL wait , 8 clk*/
#define NAND_TADL_WAIT_16CLK      (2<<11)	/*TADL wait , 8 clk*/
#define	NAND_BLOC_MODE		(1<<13)	/*DMA block mode enable*/
#define	NAND_VPDE_DETECT		(1<<14)	/*NF_VP detect circuit enable*/
#define	NAND_VP_ST				(1<<15)	/*NF_VP status*/
#define	NAND_VP_ST_18v		(0<<15)	/*1.8v*/
#define	NAND_VP_ST_33V		(1<<15)	/*3.3v*/
#define	NAND_PD_DCTL			(1<<16)	/*nandflash pad drive control*/
#define	NAND_RB_SELECT		(1<<17)	/*RB select*/ 
#define	NAND_RB_RB1PE			(1<<18)	/*RB1 pull up 2.2k resistance enable*/
#define	NAND_RB_RB2PE			(1<<19)	/*RB1 pull up 2.2k resistance enable*/
#define	NAND_SM_EIE			(1<<20)	/*status machine ending IRQ enable*/
#define	NAND_RB_RBIE1			(1<<21)	/*RB1 RDY IRQ enable*/
#define	NAND_RB_RBIE2			(1<<22)	/*RB2 RDY IRQ enable*/


/*** NAND_STATUS: (NAND_STATUS Offset: 0x04) Nand Status  Register ***/
#define NAND_STATUS_EIE             (1<<0)  /* State Machine End IRQ Enable. 0: disable, 1: enable. */
#define NAND_STATUS_RBPD1            (1<<1)  /* RB1 RDY IRQ Pending. */
#define NAND_STATUS_RBPD2            (1<<2)  /* RB2 RDY IRQ Pending. */
#define NAND_STATUS_RB1ST            (1<<3)  /* RB signal1 status bit*/
#define NAND_STATUS_RB2ST            (1<<4)  /* RB signal2 status bit*/
#define NAND_STATUS_DRQEN            (1<<5)  /* DRQ enable(for debug)*/
#define NAND_STATUS_DRQ            (1<<6)  /* DRQ state(for debug)*/
#define NAND_STATUS_EMPT            (1<<7)  /* nand fifo_state, 0: not empty, 1: empty*/
#define NAND_STATUS_FULL            (1<<8)  /* nand fifo_state, 0: not full, 1: full*/
#define NAND_STATUS_DAFF            (1<<9)  /* data all 0xff mark in sector transmit, 0: not all 0xff; 1: all 0xff*/
#define NAND_STATUS_STAT		(1<<31) /*FSM_status, 0: idle stae, 1: busy*/

/*** NAND_ECCCTL: (NAND_ECCCTL Offset: 0x2c) ECC Controller & Status Register ***/
#define NAND_ECCCTL_ENABLE            (1<<0)  /* ECC enable*/
#define NAND_ECCCTL_TYPE_DISABLE    (0<<0)  /*ECC disable */
#define NAND_ECCCTL_TEST            (1<<1)  /* Test mode. 0: normal mode. 1:test mode*/
#define NAND_ECCCTL_ECCM            (1<<2)  /* BCH encoding algorithm includes user data*/
#define	NAND_ECCCTL_ECCM_N	(1<<2)	/*not include user data*/
#define	NAND_ECCCTL_ECCM_Y	(0<<2)	/*include user data*/
#define NAND_ECCCTL_SPSTC            (3<<3)  /* Spare data struct [4:3] */
#define	NAND_ECCCTL_SPSTC_16	(0<<3)	/*512B + 16B*/
#define	NAND_ECCCTL_SPSTC_13	(1<<3)	/*512B + 13B*/
#define	NAND_ECCCTL_SPSTC_14	(2<<3)	/*512B + 14B*/
#define NAND_ECCCTL_ECCS            (3<<28)  /* ECC error flash within the transmit 512B*/

//================================ define some operations macro============================
#define NAND_REG_READ(reg) \
        (*(volatile INT32U *)((NAND_IOMAP_BASE) + (reg)))

#define NAND_REG_READ_B(reg) \
        (*(volatile INT8U *)((NAND_IOMAP_BASE) + (reg)))

#define NAND_REG_WRITE(reg, data) \
        ((*(volatile INT32U *)((NAND_IOMAP_BASE) + (reg))) = (data))

#define NAND_REG_WRITE_B(reg, data) \
        ((*(volatile INT8U *)((NAND_IOMAP_BASE) + (reg))) = (data))

#define REG_READ(reg) \
        (*(volatile INT32U *)((INT32U)reg) )

#define REG_WRITE(reg, data) \
        ((*(volatile INT32U *)((INT32U)reg)) = (data))

/*
	In GL3996, 3 operationS (wait_fifo_no_empt,wait_fifo_no_full,wait_sm_finish) 
	can't be executed immediately after send command. use one cycle delay is OK.
*/

/*if fifo not empty, can read data from it.*/

	
/*if fifo not empty, can read data from it.*/
#define wait_fifo_no_empt()\
{	INT32U i;\
	i = NAND_REG_READ(NAND_CMD_FSM);\
	while(NAND_REG_READ(NAND_STATUS)& NAND_STATUS_EMPT);\
}	

/*if fifo not full, can't write data to it.*/		


#define wait_fifo_no_full()\
{	INT32U i;\
	i = NAND_REG_READ(NAND_CMD_FSM);\
        while(NAND_REG_READ(NAND_STATUS)& NAND_STATUS_FULL);\
}



#define wait_sm_finish()\
{	INT32U i;\
	i = NAND_REG_READ(NAND_CMD_FSM);\
	while(NAND_REG_READ(NAND_STATUS)& NAND_STATUS_STAT);\
}
#define wait_nand_ready()\
        while(!(NAND_REG_READ(NAND_STATUS)& NAND_STATUS_RB1ST));
     
        

#define clear_watch_dog()\
        REG_WRITE((WATCH_DOG_CONTROL + 0xb0018000), (REG_READ(WATCH_DOG_CONTROL + 0xb0018000) | 1));

#define NAND_DMA_END_DELAY_VALUE    1000
#define NAND_FSM_END_DELAY_VALUE    1000

#define NAND_CMDRDY_DELAY_VALUE     10000000
#define NAND_TRANSRDY_DELAY_VALUE   10000000
#define NAND_RBST_DELAY_VALUE       10000000

#define CMU_DEVCLKEN_NAND           (1<<9)
//#define CMU_DEVCLKEN_GPIO           (1<<26)
#define CMU_DEVCLKEN_DMAC           (1<<8)
#define CMU_DEVCLKEN_SRAM           (1<<4)
#define CMU_DEVCLKEN_SRAMR          (1<<2)

#define CMU_DMACLK_D4EN             (1<<0)


#define MASK_COST_Write 0x30000000
#define MASK_COST_Read  0x30000000

#define DSPMEM_START_ADDR		0xb4040000
#define DSPMEM_SIZE				0x18000
#define IS_DSPMEM_ADDR(addr) \
	(((INT32U)(addr)>= DSPMEM_START_ADDR) && ((INT32U)(addr) < (DSPMEM_START_ADDR+DSPMEM_SIZE)))

#define CACHE_START_ADDR        0x80000000
#define CACHE_SIZE              0x20000000
#define IS_CACHE_ADDR(addr)         \
    (((INT32U)(addr) >= CACHE_START_ADDR) && ((INT32U)(addr) < (CACHE_START_ADDR+CACHE_SIZE)))

#define SPECIAL_DMA_START       0x04
#define ISSPECIALDMA(nDMANum)       (nDMANum >= SPECIAL_DMA_START)

#if CONFIG_AM_CHIP_ID ==1203 ||CONFIG_AM_CHIP_ID == 1211 
/* 0x04 1wait  0x08 2Wait 0x08 2Wait   */

#define AM7331CFG_NAND_CLKCTL     0x04 //0x0c OK 8->2Wait state/* */
#define AM7331CFG_CMU_NANDCLK     0x03//0x04

#define AM7531CFG_NAND_CLKCTL     0x04 //0x0c OK 8->2Wait state/* */
#define AM7531CFG_CMU_NANDCLK     0x04//0x04

#define  CFG_NAND_CLKCTL        0x04 //0x0c OK 8->2Wait state/* */
#define  CFG_CMU_NANDCLK      0x03//0x04

#define NAND_CLKCTLVal  AM7531CFG_NAND_CLKCTL

#elif CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

#define AM7331CFG_NAND_CLKCTL     0x04 //0x0c OK 8->2Wait state/* */
#define AM7331CFG_CMU_NANDCLK     0x03//0x04

#define AM7531CFG_NAND_CLKCTL     0x04 //0x0c OK 8->2Wait state/* */
#define AM7531CFG_CMU_NANDCLK     0x04//0x04

#define  CFG_NAND_CLKCTL        0x09 //0x0c OK 8->2Wait state/* */
#define  CFG_CMU_NANDCLK      0x03//0x04

#define NAND_CLKCTLVal  AM7531CFG_NAND_CLKCTL

#else
#error "CHIP Error "
#endif


#endif /* __NAND_FLASH_REG__ */

