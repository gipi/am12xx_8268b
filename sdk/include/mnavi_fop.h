/**
*@addtogroup mnavi_lib_s
*@{
*/

#ifndef MNAVI_FOP_H
#define MNAVI_FOP_H

#include "semaphore.h"

/**copy work cmd*/
#define MNAVI_CMD_BASE			0xff
#define MNAVI_CMD_FILE_COPY		MNAVI_CMD_BASE+1	
#define MNAVI_CMD_DIR_COPY		MNAVI_CMD_BASE+2	
#define MNAVI_CMD_FILE_DEL		MNAVI_CMD_BASE+3	
#define MNAVI_CMD_DIR_DEL		MNAVI_CMD_BASE+4	
#define MNAVI_CMD_FILE_OVERLAP	MNAVI_CMD_BASE+5	
#define MNAVI_CMD_DIR_OVERLAP	MNAVI_CMD_BASE+6	
#define MNAVI_CMD_INVALID		MNAVI_CMD_BASE+0xff

/** return value*/
#define MNAVI_SUCCESS			0
#define MNAVI_ERROR			-1

/**work status*/
enum MNAVI_STATUS{
	MNAVI_WORK_IN_PROCESS = 0,
	MNAVI_WORK_FINISH_SUCCESS,
	MNAVI_WORK_STOPED_FILE_EXSIT,		
	MNAVI_WORK_STOPED_RDONLY_FS,			
	MNAVI_WORK_STOPED_IO_ERROR,			
	MNAVI_WORK_STOPED_INVALID_ARG,
	MNAVI_WORK_STOPED_USER_CANCEL,
	MNAVI_WORK_STOPED_DISK_FULL
};

/**create work structure*/
struct mnavi_work_info
{
	int cmd;
	char* src;
	char* des;	
};

/**work handle*/
struct mnavi_work_handle_t{
	pthread_t tid;
	double process;	
	int status;
	int action;
	sem_t sem;		
	struct mnavi_work_info* work;
};

/**
*@brief API for create a copy/delet work
*
*@param[in] work	: work struct parameter
*@retval NULL	: failed
*@retval	!0		: work handle
*/
struct mnavi_work_handle_t* mnavi_create_work(struct mnavi_work_info *work);

/**
*@brief API for start work
*
*@param[in] work	: work handle 
*@retval 0		: success
*@retval !0		: failed
*/
int mnavi_start_work(struct mnavi_work_handle_t* handle);

/**
*@brief API for stop work
*
*@param[in] work	: work handle 
*@retval 	0		: success
*@retval 	!0		: failed
*/
int mnavi_stop_work(struct mnavi_work_handle_t* handle);

/**
*@brief API for get work process
*
*@param[in] work	: work handle 
*@retval 	<0		: error occured
*@retval	>=0		: work finished process
*/
double mnavi_get_process(struct mnavi_work_handle_t* handle);

/**
*@brief API for get work status
*@param[in] work	: work handle 
*@retval 			: work status 
*/
int mnavi_get_status(struct mnavi_work_handle_t* handle);

/**
*@brief API for release work
*
*@param[in] work	: work handle 
*@retval 	0		:  success
*@retval 	!0		: failed
*/
int mnavi_release_work(struct mnavi_work_handle_t* handle);

#endif
/**
 *@}
 */