#include "taskmanager_task.h"
#include "taskmanager_debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>


/**
* Create a task that will communicate with the task manager.
*
* @param parent_msgq_id :message queue id of the task manager
* @param task_info :a struct pointed to task info that maintained by task manager
* @param fullpath :full path of the binary file
* @param task_name :name of the binary file
*
*/

int taskmgr_create_communication_task(int parent_msgq_id,struct taskmgr_task_info *task_info,char *full_path,char *task_name)
{
	int fd[2],keyfd[2];
	pid_t pid;
	int stat;
	char msg_stream[16];
	char fd0_stream[16];
	char fd1_stream[16];
	char key_stream[16];
	int flags;
	
	if (pipe(fd) < 0){
		TASKMGR_DEBUG("pipe error");
		task_info->valid = 0;
		return -1;
	}

	/**
	* create the key pipe for send key message.
	*/
	if (pipe(keyfd) < 0){
		TASKMGR_DEBUG("pipe error");
		task_info->valid = 0;
		return -1;
	}

	pid = fork();
	if(pid == -1){
		task_info->valid = 0;
		return -1;
	}
	else if(pid == 0){
		/**
		* child process,close the write pipe as we can communicate with
		* manager via message queue.
		*/
		close(fd[1]);
		close(keyfd[1]);

		/**
		* Set read pipe nonblock.
		*/
		flags = fcntl(fd[0], F_GETFL,0);
		flags |= O_NONBLOCK;
		if (fcntl(fd[0], F_SETFL, flags) < 0)
		{
			TASKMGR_DEBUG(("set read pipe nonblock failed!\n"));
		} 

		flags = fcntl(keyfd[0], F_GETFL,0);
		flags |= O_NONBLOCK;
		if (fcntl(keyfd[0], F_SETFL, flags) < 0)
		{
			TASKMGR_DEBUG(("set read pipe nonblock failed!\n"));
		} 
		
		sprintf(msg_stream,"%d",parent_msgq_id);
		sprintf(fd0_stream,"%d",fd[0]);
		sprintf(key_stream,"%d",keyfd[0]);
		
		stat = execl(full_path,task_name,msg_stream,fd0_stream,key_stream,NULL);
		if(stat == -1){
			TASKMGR_DEBUG("exec %s error: [%d]\n",full_path,errno);
			return -1;
		}
		
	}
	else{
		/** 
		* parent process, close the read pipe as we send information to
		* child process via pipe.
		*/
		close(fd[0]);
		close(keyfd[0]);
		task_info->valid = 1;
		task_info->pid = pid;
		task_info->fd[0] = fd[0];
		task_info->fd[1] = fd[1];
		task_info->keypipe[0] = keyfd[0];
		task_info->keypipe[1] = keyfd[1];
		task_info->prev = task_info;
		task_info->next = task_info;
		task_info->child = NULL;
		return 0;
	}

	return 0;
}



/**
* Create a task and wait for it to execute and return.
*
* @param full_path :the full path of the task execute file, e.g. /bin/sh.
* @param task_name :the name of the execute file, e.g. sh
*/

int taskmgr_create_and_wait_task(char *full_path,char *task_name)
{
	pid_t pid,wait_pid;
	int status;

	pid = fork();
	if(pid == -1){
		TASKMGR_DEBUG("task fork error\n");
		return -1;
	}
	else if(pid == 0){
		// child process
		TASKMGR_DEBUG("in child process\n");
		status = execl(full_path,task_name,NULL);
		if(status == -1){
			TASKMGR_DEBUG("exec %s error: [%d]\n",full_path,errno);
		}
		
		return 0;
	}
	else{
		// parent process, wait the child to exit
		wait_pid = wait(&status);
		if(wait_pid == -1){
			TASKMGR_DEBUG("wait for child error");
			return -1;
		}
		if(WIFEXITED(status)){
			TASKMGR_DEBUG("normal termination\n");
		}	
		if(WIFSIGNALED(status)){
			TASKMGR_DEBUG("signal exit\n");
		}
		if(WIFSTOPPED(status)){
			TASKMGR_DEBUG("stopped\n");
		}
		return 0;
	}
}

/**
* Create a task to do system command for fui.app.
*
*/
int taskmgr_create_system_opt_task()
{
	int fd[2];
	pid_t pid;
	FILE *fp = NULL;
	int ret;
	
	if (pipe(fd) < 0){
		TASKMGR_DEBUG("pipe error");
		return -1;
	}
	
	fp = fopen("/tmp/pthsystem_pipe_fd", "w");
	if(fp == NULL){
		TASKMGR_DEBUG("open tmp file error\n");
		return -1;
	}
	ret = fwrite((void *)fd, sizeof(int), 2, fp);
	fclose(fp);
	if(ret < 0){
		TASKMGR_DEBUG("write tmp file error\n");
		close(fd[0]);
		close(fd[1]);
		return -1;
	}

	pid = fork();
	if(pid == -1){
		return -1;
	}else if(pid == 0){
		char rfd_stream[16];
		char wfd_stream[16];
		snprintf(rfd_stream, sizeof(rfd_stream), "%d", fd[0]);
		snprintf(wfd_stream, sizeof(wfd_stream), "%d", fd[1]);
		
		ret = execl("/am7x/bin/pthsystem.app","pthsystem.app",rfd_stream,wfd_stream,NULL);
		if(ret < 0){
			TASKMGR_DEBUG("exec /am7x/bin/pthsystem.app error: [%d]\n",errno);
			return -1;
		}
		return 0;
	}else if(pid == 1){
		close(fd[0]);
		close(fd[1]);
		return 0;
	}
	return 0;
}


int taskmgr_execute_scripts(char *scriptspath)
{
	int err;
	
	if(scriptspath == NULL){
		return -1;
	}

	err = system(scriptspath);

	return err;
}


int taskmgr_delete_task(struct taskmgr_task_info *task)
{
	int err;

	/**
	* NOTE: After kill a process, it may cause a zombie
	* process. So we should do something in manager.
	*/
	
	if(task == NULL){
		return -1;
	}
	
	err = kill((int)task->pid,SIGKILL);

	task->valid = 0;

	/**
	* TO BE ADDED.
	*/
	
	return err;
}
