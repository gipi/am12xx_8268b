#ifndef _SYS_MONITOR_H_
#define _SYS_MONITOR_H_
/**
*@file sys_monitor.h
*@brief This head file describes the operations for processes monitor\n
*All APIs here is for developers to debug and profile codes.\n
*In order to make this API work, the macro __EN_MONITOR should be set.
*	
*@author yekai
*@date 2010-06-17
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <sys/time.h>

/**
*@addtogroup monitor_lib_s
*@{
*/

/**
*@name monitor config flag
*@{
*/
/**enable monitor flag*/
#define __EN_MONITOR		0
/**use absolute time other than relative jiffy*/
#define USE_ABSOLUTE_TIME	0
/**buffer size for proc system data*/
#define PROCPS_BUFSIZE 		256
/**max threads number in proc system*/
#define PROCPS_MAX			50
/**
*@}
*/

/**
*@brief struct for monitor global time
*/
struct am_monitor{
	struct timeval timeval_base;	/**<start time*/
	int is_monitor;			/**<avalid flag of monitor*/
	int sys_tick;				/**<tick frequency of the system*/
};

/**
*@brief struct for jiffy info in proc
*/
typedef struct jiffy_counts_t {
	/* Linux 2.4.x has only first four */
	unsigned long long usr, nic, sys, idle;			/**<jiffies of usr,nice,system and idle*/
	unsigned long long iowait, irq, softirq, steal;	/**<jiffies of iowait,irq,softirq and steal*/
	unsigned long long total;					/**<total jiffies*/
	unsigned long long busy;					/**<jiffies in busy*/
} jiffy_counts_t;

/**
*@brief struct for stat in proc
*/
typedef struct pro_stat{
	DIR *dir;							/**<dir info*/
	unsigned int pid;					/**<pid of the thread*/
	unsigned long stime,utime;			/**<system and user time*/
	int task_nice;						/**<nice value*/
	char state[4];						/**<task state*/
	char comm[16];					/**<task command*/
}pro_stat_t;

/**
*@brief monitor struct for proc scan
*/
struct proc_monitor{
	pro_stat_t proc_status;					/**<status in proc*/
	unsigned long stime_base, utime_base;	/**<start time in system and user space*/
};


/**
*@brief set start position of the scope which to be monitored.
*
*This function is only for watching a single process and should be invoked before monitor_end
*@param[in] NULL
*@return
*@retval 0	: success
*@retval !0	: standard error no
*/
int monitor_start(void);

/**
*@brief set end position of the scope which to be monitored and display the corresponding process information
*
*This function is only for watching a single process and should be invoked after monitor_start.
*@param[in] NULL
*@return
*@retval 0	: success
*@retval !0	: standard error no
*/
int monitor_end(void);

/**
*@brief display the process time in a monitored scope.
*
*This function only display process utime/stime and should be invoked between monitor_start and monitor_end.
*@param[in] NULL
*@return
*@retval 0	: success
*@retval !0	: standard error no
*/
int monitor_out(void);

/**
*@brief set start position of the scope which to be monitored.
*
*This function can profile all the running process in the proc system and 
*should be invoked before easy_top_end.
*@param[in] NULL
*@return
*@retval 0	: success
*@retval !0	: standard error no
*/
int easy_top_start(void);

/**
*@brief set end position of the scope which to be monitored.
*
*This function can profile all the running process in the proc system and 
*should be invoked after easy_top_start.
*@param[in] NULL
*@return
*@retval 0	: success
*@retval !0	: standard error no
*@note
*	the status of the task which can be one of:
*	'D' = uninterruptible sleep
*	'R' = running
*	'S' = sleeping(interruptible sleep)
*	'T' = traced or stopped
*	'Z' = zombie
*/
int easy_top_end(void);

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif //_SYS_MONITOR_H_