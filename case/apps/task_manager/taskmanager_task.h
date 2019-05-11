#ifndef TASKMANAGER_TASK_H
#define TASKMANAGER_TASK_H


#include "taskmanager_debug.h"
#include <sys/types.h>
#include "ezcastpro.h"

/**
* @brief This head file includes the declaration for task manager's
* task related functions.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief 
*    we classify the task that task manager can handle into 
*    the following categories.
*/
enum {
	TASK_TYPE_ACTIVE_APP=0,
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8250 ||	MODULE_CONFIG_EZCASTPRO_MODE==8075)
	TASK_TYPE_RS232_APP,
#endif
	TASK_TYPE_DUMMY,
	TASK_TYPE_MAX                 // maximum types
};


/**
* @brief 
*     the task manager use this struct to store all of its child
*     process's information.
*/
struct taskmgr_task_info
{
	int valid;
	
	/** id of the process */
	int pid;
	
	/** pipe file descriptor */
	int fd[2]; 

	/** key pipe */
	int keypipe[2];
	
	/** 
	* if this task's parent has many child processes,
	* use "prev and next" to link them
	*/
	struct taskmgr_task_info *prev,*next; 
	
	/** pointed to the first child process */
	struct taskmgr_task_info *child;    
};



/**
* \fn extern int taskmgr_create_communication_task(int parent_msgq_id,struct taskmgr_task_info *task_info,char *full_path,char *task_name)
*
* @brief
*	Create a task that will communicate with the task manager.
*
* @param  
*	parent_msgq_id the parent message queue that to be passed to the child.
*
* @param  
*	task_info task information that the parent will change.
*
* @param  
*	full_path the executalbe file of the child.
*
* @param  
*	task_name name of the child.
*
* @return
*	0 if success or -1 if failed.
*/

extern int taskmgr_create_communication_task(int parent_msgq_id,struct taskmgr_task_info *task_info,char *full_path,char *task_name);




/**
* \fn extern int taskmgr_create_and_wait_task(char *fullpath,char *task_name);
*
* @brief
*	Create a task and wait for it to execute and return.
*
* @param  
*	fullpath path of the executable file.
*
* @param  
*	task_name name of the task.
*
* @return
*	0 if success or -1 if failed.
*/
extern int taskmgr_create_and_wait_task(char *fullpath,char *task_name);


/**
* \fn extern int taskmgr_delete_task(struct taskmgr_task_info *task)
*
* @brief
*	delete a child process.
*
* @param  
*	tpid process id of the task.
*
* @return
*	0 if success or other value if failed.
*/
extern int taskmgr_delete_task(struct taskmgr_task_info *task);



/**
* \fn extern int taskmgr_execute_scripts(char *scriptspath);
*
* @brief
*	execute a scripts file.
*
* @param  
*	scriptspath path of the scripts file.
*
* @return
*	0 if success or other value if failed.
*/
extern int taskmgr_execute_scripts(char *scriptspath);

#ifdef __cplusplus
}
#endif

#endif
