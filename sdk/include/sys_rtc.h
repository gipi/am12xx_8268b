#ifndef __SYS_RTC_H__
#define __SYS_RTC_H__
/**
*@file sys_rtc.h
*@brief this head file represents the rtc operations.
*
*
*@author yekai
*@date 2010-04-14
*@version 0.1
*
*@modified record:
*    2010-04-23 by yekai
*	add some alarm functions
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup rtc_lib_s
*@{
*/

/**RTC device path*/
#define RTC_DEV_PATH	"/dev/rtc0"

/**
*@name RTC alarm operation flag
*@{
*/
#define RTC_ALARM_ENABLE			1
#define RTC_ALARM_DISABLE			0
#define RTC_ALARM_UNDO			-1
/**
*@}
*/

/**
*@name RTC regs op, private only
*@{
*/
#define RTC_CLK_EN					(1<<2)
#define RTC_CTL_OFF					0
#define RTC_STAT_OFF				0x14
#define RTC_EN						(1<<4)
#define RTC_NORMAL					(1<<11)
#define RTC_ALARM_IRQ_EN			(1<<2)
#define RTC_ALARM_PD				1

#define RTC_ALARM_CTL_OFF			(RTC_ALARM-RTC_CTL)
#define RTC_ALARM_EN 				1
#define RTC_ALARM1_EN				2
#define RTC_ALARM_CONF_MASK		0xffff800f
#define RTC_ALARM_GPOSEL_MASK	(1<<31)
#define RTC_ALARM_GPO_MASK		(1<<8)

#define RTC_ALARM_GPIO 				(0<<12)
#define RTC_ALARM_SPEC				(1<<12)
#define RTC_ALARM_GEN				(2<<12)

#define RTC_ALARM_GPIE				(1<<11)
#define RTC_ALARM_GPOE				(1<<9)
#define RTC_ALARM_GPO_HIGH		(1<<8)
#define RTC_ALARM_GPO_LOW			(0)

#define RTC_ALARM_SMT_EN			(1<<14)
#define RTC_ALARM_PW_OFFSET		4
#define RTC_ALARM_PW_MAX			15
#define RTC_ALARM_PW_MIN			0
/**
*@}
*/

/**
*@brief rtc date struct
*/
typedef struct
{
	int year;		/**<year, ex 2010*/
	int month;	/**<month, 1~12*/
	int day;		/**<day,1~31*/
	int wday;		/**<weekday,1~7*/
}rtc_date_t;

/**
*@brief rtc time struct
*/
typedef struct{
	int hour;		/**<hour,0~23*/
	int min;		/**<minute,0~59*/
	int sec;		/**<second,0~59*/
}rtc_time_t;

/**
*@brief get rtc time and date
*
*@param[in] date	: user space pointer of the date var,  which  the rtc date value will be returned to
*@param[in] time	: user space pointer of the time var, which the rtc time value will be returned to
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	the null pointer of the input param will be ignored
*/
int tm_get_rtc(rtc_date_t *date,rtc_time_t *time);

/**
*@brief set rtc time and date
*
*@param[in] date	: user space pointer of the date var,  whose value will be set into rtc date
*@param[in] time	: user space pointer of the time var, whose value will be set into rtc time
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	the null pointer of the input param will be ignored and the coresponding value in rtc will keep unchanged
*/
int tm_set_rtc(rtc_date_t *date,rtc_time_t *time);

/**
*@brief enable rtc alarm and set the wake time at the meantime
*
*@param[in] date	: user space pointer of the date var,  whose value will be set into alarm date
*@param[in] time	: user space pointer of the time var, whose value will be set into alarm time
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	if either pointer of the input param is null, the coresponding rtc time will be set into alarm instead 
*/
int tm_open_alarm(rtc_date_t * date, rtc_time_t * time);

/**
*@brief disable rtc alarm
*
*@param[in] NULL
*@retval 0	: success
*@retval !0	: standard error no
*/
int tm_close_alarm(void);

/**
*@brief get the wake time of the rtc alarm
*
*@param[in] date	: user space pointer of the date var,  which  the alarm date value will be returned to
*@param[in] time	: user space pointer of the time var, which  the alarm time value will be returned to
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	the null pointer of the input param will be ignored
*/
int tm_get_alarm(rtc_date_t * date, rtc_time_t * time);

/**
*@brief set the wake time of the rtc alarm
*
*@param[in] date	: user space pointer of the date var,  whose value will be set into alarm date
*@param[in] time	: user space pointer of the time var, whose value will be set into alarm time
*@retval 0		: success
*@retval !0		: standard error no
*@note
*	if either pointer of the input param is null, the coresponding rtc time will be set into alarm instead 
*/
int tm_set_alarm(rtc_date_t * date, rtc_time_t * time);

/**
*@brief wait for a rtc alarm
*
*@param[in] pdate	: user space pointer of the date var,  which  the alarm date value will be returned to\n
*the upper 24 bits presents the irq num and the lower 8 bits for irq flag 
*@return number of read bytes
*@note
*	the thread will be suspended until alarm arrived and it is irq interruptible during waiting 
*/
int tm_wait_alarm(unsigned int *pdata);

/**
 *@}
 */

#ifdef __cplusplus
}
#endif
 
#endif  //__SYS_RTC_H__