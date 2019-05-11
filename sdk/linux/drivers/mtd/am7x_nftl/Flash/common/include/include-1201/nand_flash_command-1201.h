/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : nand_flash_command.h
* By      : nand flash group
* Version : V0.1
* Date    : 2007-7-3 17:29
*********************************************************************************************************
*/

#ifndef _NAND_FLASH_COMMAND_
#define _NAND_FLASH_COMMAND_

#if 0
/*below definitions are for GL3963.*/
//NAND_CMD: (NAND_CMD Offset: 0x2C) Nand Flash COMMAND Register
#define NAND_CMD_NORMAL_MODE        (00<<30)
#define NAND_CMD_SPARE_MODE         (01<<30)

#define NAND_CMD_SEQU               (1<<29)     /* Sequent CMD                                         */
#define NAND_CMD_DMAS               (1<<22)     /* DMA Sent Control. Writing 1 to this bit State Machine
                                                   sent the DMA. IF RBI=0, State Machine sent the DMA no
                                                   wait the RBI ready; if RBI=1, State Machine sent the
                                                   DMA must be wait the RBI ready.                     */
#define NAND_CMD_SPAR               (1<<23)
#define NAND_CMD_RBI                (1<<21)     /* RBI CTL. 0: State Machine sent DMA no wait RBI ready;
                                                   1: State Machine sent DMA wait RBI ready.           */
#define NAND_CMD_ADDRCYCLE          (3<<18)     /* bit 20:18, Address Cycle Selection                  */
#define NAND_CMD_ADDRCYCLE_0        (0<<18)
#define NAND_CMD_ADDRCYCLE_1        (1<<18)
#define NAND_CMD_ADDRCYCLE_2        (2<<18)
#define NAND_CMD_ADDRCYCLE_3        (3<<18)
#define NAND_CMD_ADDRCYCLE_4        (4<<18)
#define NAND_CMD_ADDRCYCLE_5        (5<<18)
#define NAND_CMD_ADDRCYCLE_6        (6<<18)
#define NAND_CMD_WCMD               (1<<17)     /* Write Command                                       */
#define NAND_CMD_RWDI               (1<<16)     /* RW Direction,0:read,1:write                         */
#define NAND_CMD_RW_READ            (0<<16)
#define NAND_CMD_RW_WRITE           (1<<16)
#define NAND_CMD_CMDHI              (1<<8)      /* Flash COMMAND [15..8]                               */
#define NAND_CMD_CMDLI              (0)         /* Flash COMMAND [7...0]                               */

#define NAND_CMD_SPAR_LEN           (1<<24)     /* set the spare len of sector to 16byte               */
#define NAND_CMD_SEC_LEN            (1<<20)     /* set the sector len to 512byte                       */



/* page read command define */
#define NAND_CMD_READ_00 \
(0x00 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_5 | NAND_CMD_SEQU)
#define NAND_CMD_READ_30 \
(0x30 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RBI | NAND_CMD_RW_READ)
#define NAND_CMD_COPY_READ_60 \
(0x60 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_3)
#define NAND_CMD_COPYBACK_READ_35 \
(0x35 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)
#define NAND_CMD_RANDOM_READ_05 \
(0x05 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_2 | NAND_CMD_SEQU)
#define NAND_CMD_RANDOM_READ_E0 \
(0xE0 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS | NAND_CMD_RW_READ)
#define NAND_CMD_SPARE_READ_05 \
(0x05 | NAND_CMD_NORMAL_MODE | NAND_CMD_ADDRCYCLE_2 | NAND_CMD_WCMD | NAND_CMD_RW_READ | NAND_CMD_SEQU)
#define NAND_CMD_SPARE_READ_E0 \
(0xE0 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_READ | NAND_CMD_SPAR)

/* page write command define */
#define NAND_CMD_WRITE_80 \
(0x80 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_ADDRCYCLE_5)
#define NAND_CMD_RANDOM_WRITE_85 \
(0x85 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_ADDRCYCLE_2 | NAND_CMD_DMAS)
#define NAND_CMD_COPYBACK_WRITE_85 \
(0x85 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_5)
#define NAND_CMD_WRITE_81 \
(0x81 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_ADDRCYCLE_5)
#define NAND_CMD_COPYBACK_WRITE_81 \
(0x81 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_5)

#define NAND_CMD_SPARE_WRITE_85 \
(0x85 | NAND_CMD_SPAR | NAND_CMD_ADDRCYCLE_2 | NAND_CMD_WCMD | NAND_CMD_RW_WRITE)
#define NAND_CMD_WRITE_10 \
(0x10 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)
#define NAND_CMD_WRITE_15 \
(0x15 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)
#define NAND_CMD_WRITE_11 \
(0x11 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* block erase command */
#define NAND_CMD_ERASE_60 \
(0x60 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_3)
#define NAND_CMD_ERASE_D0	\
(0xd0 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

/* read status command */
#define NAND_CMD_STATUS_70	\
(0x70 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)
#define NAND_CMD_STATUS_71	\
(0x71 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)

/* read interleave status command */
#define NAND_CMD_STATUS_F1	\
(0xF1 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)
#define NAND_CMD_STATUS_F2	\
(0xF2 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_DMAS)


/* read nand chip ID command */
#define NAND_CMD_READID_90	\
(0x90 | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD | NAND_CMD_ADDRCYCLE_1 | NAND_CMD_DMAS)

/* reset nand command */
#define NAND_CMD_RESET_FF \
(0xFF | NAND_CMD_NORMAL_MODE | NAND_CMD_WCMD)

#endif 


#endif//the end of #ifndef _NAND_FLASH_COMMAND_
