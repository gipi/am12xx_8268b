#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include "swf_types.h"
#include "sys_msg.h"
#include "display.h"

#define LEFT_BUTTON_DN      0x01
#define RIGHT_BUTTON_DN	    0x02
#define MIDDLE_BUTTON_DN    0x04

void *deinst=NULL;
int imagew,imageh;

static int _mouse_open_lcm()
{
	DE_config ds_conf;

	de_init(&deinst);
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	
	imagew = ds_conf.input.width;
	imageh = ds_conf.input.height;
	
	return 0;
}


static int _mouse_data_translate(char* ps2,struct am_sys_msg *msg)
{
	unsigned char button=ps2[0]&0x07;
	static unsigned char oldbutton=0;
	msg->dataload = ((ps2[1]&0x0000ffff)<<16) |(ps2[2]&0x0000ffff);
	msg->type = SYSMSG_MOUSE;
	if(ps2[3])
	{	
		if(ps2[3]>=9)//0x09--0x0f   
		{
			msg->dataload = 0x10-ps2[3];
		}
		else//0x01--0x08
		{
			ps2[3]= 0x10-ps2[3];
			msg->dataload = ps2[3]|0xfffffff0;
		}
	}

	if(button){
		if(oldbutton){
			switch(button){
				case LEFT_BUTTON_DN:
					msg->subtype = SWF_MSG_LDRAG;
					break;
				case RIGHT_BUTTON_DN:
					msg->subtype = SWF_MSG_RDRAG;
					break;
				case MIDDLE_BUTTON_DN:
					msg->subtype = SWF_MSG_MDRAG;
					break;
			}
			//msg->subtype = SWF_MSG_MOVE;
		}
		else {
			switch(button){
				case LEFT_BUTTON_DN:
					msg->subtype = SWF_MSG_LBUTTONDOWN;
					break;
				case RIGHT_BUTTON_DN:
					msg->subtype = SWF_MSG_RBUTTONDOWN;
					break;
				case MIDDLE_BUTTON_DN:
					msg->subtype = SWF_MSG_MBUTTONDOWN;
					break;
			}
		}
	}
	else{
		if(oldbutton)
		{
			switch(oldbutton){
				case LEFT_BUTTON_DN:
					msg->subtype = SWF_MSG_LBUTTONUP;
					break;
				case RIGHT_BUTTON_DN:
					msg->subtype = SWF_MSG_RBUTTONUP;
					break;
				case MIDDLE_BUTTON_DN:
					msg->subtype = SWF_MSG_MBUTTONUP;
					break;	
			}
		}
		else{
			if(ps2[3])
				msg->subtype = SWF_MSG_WHEEL;
			else
				msg->subtype = SWF_MSG_MOVE;
		}
		
	}
	oldbutton = button;
	return 0;
}


int main(int argc, char *argv[])
{
	int fdmouse;
	int nread,nwr;
	char ps2data[4];
	struct am_sys_msg msg;
	int pos,x,y,x_off,y_off;
	int msgid;

	if(argc > 1){
		if(argc != 2){
			printf("mouse process error parameters\n");
			exit(1);
		}
		msgid = (int)strtol(argv[1],NULL,10);
	}
	else{
		printf("mouse process arg err\n");
		exit(-1);
	}

	fdmouse = open("/dev/mice",O_RDONLY);
	if(fdmouse<0){
		printf("open mouse device failed\n");
		exit(-1);
	}

	_mouse_open_lcm();

	Mouse_init(100,100);

	while(1)
	{
		nread=read(fdmouse, ps2data, sizeof(ps2data));
		if(nread==sizeof(ps2data))
		{
			_mouse_data_translate(ps2data,&msg);
			pos = msg.dataload;
			x = GetMouseX();
			y = GetMouseY();
			x_off = pos>>16;
			y_off = (pos<<16)>>16;

			if(msg.subtype == SWF_MSG_MOVE 
			|| msg.subtype >= SWF_MSG_LDRAG)
			{
				x+=x_off*2;
				y+=y_off*2;
				Mouse_Move(0, x, y, imagew, imageh);
			}
			msgsnd(msgid, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
		}


	}

	close(fdmouse);
	
	return 0;

}


