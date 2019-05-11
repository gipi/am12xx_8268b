#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "taskmanager_message.h"
#include "taskmanager_task.h"
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <fcntl.h>
#include "sys_cfg.h"
#include "ezcast_public.h"


/**
* the global variable that reserves all the task information
*/
struct taskmgr_task_info task_info[TASK_TYPE_MAX];

static void task_info_init()
{
	int i;

	memset(task_info,0,sizeof(struct taskmgr_task_info)*TASK_TYPE_MAX);

	for(i=0;i<TASK_TYPE_MAX;i++){
		task_info[i].pid = -1;
	}
}

int main(int argc, char *argv[])
{
	int mgr_msgq_id;
	ssize_t msgsize;
	struct am_sys_msg msg;
	int err;
	char fpath[128];
	FILE *fp;
	task_info_init();

	/** some configuration task */
	taskmgr_create_and_wait_task("/am7x/bin/config.app","config.app");
	
	/** create the mainloop message queue */
	mgr_msgq_id = msgget(IPC_PRIVATE,IPC_CREAT);
	if(mgr_msgq_id == -1){
		TASKMGR_DEBUG("manager msg queue err\n");
		exit(1);
	}

	/** create thread for collecting message from kernel */
	taskmgr_create_message_collector(mgr_msgq_id);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8250 || MODULE_CONFIG_EZCASTPRO_MODE==8075)
    memset(fpath,0,sizeof(fpath));
    sprintf(fpath,"%s","/am7x/bin/rs232_main.app");

	err=taskmgr_create_communication_task(mgr_msgq_id,&task_info[TASK_TYPE_RS232_APP],fpath,"rs232_main.app");
	TASKMGR_DEBUG("application create err == %d, msgq_id=%d\n",err,mgr_msgq_id);
#endif	
	memset(fpath,0,sizeof(fpath));
	if(FLASH_TYPE != FLASH_TYPE_8M_SNOR)
		sprintf(fpath,"%s","/am7x/bin/fui.app");
	else
		sprintf(fpath,"%s","/am7x/bin/wireUI.app");
#ifdef SYS_DEBUG_MODE
	memset(fpath,0,sizeof(fpath));
	if(FLASH_TYPE != FLASH_TYPE_8M_SNOR)
		sprintf(fpath,"%s","/mnt/udisk/fui.app");
	else
		sprintf(fpath,"%s","/mnt/udisk/wireUI.app");
	fp = fopen(fpath,"rb");
	if(fp==NULL)
	{
		memset(fpath,0,sizeof(fpath));
#if (FLASH_TYPE==FLASH_TYPE_8M_SNOR)
		printf("use default wireUI.app\n");
		sprintf(fpath,"%s","/am7x/bin/wireUI.app");
#else
		printf("use default fui.app\n");
		sprintf(fpath,"%s","/am7x/bin/fui.app");
#endif
	}
	else
	{
		fclose(fp);
		printf("use debug app\n");
	}
#endif

	// Create system operation task
	err = taskmgr_create_system_opt_task();

	/** create main application task */
	if(FLASH_TYPE !=FLASH_TYPE_8M_SNOR)
		err=taskmgr_create_communication_task(mgr_msgq_id,&task_info[TASK_TYPE_ACTIVE_APP],fpath,"fui.app");
	else
		err=taskmgr_create_communication_task(mgr_msgq_id,&task_info[TASK_TYPE_ACTIVE_APP],fpath,"wireUI.app");
	TASKMGR_DEBUG("application create err == %d, msgq_id=%d\n",err,mgr_msgq_id);
	/** the main loop */
	while(1){
		/** manager will pend here if there is no message received */
		msgsize = msgrcv(mgr_msgq_id,(void *)&msg,sizeof(struct am_sys_msg),0,0);
		
		if(msgsize != -1){
			switch(msg.type){
				
			case SYSMSG_KEY:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].keypipe[1],(void *)&msg,sizeof(struct am_sys_msg));
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8250
				err=write(task_info[TASK_TYPE_RS232_APP].keypipe[1],(void *)&msg,sizeof(struct am_sys_msg));
#endif				
				break;

			case SYSMSG_CARD:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].fd[1],(void *)&msg,sizeof(struct am_sys_msg));
				break;
				
			case SYSMSG_MOUSE:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].fd[1],(void *)&msg,sizeof(struct am_sys_msg));
				break;	
				
			case SYSMSG_USB:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].fd[1],(void *)&msg,sizeof(struct am_sys_msg));
				break;

			case SYSMSG_TOUCH:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].fd[1],(void *)&msg,sizeof(struct am_sys_msg));
				break;
				
			case SYSMSG_ALARM:
				err=write(task_info[TASK_TYPE_ACTIVE_APP].fd[1],(void *)&msg,sizeof(struct am_sys_msg));
				break;
				
			case SYSMSG_RECOVER:
				system("rmmod am7x_recover.ko");
				break;
				
			default:
				TASKMGR_DEBUG("msg 0x%x received\n",(int)msg.type);
				break;
			}
		}
	}

	
	/** delete message queue */
	msgctl(mgr_msgq_id,IPC_RMID,NULL);
	
	exit(0);
}


