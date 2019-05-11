#ifndef __LINUX_SYSCFG_H
#define __LINUX_SYSCFG_H

/**
*@file sys_cfg.h
*@brief this head file represents the system informations which can be configed by case
*@access these infomations should infork the special API.(Ex. get_sys_info)
*
*
*@author yekai
*@date  2010-03-02
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup syscfg_lib_s
*@{
*/

/**debug mode config*/
#define SYS_DEBUG_MODE
/**mouse enable config*/
#define SYS_MOUSE_USED			0


/**
*@name global path definition
*@{
*/
#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"
#define AM_DYNAMIC_LIB_DIR 	"/am7x/lib/"
#define AM_CASE_APP_DIR        	"/am7x/bin"
#define AM_CASE_SC_DIR         	"/am7x/case/scripts"
#define AM_CASE_DAT_DIR        	"/am7x/case/data"
#define AM_CASE_SWF_DIR        "/am7x/case/data"
/**
*@}
*/

/**
*@name Generic definitions and configurations.
*@{
*/
#define UART_BAUDRATE  115200

#define AM_CHIP_7531  0x0301
#define AM_CHIP_7331  0x0302

#define AM_CHIP_7555  0x1101 //216Pin
#define AM_CHIP_7553  0x1102 //176 PIN
#define AM_CHIP_7551  0x1103 // 144PIN
#define AM_CHIP_7656  0x1104  //176PIN
#define AM_CHIP_7655  0x1105  // 144 PIn
#define AM_CHIP_7657  0x1106   //128 PIN
#define AM_CHIP_7051  0x1107
#define AM_CHIP_8201  0x1108

///zhoujian
#define AM_CHIP_5101  0x1109
#define AM_CHIP_8250  0x110a




#define INITCFG_UART1      (0 << 2)
#define INITCFG_UART2      (1 << 2)
#define INITCFG_UART_MASK  0x4

#define INITCFG_SDR         0
#define INITCFG_DDR         2
#define INITCFG_SDRAM_MASK  0x3

#define INITCFG_1CE   (1<<3)
#define INITCFG_2CE   (2<<3)

#define INITCFG_CHIPTYPE_SHIFT  16

#define AM7X_UART1     0
#define AM7X_UART2     1
/**
*@}
*/

#if ! defined(__LANGUAGE_ASSEMBLY)

#include "am_types.h"

/**
*@brief chip info struct for am7x
*   - never reorder this field
*/
struct am7x_chipinfo
{
	INT16U  dramtype       : 2; 		/**<ddr type info*/
	INT16U  uart           : 1;			/**<uart cfg*/
	INT16U  nf_max_chip_num: 2;	/**<nand flash cs pin number*/
	INT16U  reserved       : 11;		/**<reserved*/
	INT16U  chiptype;				/**<chip type of am ic*/
};

/**
*@brief SCREEN output cfg info
*/
typedef struct _screen_output_info
{
	unsigned int	screen_output_width;	/**<HDMI/CVBS/YPbPr/VGA output width*/
	unsigned int	screen_output_height;	/**<HDMI/CVBS/YPbPr/VGA output height*/
	int		screen_output_mode;			/**<HDMI/CVBS/YPbPr/VGA output mode*/
	int		screen_output_true;				/**<HDMI/CVBS/YPbPr/VGA enable auto mode switch*/
}screen_output_param;

extern struct  am7x_chipinfo am7x_chipinfo;

/**
*@brief get system partition info
*
*@param[in] NULL
*@return partiotion table pointer
*/
extern char   *prom_getAM7X_PARTS(void);
extern screen_output_param screen_output_data;

/**
*@brief get cpu type
*
*@param[in] NULL
*@return cpu type
*/
static inline unsigned int am7x_get_chiptype(void)
{	return am7x_chipinfo.chiptype;  }

/**
*@brief get dram type
*
*@param[in] NULL
*@return dram type
*/
static inline unsigned int am7x_get_dramtype(void)
{	return am7x_chipinfo.dramtype;  }

/**
*@brief get uart group number
*
*@param[in] NULL
*@return uart group number
*/
static inline unsigned int am7x_get_uart(void)
{	return am7x_chipinfo.uart;  }

/**
*@brief get nand flash cs number
*
*@param[in] NULL
*@return nand flash cs number
*/
static inline unsigned int am7x_get_max_chip_num(void)
{	return am7x_chipinfo.nf_max_chip_num;  }

/**
*@brief get system chip info,include all above functions
*
*@param[in] NULL
*@return system chip info pointer
*/
static inline unsigned int am7x_get_chip_info(void)
{	void *ptr = &am7x_chipinfo; return *(unsigned int *)ptr; }

/**
*@name macro for chip info
*@{
*/
#define CHIP_TYPE         am7x_get_chiptype()
#define USE_UART          am7x_get_uart()
#define MAX_CHIP_NUM      am7x_get_max_chip_num()
#define CFG_CHIPINFO_VAL  am7x_get_chip_info()
/**
*@}
*/

/**
*@brief struct for system gpio config
*/
struct gpio_cfg{
	/**
	* From generations to generations, the GPIO may be changed by the 
	* hardware, so the struct will change accordingly.
	*/
#if (CONFIG_AM_CHIP_ID == 1213) || (AM_CHIP_ID == 1213)
	INT8U  bl_pwm;		/**<io for pwm backlight*/
	INT8U  backlignt;		/**<io for backlight*/
	INT8U  amplify;		/**<io for amplifier*/
	INT8U  sys_power;	/**<io for system power*/
	INT8U  tft_power;		/**<io for TFT power*/
	INT8U  usb_det;		/**<io for usb detect*/
	INT8U  usb_det1;		/**<io for usb1 detect*/
	INT8U  rotate_sensor;	/**<io for rotate sensor*/
	INT8U  buzzer;		/**<io for buzzer*/
	INT8U  reset_lan;		/**<io for LAN reset*/
	INT8U  sd_power;		/**<io for SD power*/
	INT8U  sd_det;		/**<io for SD detect*/
	INT8U  sd_wp;		/**<io for SD write protect*/
	INT8U  charge_io_valid_status; /** <io status for charge ic when charging > */  
	INT8U  use_pmu;			/** <use board pmu ic or not> */
	INT8U  power_key;	/**<io for power key*/
	INT8U  board_key[15];	/**<io for board key*/
#else
	INT8U  bl_pwm;		/**<io for pwm backlight*/
	INT8U  backlignt;		/**<io for backlight*/
	INT8U  amplify;		/**<io for amplifier*/
	INT8U  sys_power;	/**<io for system power*/
	INT8U  tft_power;		/**<io for TFT power*/
	INT8U  usb_det;		/**<io for usb detect*/
	INT8U  rotate_sensor;	/**<io for rotate sensor*/
	INT8U  buzzer;		/**<io for buzzer*/
	INT8U  sd_power;		/**<io for SD power*/
	INT8U  sd_det;		/**<io for SD detect*/
	INT8U  sd_wp;		/**<io for SD write protect*/
	INT8U  xd_det;		/**<io for XD detect*/
	INT8U  xd_power;		/**<io for XD power*/
	INT8U  cf_det;		/**<io for CF detect*/
	INT8U  cf_power;		/**<io for CF power*/
	INT8U  cf_oe;			/**<io for CF work mode*/
	INT8U  charge_io;		/** <io for charge ic > */
	INT8U  use_pmu;			/** <use board pmu ic or not> */
	INT8U  charge_io_valid_status; /** <io status for charge ic when charging > */  
	INT8U  power_key;	/**<io for power key*/
	INT8U  board_key[15];	/**<io for board key*/
#endif
};

/**
*@brief struct for system config info
*/
struct sys_cfg{
	struct gpio_cfg sys_gpio_cfg;	/**<system gpio config*/
};


#endif

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_SYSCFG_H */

