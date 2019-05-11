#ifndef  _ACTIONS_REGISTER_1203_H_
#define  _ACTIONS_REGISTER_1203_H_
/**
*@file actions_1203_regs.h
*
* @brief This file is the register entry of actions chip am7x31
*
*@author yekai
*@date 2010-01-29
*@version 0.1
*/
#if CONFIG_AM_CHIP_ID != 1203
# error --------------
#endif

#define __USE_STRICT_SYNTAX__   0

#if __USE_STRICT_SYNTAX__
typedef struct am7x_hwreg *AM7X_HWREG;
#define  __AM7X_HWREG(reg)   (struct am7x_hwreg *)(reg)
#else
typedef unsigned long      AM7X_HWREG;
#define  __AM7X_HWREG(reg)   (reg)
#endif

//--------PMU/LRADC--------------------------------------------------
#define  PMU_CTL                        __AM7X_HWREG(0xB0000000+0x00)
#define  PMU_LRADC                      __AM7X_HWREG(0xB0000000+0x04)
#define  PMU_CHG                        __AM7X_HWREG(0xB0000000+0x08)

#define  AMU_CTL                        __AM7X_HWREG(0xB0000000+0x00)
#define  BL_CTL                         __AM7X_HWREG(0xB0000000+0x04)
#define  USB_PHY_CTL                    __AM7X_HWREG(0xB0000000+0x40)
#define  USB_EFUSE_REG                  __AM7X_HWREG(0xB0000000+0x44)
#define  USB_PHY_PLUG                   __AM7X_HWREG(0xB0000000+0x48)

//--------CMU/HOSC--------------------------------------------------
#define  CMU_COREPLL                    __AM7X_HWREG(0xB0010000+0x00)
#define  CMU_HOSC                       __AM7X_HWREG(0xB0010000+0x04)
#define  CMU_AUDIOPLL                   __AM7X_HWREG(0xB0010000+0x08)
#define  CMU_BUSCLK                     __AM7X_HWREG(0xB0010000+0x0C)

#define  CMU_HCMUTEST                   __AM7X_HWREG(0xB0010000+0x10)
#define  CMU_SPCLK                      __AM7X_HWREG(0xB0010000+0x38)
#define  PWM0_CTL                       __AM7X_HWREG(0xB0010000+0xA0)
#define  PWM1_CTL                       __AM7X_HWREG(0xB0010000+0xA4)
#define  CMU_TROMCLK                    __AM7X_HWREG(0xB0010000+0xA8)
#define  CMU_DISPLAYCLK2                __AM7X_HWREG(0xB0010000+0xAC)


#define  CMU_NANDCLK                    __AM7X_HWREG(0xB0010000+0x18)
#define  CMU_SDCLK                      __AM7X_HWREG(0xB0010000+0x1C)

#define  CMU_UART1CLK                   __AM7X_HWREG(0xB0010000+0x28)
#define  CMU_UART2CLK                   __AM7X_HWREG(0xB0010000+0x2C)
#define  CMU_DMACLK                     __AM7X_HWREG(0xB0010000+0x30)
#define  CMU_FMCLK                      __AM7X_HWREG(0xB0010000+0x34)

#define  CMU_DEVCLKEN                   __AM7X_HWREG(0xB0010000+0x80)
#define  CMU_DEVRST                     __AM7X_HWREG(0xB0010000+0x84)
#define  CMU_LCDPLL                     __AM7X_HWREG(0xB0010000+0x90)
#define  CMU_CARDCLK                    __AM7X_HWREG(0xB0010000+0x94)
#define  CMU_DISPLAYCLK                 __AM7X_HWREG(0xB0010000+0x98)
#define  CMU_CARDCLKSEL                 __AM7X_HWREG(0xB0010000+0x9C)

//--------RTC/LOSC/WD--------------------------------------------
#define  RTC_CTL                        __AM7X_HWREG(0xB0018000+0x00)
#define  RTC_DHMS                       __AM7X_HWREG(0xB0018000+0x04)
#define  RTC_YMD                        __AM7X_HWREG(0xB0018000+0x08)
#define  RTC_DHMSALM                    __AM7X_HWREG(0xB0018000+0x0C)
#define  RTC_YMDALM                     __AM7X_HWREG(0xB0018000+0x10)

#define  RTC_STATUS                     __AM7X_HWREG(0xB0018000+0x14)
#define  RTC_WDCTL                      __AM7X_HWREG(0xb0140000+0x14)
#define  RTC_T0CTL                      __AM7X_HWREG(0xb0140000+0x18)
#define  RTC_T0                         __AM7X_HWREG(0xb0140000+0x1C)
#define  RTC_T1CTL                      __AM7X_HWREG(0xb0140000+0x20)
#define  RTC_T1                         __AM7X_HWREG(0xb0140000+0x24)

#define  WD_CTL			RTC_WDCTL
#define  TIMER0_CTL		RTC_T0CTL
#define  TIMER0_VALUE	RTC_T0
#define  TIMER1_CTL		RTC_T1CTL
#define  TIMER1_VALUE	RTC_T1
//--------INTERRUPT CONTROLLER---------------------------------------
#define  INTC_PD                        __AM7X_HWREG(0xB0020000+0x00)
#define  INTC_MSK                       __AM7X_HWREG(0xB0020000+0x04)
#define  INTC_CFG0                      __AM7X_HWREG(0xB0020000+0x08)
#define  INTC_CFG1                      __AM7X_HWREG(0xB0020000+0x0C)
#define  INTC_CFG2                      __AM7X_HWREG(0xB0020000+0x10)
#define  INTC_EXTCTL01                  __AM7X_HWREG(0xB0020000+0x14)
#define  INTC_EXTCTL23                  __AM7X_HWREG(0xB0020000+0x18)

//--------SRAM ON CHIP-----------------------------------------------
#define  SRAMOC_CTL                     __AM7X_HWREG(0xB0030000+0x00)
#define  SRAMOC_STAT                    __AM7X_HWREG(0xB0030000+0x04)

#define  SRAM_INT_CTL                   __AM7X_HWREG(0xB0030000+0x04)
#define  SRAM_STH_CTL                   __AM7X_HWREG(0xB0030000+0x08)

//--------NOR FLASH/SRAM/BROM/ID-------------------------------------
#define  NOR_CTL                        __AM7X_HWREG(0xB0038000+0x00)
#define  NOR_BROMCTL                    __AM7X_HWREG(0xB0038000+0x04)
#define  NOR_CHIPID                     __AM7X_HWREG(0xB0038000+0x08)

//--------32BIT RISC CORE--------------------------------------------
#define  PCNT_CTL                       __AM7X_HWREG(0xB003C000+0x00)
#define  PCNT_PC0                       __AM7X_HWREG(0xB003C000+0x04)
#define  PCNT_PC1                       __AM7X_HWREG(0xB003C000+0x08)

//--------DSP CONTROL------------------------------------------------
#define  DSP_HDR0                       __AM7X_HWREG(0xB0050000+0x00)
#define  DSP_HDR1                       __AM7X_HWREG(0xB0050000+0x04)
#define  DSP_HDR2                       __AM7X_HWREG(0xB0050000+0x08)
#define  DSP_HDR3                       __AM7X_HWREG(0xB0050000+0x0C)
#define  DSP_HDR4                       __AM7X_HWREG(0xB0050000+0x10)
#define  DSP_HDR5                       __AM7X_HWREG(0xB0050000+0x14)
#define  DSP_HSR6                       __AM7X_HWREG(0xB0050000+0x18)
#define  DSP_HSR7                       __AM7X_HWREG(0xB0050000+0x1C)
#define  DSP_CTL                        __AM7X_HWREG(0xB0050000+0x20)

//--------DMA CONTROLLER---------------------------------------------
//DMA Configuration Register
#define  DMA_CTL                        __AM7X_HWREG(0xB0060000+0x00)
#define  DMA_IRQEN                      __AM7X_HWREG(0xB0060000+0x04)
#define  DMA_IRQPD                      __AM7X_HWREG(0xB0060000+0x08)
#define  DMA_DEBUG                      __AM7X_HWREG(0xB0060000+0x0C)
#define  DMA_TIMEOUT                    __AM7X_HWREG(0xB0060000+0x10)
#define  DMA_STATE                      __AM7X_HWREG(0xB0060000+0x14)

#define  DMA_WEIGHT4                    __AM7X_HWREG(0xB0060000+0x50)
#define  DMA_WEIGHT5                    __AM7X_HWREG(0xB0060000+0x54)
#define  DMA_WEIGHT6                    __AM7X_HWREG(0xB0060000+0x58)
#define  DMA_WEIGHT7                    __AM7X_HWREG(0xB0060000+0x5C)
#define  DMA_WEIGHT8                    __AM7X_HWREG(0xB0060000+0x60)
#define  DMA_WEIGHT9                    __AM7X_HWREG(0xB0060000+0x64)
#define  DMA_WEIGHT10                   __AM7X_HWREG(0xB0060000+0x68)
#define  DMA_WEIGHT11                   __AM7X_HWREG(0xB0060000+0x6C)
//DMA0 Control Register
#define  DMA_MODE0                      __AM7X_HWREG(0xB0060100+0x00)
#define  DMA_SRC0                       __AM7X_HWREG(0xB0060100+0x04)
#define  DMA_DST0                       __AM7X_HWREG(0xB0060100+0x08)
#define  DMA_CNT0                       __AM7X_HWREG(0xB0060100+0x0C)
#define  DMA_REM0                       __AM7X_HWREG(0xB0060100+0x10)
#define  DMA_CMD0                       __AM7X_HWREG(0xB0060100+0x14)
//DMA1 Control Register
#define  DMA_MODE1                      __AM7X_HWREG(0xB0060120+0x00)
#define  DMA_SRC1                       __AM7X_HWREG(0xB0060120+0x04)
#define  DMA_DST1                       __AM7X_HWREG(0xB0060120+0x08)
#define  DMA_CNT1                       __AM7X_HWREG(0xB0060120+0x0C)
#define  DMA_REM1                       __AM7X_HWREG(0xB0060120+0x10)
#define  DMA_CMD1                       __AM7X_HWREG(0xB0060120+0x14)
//DMA2 Control Register
#define  DMA_MODE2                      __AM7X_HWREG(0xB0060140+0x00)
#define  DMA_SRC2                       __AM7X_HWREG(0xB0060140+0x04)
#define  DMA_DST2                       __AM7X_HWREG(0xB0060140+0x08)
#define  DMA_CNT2                       __AM7X_HWREG(0xB0060140+0x0C)
#define  DMA_REM2                       __AM7X_HWREG(0xB0060140+0x10)
#define  DMA_CMD2                       __AM7X_HWREG(0xB0060140+0x14)
//DMA3 Control Register
#define  DMA_MODE3                      __AM7X_HWREG(0xB0060160+0x00)
#define  DMA_SRC3                       __AM7X_HWREG(0xB0060160+0x04)
#define  DMA_DST3                       __AM7X_HWREG(0xB0060160+0x08)
#define  DMA_CNT3                       __AM7X_HWREG(0xB0060160+0x0C)
#define  DMA_REM3                       __AM7X_HWREG(0xB0060160+0x10)
#define  DMA_CMD3                       __AM7X_HWREG(0xB0060160+0x14)
//DMA4 Control Register
#define  DMA_MODE4                      __AM7X_HWREG(0xB0060180+0x00)
#define  DMA_SRC4                       __AM7X_HWREG(0xB0060180+0x04)
#define  DMA_DST4                       __AM7X_HWREG(0xB0060180+0x08)
#define  DMA_CNT4                       __AM7X_HWREG(0xB0060180+0x0C)
#define  DMA_REM4                       __AM7X_HWREG(0xB0060180+0x10)
#define  DMA_CMD4                       __AM7X_HWREG(0xB0060180+0x14)
//DMA5 Control Register
#define  DMA_MODE5                      __AM7X_HWREG(0xB00601A0+0x00)
#define  DMA_SRC5                       __AM7X_HWREG(0xB00601A0+0x04)
#define  DMA_DST5                       __AM7X_HWREG(0xB00601A0+0x08)
#define  DMA_REM5                       __AM7X_HWREG(0xB00601A0+0x10)
#define  DMA_CNT5                       __AM7X_HWREG(0xB00601A0+0x0C)
#define  DMA_CMD5                       __AM7X_HWREG(0xB00601A0+0x14)
//DMA6 Control Register
#define  DMA_MODE6                      __AM7X_HWREG(0xB00601C0+0x00)
#define  DMA_SRC6                       __AM7X_HWREG(0xB00601C0+0x04)
#define  DMA_DST6                       __AM7X_HWREG(0xB00601C0+0x08)
#define  DMA_CNT6                       __AM7X_HWREG(0xB00601C0+0x0C)
#define  DMA_REM6                       __AM7X_HWREG(0xB00601C0+0x10)
#define  DMA_CMD6                       __AM7X_HWREG(0xB00601C0+0x14)
//DMA7 Control Register
#define  DMA_MODE7                      __AM7X_HWREG(0xB00601E0+0x00)
#define  DMA_SRC7                       __AM7X_HWREG(0xB00601E0+0x04)
#define  DMA_DST7                       __AM7X_HWREG(0xB00601E0+0x08)
#define  DMA_CNT7                       __AM7X_HWREG(0xB00601E0+0x0C)
#define  DMA_REM7                       __AM7X_HWREG(0xB00601E0+0x10)
#define  DMA_CMD7                       __AM7X_HWREG(0xB00601E0+0x14)
//DMA8 Control Register
#define  DMA_MODE8                      __AM7X_HWREG(0xB0060200+0x00)
#define  DMA_SRC8                       __AM7X_HWREG(0xB0060200+0x04)
#define  DMA_DST8                       __AM7X_HWREG(0xB0060200+0x08)
#define  DMA_CNT8                       __AM7X_HWREG(0xB0060200+0x0C)
#define  DMA_REM8                       __AM7X_HWREG(0xB0060200+0x10)
#define  DMA_CMD8                       __AM7X_HWREG(0xB0060200+0x14)
//DMA9 Control Register
#define  DMA_MODE9                      __AM7X_HWREG(0xB0060220+0x00)
#define  DMA_SRC9                       __AM7X_HWREG(0xB0060220+0x04)
#define  DMA_DST9                       __AM7X_HWREG(0xB0060220+0x08)
#define  DMA_CNT9                       __AM7X_HWREG(0xB0060220+0x0C)
#define  DMA_REM9                       __AM7X_HWREG(0xB0060220+0x10)
#define  DMA_CMD9                       __AM7X_HWREG(0xB0060220+0x14)
//DMA10 Control Register
#define  DMA_MODE10                     __AM7X_HWREG(0xB0060240+0x00)
#define  DMA_SRC10                      __AM7X_HWREG(0xB0060240+0x04)
#define  DMA_DST10                      __AM7X_HWREG(0xB0060240+0x08)
#define  DMA_CNT10                      __AM7X_HWREG(0xB0060240+0x0C)
#define  DMA_REM10                      __AM7X_HWREG(0xB0060240+0x10)
#define  DMA_CMD10                      __AM7X_HWREG(0xB0060240+0x14)
//DMA11 Control Register
#define  DMA_MODE11                     __AM7X_HWREG(0xB0060260+0x00)
#define  DMA_SRC11                      __AM7X_HWREG(0xB0060260+0x04)
#define  DMA_DST11                      __AM7X_HWREG(0xB0060260+0x08)
#define  DMA_CNT11                      __AM7X_HWREG(0xB0060260+0x0C)
#define  DMA_REM11                      __AM7X_HWREG(0xB0060260+0x10)
#define  DMA_CMD11                      __AM7X_HWREG(0xB0060260+0x14)

//--------SDRAM/DDR2 CONTROLLER-    -----------------------------------
#define  SDR_CTL                        __AM7X_HWREG(0xB0070000+0x00)
#define  SDR_ADDRCFG                    __AM7X_HWREG(0xB0070000+0x04)
#define  SDR_EN                         __AM7X_HWREG(0xB0070000+0x08)
#define  SDR_CMD                        __AM7X_HWREG(0xB0070000+0x0C)
#define  SDR_STAT                       __AM7X_HWREG(0xB0070000+0x10)
#define  SDR_RFSH                       __AM7X_HWREG(0xB0070000+0x14)
#define  SDR_MODE                       __AM7X_HWREG(0xB0070000+0x18)
#define  SDR_MOBILE                     __AM7X_HWREG(0xB0070000+0x1C)

#define  SDR_TIMEOUT                    __AM7X_HWREG(0xB0070000+0x20)
#define  SDR_CLKDLY                     __AM7X_HWREG(0xB0070000+0x24)
#define  SDR_INITD                      __AM7X_HWREG(0xB0070000+0x28)
#define  SDR_SR                         __AM7X_HWREG(0xB0070000+0x2C)
#define  SDR_TIMING                     __AM7X_HWREG(0xB0070000+0x30)
#define  SDR_PD                         __AM7X_HWREG(0xB0070000+0x34)
#define  SDR_STAT_EXT                   __AM7X_HWREG(0xB0070000+0x38)
#define  SDR_VERSION                    __AM7X_HWREG(0xB0070000+0x3C)
//--------MCA(WMA)---------------------------------------------------
#define  MCA_CTL                        __AM7X_HWREG(0xB0080000+0x00)

//--------ATA I/F----------------------------------------------------
#define  ATA_CONFIG                     __AM7X_HWREG(0xB0090000+0x00)
#define  ATA_UDMACTL                    __AM7X_HWREG(0xB0090000+0x04)
#define  ATA_DATA                       __AM7X_HWREG(0xB0090000+0x08)
#define  ATA_FEATURE                    __AM7X_HWREG(0xB0090000+0x0C)
#define  ATA_SECCNT                     __AM7X_HWREG(0xB0090000+0x10)
#define  ATA_SECNUM                     __AM7X_HWREG(0xB0090000+0x14)
#define  ATA_CLDLOW                     __AM7X_HWREG(0xB0090000+0x18)
#define  ATA_CLDHI                      __AM7X_HWREG(0xB0090000+0x1C)
#define  ATA_HEAD                       __AM7X_HWREG(0xB0090000+0x20)
#define  ATA_CMD                        __AM7X_HWREG(0xB0090000+0x24)
#define  ATA_BYTECNT                    __AM7X_HWREG(0xB0090000+0x28)
#define  ATA_FIFOCTL                    __AM7X_HWREG(0xB0090000+0x2C)
#define  ATA_FIFOCFG                    __AM7X_HWREG(0xB0090000+0x30)
#define  ATA_ADDRDEC                    __AM7X_HWREG(0xB0090000+0x34)
#define  ATA_IRQCTL                     __AM7X_HWREG(0xB0090000+0x38)

//--------SD I/F-----------------------------------------------------
#define  SD_EN                          __AM7X_HWREG(0xB00B0000+0x00)
#define  SD_CTL                         __AM7X_HWREG(0xB00B0000+0x04)
#define  SD_STAT                        __AM7X_HWREG(0xB00B0000+0x08)
#define  SD_CMD                         __AM7X_HWREG(0xB00B0000+0x0C)
#define  SD_ARG                         __AM7X_HWREG(0xB00B0000+0x10)
#define  SD_RSPBUF0                     __AM7X_HWREG(0xB00B0000+0x10)
#define  SD_RSPBUF1                     __AM7X_HWREG(0xB00B0000+0x14)
#define  SD_RSPBUF2                     __AM7X_HWREG(0xB00B0000+0x18)
#define  SD_RSPBUF3                     __AM7X_HWREG(0xB00B0000+0x1C)
#define  SD_RSPBUF4                     __AM7X_HWREG(0xB00B0000+0x20)
#define  SD_BLKSZ                       __AM7X_HWREG(0xB00B0000+0x24)
#define  SD_BLKNUM                      __AM7X_HWREG(0xB00B0000+0x28)
#define  SD_DAT                         __AM7X_HWREG(0xB00B0000+0x2c)
#define  SD_FIFOCTL                     __AM7X_HWREG(0xB00B0000+0x30)

#define  CRC_CTL                        __AM7X_HWREG(0xB00C0000+0x00)
#define  CRC_STATUS                     __AM7X_HWREG(0xB00C0000+0x04)
#define  CRC_DATA                       __AM7X_HWREG(0xB00C0000+0x08)
#define  CRC_IRQ_EN                     __AM7X_HWREG(0xB00C0000+0x0c)
#define  CRC_IRQ_CLR                    __AM7X_HWREG(0xB00C0000+0x10)
#define  CRC_DRQ_SET                    __AM7X_HWREG(0xB00C0000+0x14)
#define  CRC_XD_CD                      __AM7X_HWREG(0xB00C0000+0x18)
#define  CRC_SM                         __AM7X_HWREG(0xB00C0000+0x1c)
#define  CRC_DATA_P                     __AM7X_HWREG(CRC_DATA&0x1fffffff)


//--------ITU656 PORT------------------------------------------------
//General Register
#define  BT_MODESEL                     __AM7X_HWREG(0xB00D0000+0x00)
#define  BT_FIFODAT                     __AM7X_HWREG(0xB00D0000+0x04)

//Video Encoder
#define  BT_VEICTL                      __AM7X_HWREG(0xB00D0000+0x08)
#define  BT_VEIVSEPOF                   __AM7X_HWREG(0xB00D0000+0x14)
#define  BT_VEIVSEPEF                   __AM7X_HWREG(0xB00D0000+0x18)
#define  BT_VEIFTP                      __AM7X_HWREG(0xB00D0000+0x24)
#define  BT_VEIFIFOCTL                  __AM7X_HWREG(0xB00D0000+0x30)

//Video Decoder
#define  BT_VDICTL                      __AM7X_HWREG(0xB00D0000+0x08)
#define  BT_VDIHSPOS                    __AM7X_HWREG(0xB00D0000+0x0C)
#define  BT_VDIHEPOS                    __AM7X_HWREG(0xB00D0000+0x10)
#define  BT_VDIVSEPOF                   __AM7X_HWREG(0xB00D0000+0x14)
#define  BT_VDIVSEPEF                   __AM7X_HWREG(0xB00D0000+0x18)
#define  BT_VDIIRQSTA                   __AM7X_HWREG(0xB00D0000+0x28)
#define  BT_VDIXYDAT                    __AM7X_HWREG(0xB00D0000+0x2C)
#define  BT_VDIFIFOCTL                  __AM7X_HWREG(0xB00D0000+0x30)

//CMOS Sensor Interface
#define  BT_CSICTL                      __AM7X_HWREG(0xB00D0000+0x08)
#define  BT_CSIHSPOS                    __AM7X_HWREG(0xB00D0000+0x0C)
#define  BT_CSIHEPOS                    __AM7X_HWREG(0xB00D0000+0x10)
#define  BT_CSIVSPOS                    __AM7X_HWREG(0xB00D0000+0x1C)
#define  BT_CSIVEPOS                    __AM7X_HWREG(0xB00D0000+0x20)
#define  BT_CSIIRQSTA                   __AM7X_HWREG(0xB00D0000+0x28)
#define  BT_CSIXYDAT                    __AM7X_HWREG(0xB00D0000+0x2C)
#define  BT_CSIFIFOCTL                  __AM7X_HWREG(0xB00D0000+0x30)

//TS
#define  BT_TSICTL                      __AM7X_HWREG(0xB00D0000+0x08)
#define  BT_TSIFIFOCTL                  __AM7X_HWREG(0xB00D0000+0x30)

//Integrated Video Encoder
#define  BT_IVECTL                      __AM7X_HWREG(0xB00D0000+0x34)
#define  BT_IVEOUTCTL                   __AM7X_HWREG(0xB00D0000+0x38)
#define  BT_IVECOTCTL                   __AM7X_HWREG(0xB00D0000+0x3C)
#define  BT_IVEBRGCTL                   __AM7X_HWREG(0xB00D0000+0x40)
#define  BT_IVECSATCTL                  __AM7X_HWREG(0xB00D0000+0x44)
#define  BT_IVECBURCTL                  __AM7X_HWREG(0xB00D0000+0x48)
#define  BT_IVESYNCAMCTL                __AM7X_HWREG(0xB00D0000+0x4C)

//--------USB OTG-----------------------------------------------------
#define  USB_OTG_BASE                   0xB00E0000
#define  OTG_IRQ                        __AM7X_HWREG(0xB00E0000+0x1BC)
#define  OTG_FSMSTAT                    __AM7X_HWREG(0xB00E0000+0x1BD)
#define  OTG_CTRL                       __AM7X_HWREG(0xB00E0000+0x1BE)
#define  OTG_STAT                       __AM7X_HWREG(0xB00E0000+0x1BF)
#define  OTG_IEN                        __AM7X_HWREG(0xB00E0000+0x1C0)

#define  OTG_TAAIDLBDIS                 __AM7X_HWREG(0xB00E0000+0x1C1)
#define  OTG_TAWAITBCON                 __AM7X_HWREG(0xB00E0000+0x1C2)
#define  OTG_TBVBUSPLS                  __AM7X_HWREG(0xB00E0000+0x1C3)
#define  OTG_TBVBUSDISPLS               __AM7X_HWREG(0xB00E0000+0x1C7)

//--------YVU2RGB----------------------------------------------------
#define  YUV2RGB_CTL                    __AM7X_HWREG(0xB00F0000+0x00)
#define  YUV2RGB_FIFODAT                __AM7X_HWREG(0xB00F0000+0x04)
#define  YUV2RGB_CLKCTL                 __AM7X_HWREG(0xB00F0000+0x08)
#define  YUV2RGB_FCNT                   __AM7X_HWREG(0xB00F0000+0x0C)

//--------DAC+PA/IIS-------------------------------------------------
#define  DAC_CTL                        __AM7X_HWREG(0xB0100000+0x00)
#define  DAC_DAT                        __AM7X_HWREG(0xB0100000+0x04)
#define  DAC_DEBUG                      __AM7X_HWREG(0xB0100000+0x08)
#define  DAC_ANALOG                     __AM7X_HWREG(0xB0100000+0x0c)
#define  DAC_BUF_CONF1                  __AM7X_HWREG(0xB0100000+0x10)
#define  DAC_BUF_CONF2                  __AM7X_HWREG(0xB0100000+0x14)
#define  DAC_BUF_CTL                    __AM7X_HWREG(0xB0100000+0x18)
#define  DAC_KEY_SOUND_CTL              __AM7X_HWREG(0xB0100000+0x1C)
#define  DAC_KEY_SOUND_DAT              __AM7X_HWREG(0xB0100000+0x20)
#define  DAC_IRQ_DRQ_CTL                __AM7X_HWREG(0xB0100000+0x24)
#define  DAC_ANALOG2                    __AM7X_HWREG(0xB0100000+0x28)
//--------ADC--------------------------------------------------------
#define  ADC_CTL                        __AM7X_HWREG(0xB0110000+0x00)
#define  ADC_FIFOCTL                    __AM7X_HWREG(0xB0110000+0x04)
#define  ADC_DAT                        __AM7X_HWREG(0xB0110000+0x08)
#define  ADC_ANALOG                     __AM7X_HWREG(0xB0110000+0x0C)
#define  ADC_DEBUG                      __AM7X_HWREG(0xB0110000+0x10)

//--------TP---------------------------------------------------------
#define  TP_CTL                         __AM7X_HWREG(0xB0120000+0x00)
#define  TP_DAT                         __AM7X_HWREG(0xB0120000+0x04)

//--------SPIDIF-----------------------------------------------------
#define  SPDIF_CTL                      __AM7X_HWREG(0xB0140000+0x00)
#define  SPDIF_STAT                     __AM7X_HWREG(0xB0140000+0x04)
#define  SPDIF_TXDAT                    __AM7X_HWREG(0xB0140000+0x08)
#define  SPDIF_RXDAT                    __AM7X_HWREG(0xB0140000+0x0C)
#define  SPDIF_TXCSTAT                  __AM7X_HWREG(0xB0140000+0x10)
#define  SPDIF_RXCSTAT                  __AM7X_HWREG(0xB0140000+0x14)

//---------IR REMOTE-------------------------------------------------
#define  IR_REMOTE_CTRL                 __AM7X_HWREG(0xB0140000+0x00)
#define  IR_REMOTE_RXDATA               __AM7X_HWREG(0xB0140000+0x04)
#define  IR_REMOTE_ALARM_CODE           __AM7X_HWREG(0xB0140000+0x08)
#define  IR_REMOTE_STATUS               __AM7X_HWREG(0xB0140000+0x0C)

//--------PCM--------------------------------------------------------
#define  PCM_CTL                        __AM7X_HWREG(0xB0150000+0x00)
#define  PCM_STAT                       __AM7X_HWREG(0xB0150000+0x04)
#define  PCM_RXDAT                      __AM7X_HWREG(0xB0150000+0x08)
#define  PCM_TXDAT                      __AM7X_HWREG(0xB0150000+0x0C)

//--------UART-2-----------------------------------------------------
#define  UART1_CTL                      __AM7X_HWREG(0xb0160000+0x00)
#define  UART1_RXDAT                    __AM7X_HWREG(0xb0160000+0x04)
#define  UART1_TXDAT                    __AM7X_HWREG(0xb0160000+0x08)
#define  UART1_STAT                     __AM7X_HWREG(0xb0160000+0x0C)

#define  UART2_CTL                      __AM7X_HWREG(0xb0190000+0x20)
#define  UART2_RXDAT                    __AM7X_HWREG(0xb0190000+0x24)
#define  UART2_TXDAT                    __AM7X_HWREG(0xb0190000+0x28)
#define  UART2_STAT                     __AM7X_HWREG(0xb0190000+0x2C)

//--------IR--------------------------------------------------------
#define  IR_PL                          __AM7X_HWREG(0xb0160000+0x30)
#define  IR_RBC                         __AM7X_HWREG(0xb0160000+0x34)

//--------IIC--------------------------------------------------------
#define  I2C1_CTL                       __AM7X_HWREG(0xB0180000+0x00)
#define  I2C1_CLKDIV                    __AM7X_HWREG(0xB0180000+0x04)
#define  I2C1_STAT                      __AM7X_HWREG(0xB0180000+0x08)
#define  I2C1_ADDR                      __AM7X_HWREG(0xB0180000+0x0C)
#define  I2C1_DAT                       __AM7X_HWREG(0xB0180000+0x10)

#define  I2C2_CTL                       __AM7X_HWREG(0xB0180000+0x20)
#define  I2C2_CLKDIV                    __AM7X_HWREG(0xB0180000+0x24)
#define  I2C2_STAT                      __AM7X_HWREG(0xB0180000+0x28)
#define  I2C2_ADDR                      __AM7X_HWREG(0xB0180000+0x2C)
#define  I2C2_DAT                       __AM7X_HWREG(0xB0180000+0x30)


//--------SPI--------------------------------------------------------
#define  SPI_CTL                        __AM7X_HWREG(0xB0098000+0x00)
#define  SPI_SIZE                       __AM7X_HWREG(0xB0098000+0x04)
#define  SPI_CLKDIV                     __AM7X_HWREG(0xB0098000+0x08)
#define  SPI_STAT                       __AM7X_HWREG(0xB0098000+0x0C)
#define  SPI_TXDAT                      __AM7X_HWREG(0xB0098000+0x10)
#define  SPI_RXDAT                      __AM7X_HWREG(0xB0098000+0x14)

//--------KEY SCAN------------------------------------------
#define  KEY_CTL                        __AM7X_HWREG(0xB0008000+0x00)
#define  KEY_VALUE                      __AM7X_HWREG(0xB0008000+0x04)

#define  GPIO_31_0OUTEN                 __AM7X_HWREG(0xB01C0000+0x00)
#define  GPIO_31_0INEN                  __AM7X_HWREG(0xB01C0000+0x04)
#define  GPIO_31_0DAT                   __AM7X_HWREG(0xB01C0000+0x08)
#define  GPIO_63_32OUTEN                __AM7X_HWREG(0xB01C0000+0x0C)
#define  GPIO_63_32INEN                 __AM7X_HWREG(0xB01C0000+0x10)
#define  GPIO_63_32DAT                  __AM7X_HWREG(0xB01C0000+0x14)

//--------GPIO For Chip 1203--------------------------------
#define  LVDS_PAD_CFG0                  __AM7X_HWREG(0xB01C0000+0x30)
#define  LVDS_PAD_CFG1                  __AM7X_HWREG(0xB01C0000+0x34)

#define  GPIO_MFCTL0                    __AM7X_HWREG(0xB01C0000+0x40)
#define  GPIO_MFCTL1                    __AM7X_HWREG(0xB01C0000+0x44)
#define  GPIO_MFCTL2                    __AM7X_HWREG(0xB01C0000+0x48)
#define  GPIO_MFCTL3                    __AM7X_HWREG(0xB01C0000+0x4c)
#define  GPIO_MFCTL4                    __AM7X_HWREG(0xB01C0000+0x50)
#define  GPIO_MFCTL5                    __AM7X_HWREG(0xB01C0000+0x54)
#define  GPIO_MFCTL6                    __AM7X_HWREG(0xB01C0000+0x58)
#define  DRN_REG                        __AM7X_HWREG(0xB01C0000+0xA0)

#define  MU_REG1                        __AM7X_HWREG(0xB01C0000+0x80)
#define  MU_REG2                        __AM7X_HWREG(0xB01C0000+0x90)
#define  PH_REG                         __AM7X_HWREG(0xB01C0000+0x94)
#define  SDR_PD_REG                     __AM7X_HWREG(0xB01C0000+0xA0)
#define  NOR_ADDR_OE                    __AM7X_HWREG(0xB01C0000+0xB0)
#define  DEBUG_CON                      __AM7X_HWREG(0xB01C0000+0xC0)

//--------GPIO For Chip 3963D-------------------------------
#define  GPIO_AOUTEN                    __AM7X_HWREG(0xB01C0000+0x00)
#define  GPIO_AINEN                     __AM7X_HWREG(0xB01C0000+0x04)
#define  GPIO_ADAT                      __AM7X_HWREG(0xB01C0000+0x08)
#define  GPIO_BOUTEN                    __AM7X_HWREG(0xB01C0000+0x0C)
#define  GPIO_BINEN                     __AM7X_HWREG(0xB01C0000+0x10)
#define  GPIO_BDAT                      __AM7X_HWREG(0xB01C0000+0x14)
#define  GPIO_PAD_DRV                   __AM7X_HWREG(0xB01C0000+0x88)
#define  GPIO_PXMU                      __AM7X_HWREG(0xB01C0000+0x88)
#define  GPIO_PADDRV                    __AM7X_HWREG(0xB01C0000+0x88)

/*******************************************
1201和1205系列IC 如下寄存器在读写时需要注意
B00F：04，20，2C
B00D：00
1.首先在操作以上寄存器时需要先操作一下同模块
   其他寄存器。
2.操作寄存器的这两段代码，需要关中断，
并且将代码预取到cache中运行。
********************************************/
/*----------------DE Registers Description----------------*/
#define  DeBaseadd                      0xB0040000 /*phy_addr:0x10040000*/
#define  DE_Con                         __AM7X_HWREG(DeBaseadd+0x0000)
#define  D_Color                        __AM7X_HWREG(DeBaseadd+0x0004)
#define  Frm_Size                       __AM7X_HWREG(DeBaseadd+0x0008)
#define  Frm_Stride                     __AM7X_HWREG(DeBaseadd+0x000c)
#define  Frm_BA                         __AM7X_HWREG(DeBaseadd+0x0010)
#define  UV_BA                          __AM7X_HWREG(DeBaseadd+0x0014)
#define  OSD0_BA                        __AM7X_HWREG(DeBaseadd+0x0018)
#define  OSD1_BA                        __AM7X_HWREG(DeBaseadd+0x001c)
//#define  OSD2_BA                        __AM7X_HWREG(DeBaseadd+0x0020)
//#define  OSD3_BA                        __AM7X_HWREG(DeBaseadd+0x0024)
#define  OSD0_Stride                    __AM7X_HWREG(DeBaseadd+0x0028)
#define  OSD1_Stride                    __AM7X_HWREG(DeBaseadd+0x002c)
//#define  OSD2_Stride                    __AM7X_HWREG(DeBaseadd+0x0030)
//#define  OSD3_Stride                    __AM7X_HWREG(DeBaseadd+0x0034)
#define  OSD0_FIFO_Thrd                 __AM7X_HWREG(DeBaseadd+0x0030)
#define  OSD1_FIFO_Thrd                 __AM7X_HWREG(DeBaseadd+0x0034)
#define  Frm_FIFO_Thrd                  __AM7X_HWREG(DeBaseadd+0x0038)
//#define  OSD_FIFO_Thrd                  __AM7X_HWREG(DeBaseadd+0x003c)
#define  DE_Status                      __AM7X_HWREG(DeBaseadd+0x0040)
#define  Win_Coor1                      __AM7X_HWREG(DeBaseadd+0x0044)
#define  Win_Coor2                      __AM7X_HWREG(DeBaseadd+0x0048)
#define  OSD0con                        __AM7X_HWREG(DeBaseadd+0x004c)
#define  OSD0coor                       __AM7X_HWREG(DeBaseadd+0x0050)
#define  OSD0size                       __AM7X_HWREG(DeBaseadd+0x0054)
#define  OSD1con                        __AM7X_HWREG(DeBaseadd+0x0058)
#define  OSD1coor                       __AM7X_HWREG(DeBaseadd+0x005c)
#define  OSD1size                       __AM7X_HWREG(DeBaseadd+0x0060)
//#define  OSD2con                        __AM7X_HWREG(DeBaseadd+0x0064)
//#define  OSD2coor                       __AM7X_HWREG(DeBaseadd+0x0068)
//#define  OSD2size                       __AM7X_HWREG(DeBaseadd+0x006c)
//#define  OSD3con                        __AM7X_HWREG(DeBaseadd+0x0070)
//#define  OSD3coor                       __AM7X_HWREG(DeBaseadd+0x0074)
//#define  OSD3size                       __AM7X_HWREG(DeBaseadd+0x0078)
#define  Dithering                      __AM7X_HWREG(DeBaseadd+0x007c)
#define  Pallet0Ram                     __AM7X_HWREG(DeBaseadd+0x0080)
//#define  Pallet_tcolor                  __AM7X_HWREG(DeBaseadd+0x0084)
//#define  Pallet0_ADD                    __AM7X_HWREG(DeBaseadd+0x0088)
//#define  Pallet1Ram                     __AM7X_HWREG(DeBaseadd+0x008c)
//#define CursorCoor		(DeBaseadd+0x0090)
//#define CursorClr		(DeBaseadd+0x0094)
#define  GammaRAM                       __AM7X_HWREG(DeBaseadd+0x0098)
#define  CSCcef1                        __AM7X_HWREG(DeBaseadd+0x009c)
#define  CSCcef2                        __AM7X_HWREG(DeBaseadd+0x00a0)
#define  CSCcef3                        __AM7X_HWREG(DeBaseadd+0x00a4)
#define  Color_coef                     __AM7X_HWREG(DeBaseadd+0x00a8)
#define  DE_Offset                      __AM7X_HWREG(DeBaseadd+0x00ac)
#define  Contrast                       __AM7X_HWREG(DeBaseadd+0x00b0)
#define  Input_size                     __AM7X_HWREG(DeBaseadd+0x00b4)
#define  Output_size                    __AM7X_HWREG(DeBaseadd+0x00b8)
#define  Scalar_step                    __AM7X_HWREG(DeBaseadd+0x00bc)
#define  cpumd_en                       __AM7X_HWREG(DeBaseadd+0x00c0)
#define  cpumd_tc                       __AM7X_HWREG(DeBaseadd+0x00c4)
#define  cpumd_lt                       __AM7X_HWREG(DeBaseadd+0x00c8)
#define  DE_INT                         __AM7X_HWREG(DeBaseadd+0x00cc)
#define  DE_INT_CFG                     __AM7X_HWREG(DeBaseadd+0x00d0)
#define  DE_INT_BIST                    __AM7X_HWREG(DeBaseadd+0x00d4)
#define  DE_DLTI                        __AM7X_HWREG(DeBaseadd+0x00d8)
#define  DE_DCTI                        __AM7X_HWREG(DeBaseadd+0x00dc)
#define  PEAK_Coring                    __AM7X_HWREG(DeBaseadd+0x00e0)
#define  DB_WR                          __AM7X_HWREG(DeBaseadd+0x00e4)
#define  BACK_COLOR                     __AM7X_HWREG(DeBaseadd+0x00e8)
#define  BLE                            __AM7X_HWREG(DeBaseadd+0x00ec)
#define  WLE                            __AM7X_HWREG(DeBaseadd+0x00f0)
#define  DE_OFFSET                      __AM7X_HWREG(DeBaseadd+0x00f4)
#define  Histogram_BaseAdd              __AM7X_HWREG(DeBaseadd+0x0100)

/*--- -----------------TCON GPOs Registers Description-------------------*/
#define  TCON_BASE                      0xb00f0000/*phy_addr:0x100f0000*/
#define  VIDEO_SIZE                     __AM7X_HWREG(TCON_BASE+0x00ec)
#define  TCON_CON                       __AM7X_HWREG(TCON_BASE+0x002c)
#define  GPO0_CON                       __AM7X_HWREG(TCON_BASE+0x0030)
#define  GPO0_HP                        __AM7X_HWREG(TCON_BASE+0x0034)
#define  GPO0_VP                        __AM7X_HWREG(TCON_BASE+0x0038)

#define  GPO1_CON                       __AM7X_HWREG(TCON_BASE+0x0040)
#define  GPO1_HP                        __AM7X_HWREG(TCON_BASE+0x0044)
#define  GPO1_VP                        __AM7X_HWREG(TCON_BASE+0x0048)

#define  GPO2_CON                       __AM7X_HWREG(TCON_BASE+0x0050)
#define  GPO2_HP                        __AM7X_HWREG(TCON_BASE+0x0054)
#define  GPO2_VP                        __AM7X_HWREG(TCON_BASE+0x0058)

#define  GPO3_CON                       __AM7X_HWREG(TCON_BASE+0x0060)
#define  GPO3_HP                        __AM7X_HWREG(TCON_BASE+0x0064)
#define  GPO3_VP                        __AM7X_HWREG(TCON_BASE+0x0068)

#define  GPO4_CON                       __AM7X_HWREG(TCON_BASE+0x0070)
#define  GPO4_HP                        __AM7X_HWREG(TCON_BASE+0x0074)
#define  GPO4_VP                        __AM7X_HWREG(TCON_BASE+0x0078)

#define  GPO5_CON                       __AM7X_HWREG(TCON_BASE+0x0080)
#define  GPO5_HP                        __AM7X_HWREG(TCON_BASE+0x0084)
#define  GPO5_VP                        __AM7X_HWREG(TCON_BASE+0x0088)

#define  GPO6_CON                       __AM7X_HWREG(TCON_BASE+0x0090)
#define  GPO6_HP                        __AM7X_HWREG(TCON_BASE+0x0094)
#define  GPO6_VP                        __AM7X_HWREG(TCON_BASE+0x0098)

#define  GPO7_CON                       __AM7X_HWREG(TCON_BASE+0x00a0)
#define  GPO7_HP                        __AM7X_HWREG(TCON_BASE+0x00a4)
#define  GPO7_VP                        __AM7X_HWREG(TCON_BASE+0x00a8)

/*--------------------- LCD Controller Registers ------------------*/
#define  LCD_BASE                       0xb00f0000/*phy_addr:0x100f0000*/
#define  LCD_CTL                        __AM7X_HWREG(LCD_BASE+0x0000)
#define  LCD_RGBCTL                     __AM7X_HWREG(LCD_BASE+0x0004)
#define  LCD_SIZE                       __AM7X_HWREG(LCD_BASE+0x0008)
#define  LCD_LSP                        __AM7X_HWREG(LCD_BASE+0x000C)
#define  LCD_HTIMING                    __AM7X_HWREG(LCD_BASE+0x0010)
#define  LCD_VTIMING                    __AM7X_HWREG(LCD_BASE+0x0014)
#define  LCD_CLR                        __AM7X_HWREG(LCD_BASE+0x0018)
#define  LCD_PWM                        __AM7X_HWREG(LCD_BASE+0x001C)
#define  LCD_CPUCTL                     __AM7X_HWREG(LCD_BASE+0x0020)
#define  LCD_CPUCOM                     __AM7X_HWREG(LCD_BASE+0x0024)
#define  LCD_DAC_CTL                    __AM7X_HWREG(LCD_BASE+0x0028)

#define  LCD_PWM0                       __AM7X_HWREG(LCD_BASE+0x00f0)
#define  LCD_PWM1                       __AM7X_HWREG(LCD_BASE+0x00f4)

/*--------------------- BTVEI Controller Registers ------------------*/
#define  BTVE_BASE                      0xb00d0000/*phy_addr:0x100d0000*/
//Video Encoder  
#define  BTVE_CTL                       __AM7X_HWREG(BTVE_BASE+0x0000)
#define  BTVE_SIZE                      __AM7X_HWREG(BTVE_BASE+0x0004)
#define  BTVE_HS                        __AM7X_HWREG(BTVE_BASE+0x0008)
#define  BTVE_HDE                       __AM7X_HWREG(BTVE_BASE+0x000c)
#define  BTVE_VSODP                     __AM7X_HWREG(BTVE_BASE+0x0010)
#define  BTVE_VSEVEN                    __AM7X_HWREG(BTVE_BASE+0x0014)
#define  BTVE_VDEODD                    __AM7X_HWREG(BTVE_BASE+0x0018)
#define  BTVE_VDEEVEP                   __AM7X_HWREG(BTVE_BASE+0x001c)
#define  BTVE_FT                        __AM7X_HWREG(BTVE_BASE+0x0020)
#define  BTVE_LSP                       __AM7X_HWREG(BTVE_BASE+0x0024)

#define  BT_IVETEST                     __AM7X_HWREG(BTVE_BASE+0x50)
#define  BT_IVEVERIFY                   __AM7X_HWREG(BTVE_BASE+0x54)
#define  BT_IVEROM                      __AM7X_HWREG(BTVE_BASE+0x58)

/*---------------------BTVD Controller Registers---------------------*/
#define  BTVD_BASE                      0xb00d8000
#define  BTVD_CTL                       __AM7X_HWREG(BTVD_BASE+0x0000)
#define  BTVD_FIFOODAT                  __AM7X_HWREG(BTVD_BASE+0x0004)
#define  BTVD_HDE                       __AM7X_HWREG(BTVD_BASE+0x0008)
#define  BTVD_VPEODD                    __AM7X_HWREG(BTVD_BASE+0x000C)
#define  BTVD_VPEEVCS                   __AM7X_HWREG(BTVD_BASE+0x0010)
#define  BTVD_FIFOCTL                   __AM7X_HWREG(BTVD_BASE+0x0014)
#define  BTVD_IRQ                       __AM7X_HWREG(BTVD_BASE+0x0018)
#define  BTVD_XY                        __AM7X_HWREG(BTVD_BASE+0x001c)

/*--------------------- 2D Controller Registers ------------------*/
#define  _2DBaseAddress                 0xb0048000
#define  _2D_CTRL                       __AM7X_HWREG(_2DBaseAddress+0x0000)
#define  _2D_BLEND_RATE_CONF            __AM7X_HWREG(_2DBaseAddress+0x0004)
#define  _2D_COLOR_KEY_CONF             __AM7X_HWREG(_2DBaseAddress+0x0008)
#define  _2D_DEFAULT_COLOR_CONF         __AM7X_HWREG(_2DBaseAddress+0x000c)
#define  _2D_STRIDE_CONF                __AM7X_HWREG(_2DBaseAddress+0x0010)
#define  _2D_SCAN_AREA_CONF             __AM7X_HWREG(_2DBaseAddress+0x0014)
#define  _2D_DST_HEAD_ADDR_CONF         __AM7X_HWREG(_2DBaseAddress+0x0018)
#define  _2D_SRC_HEAD_ADDR              __AM7X_HWREG(_2DBaseAddress+0x001c)
#define  _2D_HEAD_POINT_ROTATE_X_COORDI __AM7X_HWREG(_2DBaseAddress+0x0020)
#define  _2D_HEAD_POINT_ROTATE_Y_COORDI __AM7X_HWREG(_2DBaseAddress+0x0024)
#define  _2D_SRC_CENTER_ADDR            __AM7X_HWREG(_2DBaseAddress+0x0028)
#define  _2D_SRC_BOUNDARY_X_COORDI      __AM7X_HWREG(_2DBaseAddress+0x002c)
#define  _2D_SRC_BOUNDARY_Y_COORDI      __AM7X_HWREG(_2DBaseAddress+0x0030)
#define  _2D_COEFF_A                    __AM7X_HWREG(_2DBaseAddress+0x0034)
#define  _2D_COEFF_B                    __AM7X_HWREG(_2DBaseAddress+0x0038)
#define  _2D_COEFF_C                    __AM7X_HWREG(_2DBaseAddress+0x003c)
#define  _2D_COEFF_D                    __AM7X_HWREG(_2DBaseAddress+0x0040)
#define  _2D_POINT_PAIR_NUM             __AM7X_HWREG(_2DBaseAddress+0x0044)
#define  _2D_POINT_PAIR_NUM_REMAIN      __AM7X_HWREG(_2DBaseAddress+0x0048)
#define  _2D_POINT_PAIR_ADDR            __AM7X_HWREG(_2DBaseAddress+0x004c)
#define  _2D_FORMAT_TRANS_CONF1         __AM7X_HWREG(_2DBaseAddress+0x0050)
#define  _2D_FORMAT_TRANS_CONF2         __AM7X_HWREG(_2DBaseAddress+0x0054)
#define  _2D_FORMAT_TRANS_CONF3         __AM7X_HWREG(_2DBaseAddress+0x0058)
#define  _2D_FORMAT_TRANS_CONF4         __AM7X_HWREG(_2DBaseAddress+0x005c)
#define  _2D_FORMAT_TRANS_CONF5         __AM7X_HWREG(_2DBaseAddress+0x0060)
#define  _2D_FORMAT_TRANS_CONF6         __AM7X_HWREG(_2DBaseAddress+0x0064)
#define  _2D_FORMAT_TRANS_CONF7         __AM7X_HWREG(_2DBaseAddress+0x0068)
#define  _2D_FORMAT_TRANS_CONF8         __AM7X_HWREG(_2DBaseAddress+0x006C)
#define  _2D_FORMAT_TRANS_ADD_PARAM1    __AM7X_HWREG(_2DBaseAddress+0x0070)
#define  _2D_FORMAT_TRANS_ADD_PARAM2    __AM7X_HWREG(_2DBaseAddress+0x0074)
#define  _2D_FORMAT_TRANS_CLIP_PARAM1   __AM7X_HWREG(_2DBaseAddress+0x0078)
#define  _2D_FORMAT_TRANS_CLIP_PARAM2   __AM7X_HWREG(_2DBaseAddress+0x007C)
#define  _2D_ZBUF_STRIDE_CONF           __AM7X_HWREG(_2DBaseAddress+0x0080)
#define  _2D_ZBUF_HEAD_ADDR_CONF        __AM7X_HWREG(_2DBaseAddress+0x0084)

/*--------------------- LVDS Interface Registers ------------------*/
#define  LVDSBaseAddress                0xb00f0000
#define  LVDS_CTL                       __AM7X_HWREG(LVDSBaseAddress+0x00f4)
#define  RSDS_CTL                       __AM7X_HWREG(LVDSBaseAddress+0x00f8)
#define  LVDS_ANA_CTL1                  __AM7X_HWREG(LVDSBaseAddress+0x00fc)
#define  LVDS_ANA_CTL2                  __AM7X_HWREG(LVDSBaseAddress+0x0100)

#endif  /*_ACTIONS_REGISTER_1203_H_*/
