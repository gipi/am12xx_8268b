#ifndef _ACTIONS_PM_H_
#define _ACTIONS_PM_H_
/**
*@file am7x_pm.h
*
*@author yekai
*@date 2010-08-24
*@version 0.1
*/
#include <actions_regs.h>
#include <actions_io.h>

/**
*@addtogroup power_driver_s
*@{
*/

/**max device number of pmu*/
#define PMU_MAX_DEVS		2	
/**start address for function run in sram*/
#define SRAM_FUNC_START	0xb4040000
/**stack point address in sram*/
#if CONFIG_AM_CHIP_ID==1203
#define SP_IN_SRAM		0xb4043800
#elif CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID ==1220 || CONFIG_AM_CHIP_ID == 1213
#define SP_IN_SRAM		0xb4047a00
#endif

/**
*@name dynamic ddr clock value
*@{
*/
#define DDR_LOW_CLOCK		0x8444c61
#define DDR_LOW_TIMING		0x06494145
#define DDR_LOW_RFC		0x240

#define DDR_HIGH_CLOCK		0x8444cc5
#define DDR_HIGH_TIMING		0x0ed3014C
#define DDR_HIGH_RFC		0x4e0
/**
*@}
*/

#ifndef __ASSEMBLY__
/**
*@ brief change sp and pc to uncacheable address
*
*this functions is private for pmu only 
*@param[in] NULL
*@return NULL
*@note
*	-This function is used to change cacheable space to uncacheable
*	-Remember that it only chages sp and pc so if you want the follow program keep in uncacheable
*	-do not invoke any function that jumps with absolute cacheable address unless cache is forbidden.
*/
void switch_to_uncache(void);

/**
*@ brief disable cache function
*
*this functions is private for pmu only 
*@param[in] NULL
*@return NULL
*@note
*	-This function is used to disable cache configuration in KSEG0 
*/
void mips24kec_cache_close(void);

/**
*@ brief enable cache function
*
*@param[in] NULL
*@return NULL
*@note
*	-This function is used to enable cache configuration in KSEG0 
*/
void mips24kec_cache_init(void);

/**
*@ brief flush all cache(dcache and icache)
*
*@param[in] NULL
*@return NULL
*@note
*	-This function is used to flush all cache in local cpu
*/
void am7x_flush_cache_all(void);

/**
*@ brief save stack pointer
*
*@param[in] NULL
*@return old sp
*/
int save_sp(void);

/**
*@ brief restore stack pointer
*
*@param[in] sp	: the old saved sp
*@return NULL
*/
void restore_sp(int sp);

/**
*@brief change ddr clock when system running
*
*@param[in] ddr_clk	: clock value filled in DDRPLL
*@param[in] clk_dly	: clock delay value filled in CLK_DLY 
*@return NULL
*
*@note: This function should be invoked in critical area!
*/
void change_ddr_clk(unsigned int ddr_clk, unsigned int clk_dly);

#ifdef USE_INTERNAL_PMU
static inline void am_set_vcc(unsigned int val)
{
	am7x_writel((am7x_readl(PMU_DHVLDO_CON)&~0x7)|val,PMU_DHVLDO_CON);
}

static inline void am_set_vdd(unsigned int val)
{
	am7x_writel((am7x_readl(PMU_DLVLDO_CON)&~0x7)|val,PMU_DLVLDO_CON);
}

static inline void am_set_avcc(unsigned int val)
{
	am7x_writel(val,PMU_AHVLDO);
}

static inline void am_set_avdd(unsigned int val)
{
	am7x_writel(val,PMU_ALVLDO);
}

static inline unsigned int am_get_vcc(void)
{
	return (am7x_readl(PMU_DHVLDO_CON)&0x7);
}

static inline unsigned int am_get_vdd(void)
{
	return (am7x_readl(PMU_DLVLDO_CON)&0x7);
}

static inline unsigned int am_get_avcc(void)
{
	return (am7x_readl(PMU_AHVLDO)&0x7);
}

static inline unsigned int am_get_avdd(void)
{
	return (am7x_readl(PMU_ALVLDO)&0x7);
}
#endif

/** 
*@name vdd value to ddr delay
*@{
*/
#define VDD_500_DLY				1
#define VDD_1000_DLY			2
/**
*@}
*/

typedef enum{
	AM_VCC,
	AM_VDD,
	AM_AVCC,
	AM_AVDD,
	AM_DRAM,
	AM_VBUS,
	AM_VBUS_CON,
	AM_ALDO,
	AM_AUDIO_CON,
	AM_CHARGE_STA,
	AM_BAT_VOL,
	AM_BAT_TEMP,
}am_pm_type;

/**
*@name pm ioctl cmd for kernel mode
*@{
*/
#define AM_PM_SET				0x1
#define AM_PM_GET				0x2

#define AM_PM_NEEDS_NO_ARG	0x1000
#define AM_PM_ENABLE			0x1001
#define AM_PM_DISABLE			0x1002
#define AM_SYS_TO_VBUS_ON		0x1003
#define AM_SYS_TO_VBUS_OFF	0x1004
#define AM_ENABLE_VBUS			0x1005
#define AM_DISABLE_VBUS		0x1006
#define AM_ENABLE_AUD_AMP		0x1007
#define AM_DISABLE_AUD_AMP	0x1008

/**
*@}
*/

struct board_pm_ops{
	int (*board_pm_ioctl)(am_pm_type  id, unsigned int cmd, void  *arg);
};

/**
*@brief set pm opertions of pmu chips on board
*
*@param[in] func	: pm opertion function address of onboard chip
*@retval 0		: success
*@retval !0		: standard error no
*/
int am_set_pm_ctl(void* func);

/**
*@brief pmu operation function
*
*@param[in] id	: power type id specified in am_pm_type
*@param[in] cmd	: pmu command defined in this head file
*@param[in] arg	: read/write data buffer in which the first byte should be reserved!
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	-the first byte of the buffer pointed by arg will be used by driver!
*	-so usr should not use this byte and the buffer length can not less than two!
*	-read data will be stored in arg buffer from the second byte.
*/
int am_pm_ioctl(am_pm_type  id, unsigned int cmd, u8  *arg);

/**
*@ brief change DDR data delay cycle
*
*@param[in] new_vdd	: vdd value which will change to
*@param[in] old_vdd	: original vdd value
*@return NULL
*@note
*	-DDR data delay cycle must change if vdd value is modified 
*	-so this function must be invoked before vdd changed.
*	-However do not call this function unless you must.
*/
void am_change_ddr_delay(unsigned int new_vdd,unsigned int old_vdd);
#endif /* __ASSEMBLY__ */
/**
 *@}
 */

#endif /*_ACTIONS_PM_H_S*/