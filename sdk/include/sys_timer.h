#ifndef _SYS_TIMER_H_
#define _SYS_TIMER_H_
/**
*@file sys_timer.h
*@brief this head file describes the timer operations for Actions-micro IC
*
*@author yekai
*@date 2010-05-12
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup timer_lib_s
*@{
*/

/**
*@brief create a friendly timer, which can invoke a namely ISR automatically
*
*@param[in] mdelay	: interval time of the timer, based on ms
*@param[in] proc		:  function entry of the ISR
*@param[in] pdata		: pointer of the ISR params
*@retval >0			: timer id
*@retval <=0			: standard error no
*/
int am_timer_create(int mdelay, void (*proc)(void*), void *pdata);

/**
*@brief delete an existed timer
*
*@param[in] timer_id	: id of the timer which is to be deleted
*@retval 0			: success
*@retval !0			: standard error no
*/
int am_timer_del(int timer_id);

/**
*@brief modify interval time of an existed timer
*
*@param[in] timer_id	: id of the timer which is to be modify
*@param[in] mdelay	: new interval time of the timer, based on ms
*@retval 0			: success
*@retval !0			: standard error no
*/
int am_timer_set(int timer_id,int mdelay);

/**
 *@}
 */

#ifdef __cplusplus
}
#endif
 	
#endif //_SYS_TIMER_H_