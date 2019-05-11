#ifndef _ACTIONS_IO_H_
#define _ACTIONS_IO_H_
/**
*@file actions_io.h
*
*@brief This file provides register access for actions micro IC
*
*@author yekai
*@date 2010-07-22
*@version 0.1
*/

#include <am_types.h>
#include <actions_regs.h>

#ifndef CONFIG_AM_CHIP_ID
	#error "AM_CHIP_ID is not defined!!"
#endif

/**
*@addtogroup io_lib_s
*@{
*/

#ifndef _LANGUAGE_ASSEMBLY

/**
*@brief write a register in 8bits format
*
*@param[in] val	: value written to the register
*@param[in] reg	: register address
*@return NULL
*/
static inline void act_writeb(INT8U val, AM7X_HWREG reg)
{
    *(volatile INT8U *)(reg) = val;
}

/**
*@brief write a register in 16bits format
*
*@param[in] val	: value written to the register
*@param[in] reg	: register address
*@return NULL
*/
static inline void act_writew(INT16U val, AM7X_HWREG reg)
{
    *(volatile INT16U *)(reg) = val;
}

/**
*@brief write a register in 32bits format
*
*@param[in] val	: value written to the register
*@param[in] reg	: register address
*@return NULL
*/
static inline void act_writel(INT32U val, AM7X_HWREG reg)
{
    *(volatile INT32U *)(reg) = val;
}

/**
*@brief "&" the register with val in 32bits format
*
*@param[in] val	: value "&" with the register
*@param[in] reg	: register address
*@return NULL
*/
static inline 
void act_andl(INT32U val, AM7X_HWREG reg)
{
    *(volatile INT32U *)(reg) &= val;
}

/**
*@brief "|" the register with val in 32bits format
*
*@param[in] val	: value "|" with the register
*@param[in] reg	: register address
*@return NULL
*/
static inline 
void act_orl(INT32U val, AM7X_HWREG reg)
{
    *(volatile INT32U *)(reg) |= val;
}

/**
*@brief read a register in 8bits format
*
*@param[in] reg	: register address
*@return register value in 8bits format
*/
static inline INT8U act_readb(AM7X_HWREG port)
{
    return (*(volatile INT8U *)port);
}

/**
*@brief read a register in 16bits format
*
*@param[in] reg	: register address
*@return register value in 16bits format
*/
static inline INT16U act_readw(AM7X_HWREG port)
{
    return (*(volatile INT16U *)port);
}

/**
*@brief read a register in 32bits format
*
*@param[in] reg	: register address
*@return register value in 32bits format
*/
static inline INT32U act_readl(AM7X_HWREG port)
{
    return (*(volatile INT32U *)port);
}

/**
*@brief set the specified bit of the register
*
*@param[in] reg	: register address
*@param[in] bit	: bit offset in the register 
*@return NULL
*/
static inline void act_set(AM7X_HWREG reg, int bit)
{
  volatile int * p = (volatile int *)reg;
  *p |= (1<<bit);
}

/**
*@brief clear the specified bit of the register
*
*@param[in] reg	: register address
*@param[in] bit	: bit offset in the register 
*@return NULL
*/
static inline void act_clear(AM7X_HWREG reg, int bit)
{
  volatile int * p = (volatile int *)reg;
  *p &= ~(1<<bit);
}

/**
*@brief test the specified bit of the register whether it is 1 or not
*
*@param[in] port	: register address
*@param[in] bit	: bit offset in the register 
*@return NULL
*/
static inline INT32U act_test(AM7X_HWREG port, int bit)
{
    return ((*(volatile INT32U *)port)&(1<<bit));
}

/**
*@brief write a register in 32bits format
*
*@param[in] val	: value written to the register
*@param[in] reg	: register address
*@return NULL
*/
static inline void am7x_writel(INT32U val, AM7X_HWREG reg)
{
    *(volatile INT32U *)(reg) = val;
}

/**
*@brief read a register in 32bits format
*
*@param[in] reg	: register address
*@return register value in 32bits format
*/
static inline INT32U am7x_readl(AM7X_HWREG port)
{
    return (*(volatile INT32U *)port);
}


/**********************************************************************
RTC op
************************************************************************/
/**
*@brief read a rtc register
*
*@param[in] reg	: rtc register address
*@return register value in 32bits format
*/
static inline INT32U  RTC_READ(INT32U reg)
{
	return *(volatile INT32U *)(reg);
}

/**
*@brief write a rtc register in 32bits format
*
*@param[in] val	: value written to the register
*@param[in] reg	: rtc register address
*@return NULL
*/
static inline void RTC_WRITE(INT32U val, INT32U reg)
{
	*(volatile INT32U *)(0xb0018014 + 0x60) = 0x80000000; // Enable writing RTC register
	*(volatile INT32U *)(reg + 0x60) = val;
}

/****************************************************************************
IRQ Pending clear op
*****************************************************************************/
/**
*@brief clear irq pending bits in a register
*
*@param[in] val	: value written to the register
*@param[in] reg	: register address
*@return NULL
*@note
*	write 1 to the corresponding bit will clear it, not 0.
*/
static inline void IRQ_CLEAR_PENDING(INT32U val, INT32U reg)
{
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	int i=0;
	for(i=0;i<8;i++)
#endif
	am7x_writel(val,reg);
}

/**
*@brief clear irq pending bits in a rtc register
*
*@param[in] val	: value written to the register
*@param[in] reg	: rtc register address
*@return NULL
*@note
*	write 1 to the corresponding bit will clear it, not 0.
*/
static inline void RTC_IRQ_CLEAR_PENDING(INT32U val, INT32U reg)
{
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	int i=0;
	for(i=0;i<8;i++)
#endif
	RTC_WRITE(val,reg);
}

#endif /* !defined (_LANGUAGE_ASSEMBLY) */
/**
 *@}
 */

#endif /* _ACTIONS_IO_H_ */


