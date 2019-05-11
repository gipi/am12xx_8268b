#ifndef TASK_MANAGER_DEBUG_H
#define TASK_MANAGER_DEBUG_H

/**
* macro contol the manager debug
*/

#define TASK_MANAGER_DEBUG

#ifdef TASK_MANAGER_DEBUG
	#define TASKMGR_DEBUG printf
#else
	#define TASKMGR_DEBUG(...) 
#endif


#endif
