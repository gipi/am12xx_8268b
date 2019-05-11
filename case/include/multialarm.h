
#ifndef _ALARM_H_
#define _ALARM_H_

#include "sys_rtc.h"
#include <time.h>
/**
*@addtogroup alarm_lib_s
*@{
*/

#define ALARM_MAX_NUM  8 			///< the max num of alarm in the system is 8
#define ALARM_SHUTUP_BITMAP 1		///< how many bytes will the the shutup bitmap occupied, this value is equal to (ALARM_MAX_NUM/8)
#define ALARM_VAILD_BITMAP ALARM_SHUTUP_BITMAP	///< how many bytes will the the valid bitmap occupied
#define ALARM_NAME_LEN 12			///< how many bytes will the name buffer occupied

typedef enum{
	ALARM_GET_TOLNUM=0,			///< get total num of the alarms
	ALARM_GET_INFO_VIRID=1,		///< get the virtual idx of the alarm
	ALARM_GET_INFO_PHYID=2,		///< get the real idx of the alarm
}alarm_cmd_getinfo_e;

typedef enum{
	ALARM_INVALID=0,		///< alarm which is invaild
	ALARM_VALID=1,			///< alarm which is vailid
}alarm_cmd_getid_e;

typedef enum{
	ALARM_OPEN_ALL=0,		///< open all alarms
	ALARM_OPEN_ONE=1,		///< open the specified alarm
}alarm_cmd_open_e;

typedef enum{
	ALARM_MSG_ALARMON=0,
}alarm_cmd_msgsend_e;

typedef struct alarm_info{
	unsigned char enable:1;		///< enable the alarm
	unsigned char days:7;			///< which day can the alarm works
	unsigned char snooze:3;		///< snoozing time
	unsigned char hour:5;			///< hour
	unsigned char min;			///< miniute
	int user_id;					///< user id 
	unsigned char name[ALARM_NAME_LEN];	///< the name of the alarm
	unsigned char ismsgsend:1;			///< whether the message is sent
	unsigned char istimematch:1;			///< whether the tiem had matched 
}alarm_info_t;

typedef struct alarm_msg_send_s{
	void* pararcv;										///< the params received for using when the msg_send_func is called
	char (*msg_send_func)(void* pararcv,void* parasend);		///< the callback function which used for sending msg
}alarm_msg_send_t;

typedef struct alarm_info_head{
	unsigned char num;								///< the real number of alarms which had been added
	char curday;										///< current day
	char curmonth;									///< current month
	short int curyear;									///< current year
	unsigned char alarmbitmap[ALARM_VAILD_BITMAP];	///< the bitmap of the alarms
	unsigned char shutup[ALARM_SHUTUP_BITMAP];		///< the bitmap of the alarms which represent whethe the alarm is turn off
	alarm_info_t info[ALARM_MAX_NUM];					///< the information of alarms
	alarm_msg_send_t msg_send;
}alarm_info_head_t;

typedef struct auto_power_info{
	char auto_power_on_enable; 		//< 0 disalbe 1 enable
	rtc_time_t power_on_time;

	char auto_power_off_enable; 		//< 0 disable  1 enable
	rtc_time_t power_off_time;

	char auto_power_frequence;		//< 01111111 form right to left, each bit stands for one day(monday to sunday),if the bit is 1, the day is enable 
}auto_power_info_t;

typedef struct auto_screenoff_info{
	char auto_screen_off_enable; 		//< 0 disalbe 1 enable
	int auto_screen_off_time;		//< mins
	rtc_time_t screen_off_time;
	rtc_date_t screen_off_date;
}auto_screenoff_info_t;

/**
*@brief add a new alarm
*
*@param[in] palarminfo	: the information of the alarm to be added
*@retval -1 : error
*@retval others : the id of the alarm 
*@see the definition of alarm_info_t structure
*/
char al_add_alarm(alarm_info_t *palarminfo);


/**
*@brief delete an alarm
*
*@param[in] ID	:  id of a alarm which had been added
*@retval -1 : error
*@retval 0 : success 
*/
char al_del_alarm(char ID);


/**
*@brief set an alarm
*
*@param[in] ID	:  id of a alarm which had been added
*@param[in] palarminfo :   new information of the alarm
*@retval -1 : error
*@retval 0 : success 
*/
char al_set_alarm(char ID,alarm_info_t *palarminfo);


/**
*@brief get information of an alarm
*
*@param[in] cmd	:  see alarm_cmd_getinfo_e
*@param[in] ID	:  id of a alarm which had been added
*@param[in] palarminfo :   the information of the alarm which will be filled
*@retval -1 : error
*@retval >=0 : the  real index of the  alarm
*/
char al_get_alarm_info(char cmd,char Idx,alarm_info_t *palarminfo);


/**
*@brief load the information of alarms from vram
*
*@param[in] NULL
*@retval 0
*/
char al_load_alarm_info();


/**
*@brief strore the information of alarms into vram
*
*@param[in] NULL
*@retval 0
*/
char al_store_alarm_info();


/**
*@brief turn off an alarm
*
*@param[in] ID : which alarm to be turned off
*@retval 0
*/
char al_close_alarm(char ID);


/**
*@brief open an alarm
*
*@param[in] ID : which alarm to be opened
*@retval 0
*/
char al_open_alarm(char ID);


/**
*@brief check whether it is the time to be alarm
*
*@param[in] NULL
*@retval 0
*/
char al_check();

char al_set_standby(unsigned int *alarmtime);


/**
*@brief initial the alarms
*
*@param[in] alarm_msg_send_t : the information which wil be used when the message is sent
*@retval 0
*/
char al_init_alarms(alarm_msg_send_t *msg_send);

/**
*@brief save the alarm information if needed
*
*@param[in] NULL
*@retval 0
*/
char al_save_alarm_info();


/**
*@brief load the information of auto power from vram
*
*@param[in] NULL
*@retval 0
*/
char al_load_poweroff_info();

/**
*@brief store the information of auto power into vram
*
*@param[in] NULL
*@retval 0
*/
char al_store_poweroff_info();

/**
*@brief execute auto power off check
*
*@param[in] NULL
*@retval 0
*/
char power_off_check();

/**
*@brief execute auto power off according to wakeup mode
*
*@param[in] NULL
*@retval 0
*/
int auto_power_off(int wakeup_mode);

/**
*@brief set auto power on time
*
*@param[in] NULL
*@retval 0
*/
struct tm * al_set_standbytime();

/**
*@brief load the information of auto screen off from vram
*
*@param[in] NULL
*@retval 0
*/
char al_load_screenoff_info();

/**
*@brief store the information of auto screen off into vram
*
*@param[in] NULL
*@retval 0
*/
char al_store_screenoff_info();

/**
*@brief set the date and time of auto screen off
*
*@param[in] NULL
*@retval 0
*/
char al_set_screenoff_time();

/**
 *@}
 */

#endif

