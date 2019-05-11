#ifndef __SYS_PMU_H__
#define __SYS_PMU_H__
/**
*@file sys_pmu.h
*@brief This file describs system pmu info
*
*@author yekai
*@date 2010-09-07
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup pmu_lib_s
*@{
*/

/**arg size: 4*len(bytes)*/
#define AM_PMU_ARG_LEN		5
/**flag of standby in DC*/
#define IS_DC_MODE			0x10000


/**
*@al1601 operation
*@{
*/
#define READ_REG		0x1
#define WRITE_REG		0x2
/**
*@}
*/

/**
*@brief struct of al1601 data between user space and kernel space
*/
struct al1601_data{
	unsigned char reg;
	unsigned char val;
};


/**
*@name mode for standby
*@{
*/
#define RTC_DC_MODE		    0x10001	
#define EXT_DC_MODE		    0x10002	//gpio key
#define KEYE_DC_MODE		0x10004
#define TP_DC_MODE			0x10008
#define IRE_DC_MODE		    0x10010
#define REBOOT_DC_MODE	    0x01000
#define LAN_DC_MODE	        0x10020
#define KEY_NDC_MODE		0x20001
/**
*@}
*/

/**
*@name mode for power on/off
*@{
*/
#define PM_GPIO_MODE		0x30001
#define PM_SPEC_MODE		0x30002
#define PM_GEN_MODE		0x30003
/**
*@}
*/

/**
*@name clock for change pll
*@{
*/
#define DDR_LOW_PLL			196
#define DDR_HIGH_PLL		392
/**
*@}
*/


/**
*@name PMU command
*@{
*/
#define AM_PMU_SET			0x10
#define AM_PMU_GET			0x20
#define AM_PMU_VALID		0x30
#define AM_PMU_POFF		0x40

#define AM_PMU_PLOW		0x51
#define AM_PMU_PHIGH	0x52
#define AM_PMU_GETCD	0x53
/**
*@}
*/

#ifndef __ASSEMBLY__
/**
*@brief struct of power management arg
*/
struct am_pm_arg{
	unsigned int wakeup_mode;				/**<wake up mode for standby*/
	unsigned int ext_param[AM_PMU_ARG_LEN];		/**<standby params*/
	unsigned int rtc_param[AM_PMU_ARG_LEN];
	unsigned int key_param[AM_PMU_ARG_LEN];
	unsigned int ire_param[AM_PMU_ARG_LEN];	
	unsigned int sram_param[AM_PMU_ARG_LEN];
	unsigned int lan_param[AM_PMU_ARG_LEN];
};

/**
*@brief struct of power management info
*/
struct am_pm_info{
	void (*sram_func)(struct am_pm_arg *,unsigned int,unsigned int);		/**<standby main function run in sram*/
	void  *func_addr;							/**<main function address in dram*/
	unsigned int func_size;						/**<function size in byte*/
	struct am_pm_arg arg;						/**<args used by main function*/
};

/**<date format used by PMU*/
#define AM_PMU_MK_DATE(y,m,d)	(((y%1000)<<16) + (m<<8) + d)
/**<time format used by PMU*/
#define AM_PMU_MK_TIME(h,m,s)	((h<<16) + (m<<8) + s)

/**
*@brief struct of change ddr pll info
*/
struct am_chpll_arg {
	int   (*sram_entry)(unsigned int, unsigned int);	/**<standby main function run in sram*/
	void  *code_start;							    /**<main function address in dram*/
	unsigned int code_size;						    /**<function size in byte*/
	unsigned int clock;
};





/**
 *@}
 */
#endif /* __ASSEMBLY__ */

#ifdef __cplusplus
}
#endif

#endif /*__SYS_PMU_H__*/
