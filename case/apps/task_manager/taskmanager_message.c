#include "taskmanager_message.h"
#include "taskmanager_task.h"
#include "taskmanager_debug.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include "sys_msg.h"
#include <sys/msg.h>
#include <sys/select.h>
#include "key_map.h"
#include "linux/input.h"
#include "sys_msg.h"
#include <sys/time.h>
#include "sys_cfg.h"
#include "swf_types.h"
/**
* @brief the thread loop function for message collecting.
*/

static int _key_data_translate(struct input_event* event,struct am_sys_msg *msg)
{
	if(event->type==EV_KEY)
	{
		msg->type = SYSMSG_KEY;
		if(event->code<0x100){
			msg->subtype = KEY_SUBTYPE_BOARD;
			msg->dataload = (unsigned int)event->code;
		}
		else{
			msg->subtype = KEY_SUBTYPE_IR;
			msg->dataload = (unsigned int)event->code;
		}
		switch(event->value){
		case 0:
			msg->dataload = (msg->dataload&KEY_VALUE_MASK)|(KEY_ACTION_UP<<KEY_ACTION_MASK_SHIFT);
			break;
		case 1:
			msg->dataload = (msg->dataload&KEY_VALUE_MASK)|(KEY_ACTION_DOWN<<KEY_ACTION_MASK_SHIFT);
			break;
		case 3:
			msg->dataload = (msg->dataload&KEY_VALUE_MASK)|(KEY_ACTION_LONG<<KEY_ACTION_MASK_SHIFT);
			break;
		case 2:
			msg->dataload = (msg->dataload&KEY_VALUE_MASK)|(KEY_ACTION_HOLD<<KEY_ACTION_MASK_SHIFT);
			break;
		default:
			msg->dataload = (msg->dataload&KEY_VALUE_MASK)|(KEY_ACTION_SYNC<<KEY_ACTION_MASK_SHIFT);
			break;
		}
		
		printf("\nhwkeycode = 0x%x \n",msg->dataload);
		return 0;
	}
	return -1;
}

static int _tp_data_translate(struct input_event* event,struct am_sys_msg *msg)
{
	int ret=-1;
	static int x_pos=0,y_pos=0;
	static unsigned int subtype=0;
	if(event->type==EV_ABS)
	{
		switch(event->code){
			case ABS_X:
				x_pos = event->value;
				break;
			case ABS_Y:
				y_pos = event->value;
				break;
			case ABS_PRESSURE:
				subtype = event->value;
				break;
		}
	}
	else if(event->type==EV_SYN){
		msg->type = SYSMSG_TOUCH;
		msg->subtype = subtype;
		msg->dataload = (0x0000ffff&x_pos) |((0x0000ffff&y_pos)<<16);
		ret = 0;
	}
	return ret;
}
void *taskmgr_message_collector(void *arg)
{
	int msgq_id = (int)arg;
	int fd,fdkey,fdtp,fdmouse,rbkey;
	struct am_sys_msg msg;
	int nread=0;
	int thread_exit=0;
	fd_set rset;
	int maxfd;
	char ps2data[4];
	struct input_event event;

	
	/** open the device that kernel put message into */
	fd = open("/dev/sysmsg", O_RDONLY|O_NONBLOCK);
	if(fd==-1){
		TASKMGR_DEBUG("can't open sysmsg device\n");
		thread_exit = -1;
		pthread_exit((void *)&thread_exit);
	}

	/** open the key module */
	fdkey = open("/dev/event0",O_RDONLY | O_NONBLOCK);
	
	//write(fdkey,&skey,sizeof(set_keycode));
	if(fdkey<0){
		TASKMGR_DEBUG("open key device failed\n");
	}

	//keyboard_init(fdkey);
	
	/** open the touch module */
	fdtp = open("/dev/event1",O_RDONLY | O_NONBLOCK);
	if(fdtp<0){
		TASKMGR_DEBUG("open touchpanel device failed\n");
	}


	maxfd = fdkey>fdtp? fdkey:fdtp;
	maxfd = maxfd>fd? maxfd:fd;

#if 0
	rbkey = open("/dev/event2",O_RDONLY | O_NONBLOCK);
	if(rbkey<0){
		TASKMGR_DEBUG("open rbkey device failed\n");
	}
	maxfd = maxfd>rbkey? maxfd:rbkey;
#endif

	//
	/** mainloop for collect message from kernel */
	while(1){
		
		FD_ZERO(&rset);

		if(fdkey >= 0){
			FD_SET(fdkey,&rset);
		}

		if(fdtp >= 0){
			FD_SET(fdtp,&rset);
		}
		
		FD_SET(fd,&rset);
		
	#if 0
		FD_SET(rbkey,&rset);
	#endif
	
		if(select(maxfd+1,&rset,NULL,NULL,NULL)>0){

			if(fdkey >= 0){
				if(FD_ISSET(fdkey,&rset)){
					nread = read(fdkey,&event,sizeof(struct input_event));
					if(_key_data_translate(&event,&msg)==0)
						msgsnd(msgq_id, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
				}
			}

			if(fdtp >= 0){
				if(FD_ISSET(fdtp,&rset)){
					nread = read(fdtp,&event,sizeof(struct input_event));
					if(_tp_data_translate(&event, &msg)==0)
					{
						msgsnd(msgq_id, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
					}
				}
			}
		
			if(FD_ISSET(fd,&rset)){
				nread=read(fd, &msg, sizeof(struct am_sys_msg));
				msgsnd(msgq_id, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
			}
			
		#if 0
			if(FD_ISSET(rbkey,&rset)){
				nread = read(rbkey,&event,sizeof(struct input_event));
				if(_key_data_translate(&event,&msg)==0)
					msgsnd(msgq_id, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
			}
		#endif
		
			//printf("msg->type=%x,msg->subtype=%x,msg->dataload=%x\n",msg.type,msg.subtype,msg.dataload);


		}
	}
	close(fd);
	close(fdkey);
	close(fdtp);
#if 0
	close(rbkey);
#endif
	pthread_exit((void *)&thread_exit);

}

/**
* @brief create the message collector thread.
*/
int taskmgr_create_message_collector(int msgq_id)
{
	int err;
	pthread_t thread_id;

	err = pthread_create (&thread_id,NULL,taskmgr_message_collector,(void *)msgq_id);
	if(err != 0){
		TASKMGR_DEBUG("message collector create failed : %d\n",err);
	}
	else{
		TASKMGR_DEBUG("message collector id : %d\n",(int)thread_id);
	}

	return err;
}




