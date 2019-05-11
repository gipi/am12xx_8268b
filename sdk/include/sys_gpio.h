#ifndef _SYS_GPIO_H_
#define _SYS_GPIO_H_
/**
*@file sys_gpio.h
*@brief this head file describes the GPIO operations for Actions-micro IC
*
*@author yekai
*@date 2010-04-14
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup gpio_lib_s
*@{
*/

/**GPIO device path*/
#define GPIO_DEV_PATH			"/dev/gpio"
#define AM_REG_DEV_PATH		"/dev/amreg"

/**
*@name command for gpio operation, not used yet
*@{
*/
#define GPIO_PAD_INIT			1
#define GPIO_BITS_READ			2
#define GPIO_BITS_WRITE_AND		3
#define GPIO_BITS_WRITE_OR		4


/**
* For dynamic adjust the bus priority.
*/
#define SOC_GET_PRIORITY    5
#define SOC_SET_PRIORITY    6
struct am_bus_priority
{
	unsigned int special_dma;  // the special dma bus
	unsigned int mac;          // the MAC
	unsigned int ahb_bus;      // the bus dma
	unsigned int graph_2d;     // the 2D
	unsigned int de;           // the de
	unsigned int dac;          // the dac
	unsigned int vdc;          // the video decoder
	unsigned int axi_bus;      // the CPU
	unsigned int vec;          // the video encoder
};

/**
* @brief Distinguish Chips.
*/
enum{
	CHIP_ID_AM8250=0,
	CHIP_ID_AM8251=1,
	CHIP_ID_AM8252=2,
	CHIP_ID_ERROR
};
enum{
	CHIP_ID_AM8258B=0,
	CHIP_ID_AM8258L=1,
	CHIP_ID_AM8258D=2,
	CHIP_ID_AM8258N=3,
	CHIP_ID_AM8258ERROR
};
extern int am_soc_get_chip_id();


/**
*@}
*/

/**
*@brief set the specified gpio 1 or 0
*
*@param[in] num	: gpio index, based on actions-micro spec.
*@param[in] value	: 1 or 0
*@return
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	the related pin must be configed as a standard gpio first (this premise should be done by system)
*	and invoking this function will cause the corresponding gpio to be output mode
*/
 int set_gpio(long num, char value);

/**
*@brief get the specified gpio status
*
*@param[in] num	: gpio index, based on actions-micro spec.
*@param[in] value	: pointer to the varient, to which the gpio status should be returned
*@return
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	the related pin must be configed as a standard gpio first (this premise should be done by system)
*	and invoking this function will cause the corresponding gpio to be input mode
*/
 int get_gpio(long num, char *value);




//int reg_writel(unsigned int reg, unsigned int value);
//unsigned int reg_readl(unsigned int reg);
int reg_writel(unsigned long reg, unsigned long value);
unsigned long reg_readl(unsigned long reg);

void RegBitSet(int val,int reg,int msb,int lsb);
unsigned int RegBitRead(int reg,int msb,int lsb);




/**
 *@}
 */

#ifdef __cplusplus
}
#endif
 	
#endif //_SYS_GPIO_H_
