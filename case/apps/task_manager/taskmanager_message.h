#ifndef _TASKMANAGER_MESSAGE_H
#define _TASKMANAGER_MESSAGE_H

#include "sys_msg.h"
#include "taskmanager_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* \fn extern void *taskmgr_message_collector(void *arg)
*
* @brief
*	the thread loop function for message collecting.
*
* @param  
*	arg the manager mainloop's message qureue identification.
*
* @return
*	will not return except that manager exits.
*/
extern void *taskmgr_message_collector(void *arg);



/**
* \fn extern int taskmgr_create_message_collector(int msgq_id) 
*
* @brief
*	create the message collector thread.
*
* @param  
*	msgq_id the manager mainloop's message qureue identification.
*
* @return
*	0 if success or -1 if failed.
*/
extern int taskmgr_create_message_collector(int msgq_id);



/**
* @brief map raw key data to system standard key.
*/
/**
* \fn extern int taskmgr_syskey_map(struct am_sys_msg *rawkey)
*
* @brief
*	map raw key data to system standard key.
*
* @param  
*	rawkey the key message that to be mappped.
*
* @return
*	0 if success or -1 if failed.
*/
extern int taskmgr_syskey_map(struct am_sys_msg *rawkey);

#ifdef __cplusplus
}
#endif

#endif
