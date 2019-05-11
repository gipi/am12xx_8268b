/**
* @brief mapping from standard system keys to SWF-Player keys
*
* @author Simon Lee
* @date: 2010.05.21
* @version: draft version 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "key_map.h"
#include "fui_common.h"
#include "am7x_dac.h"
#include "sys_timer.h"
#include "swfdec.h"
#include "apps_vram.h"
#include "swf_types.h"
#include <sys/poll.h>
#include "linux/input.h"
#include <sys/ipc.h>

#define LEFT_BUTTON_DN		0x01
#define RIGHT_BUTTON_DN	0x02
#define MIDDLE_BUTTON_DN	0x04
/**
* @brief process some hot key here.
*/
int fui_deal_vol_key(int key)
{
	int vol;
	int rtn=0;
	struct sysconf_param sys_cfg_data;
	if((key!=SWF_MSG_KEY_EXT_VOL_PLUS)&&(key!=SWF_MSG_KEY_EXT_VOL_MINUS) \
		&&(key!=SWF_MSG_KEY_EXT_MUTE))
		return 0;
	
	switch(key){
		case SWF_MSG_KEY_EXT_VOL_PLUS:
			rtn = sys_volume_ctrl(_VOLUME_CTRL_INC,NULL);
		break;

		case SWF_MSG_KEY_EXT_VOL_MINUS:
			rtn = sys_volume_ctrl(_VOLUME_CTRL_DEC,NULL);
		break;

		case SWF_MSG_KEY_EXT_MUTE:
			rtn = sys_volume_ctrl(_VOLUME_CTRL_MUTE,NULL);
		break;
	}

	sys_volume_ctrl(_VOLUME_CTRL_GET,(void *)&vol);
	printf("%s,%d:Set Volume Key=0x%x,vol=%d\n",__FILE__,__LINE__,key,vol);
	
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	sys_cfg_data.sys_volume = (char)vol;
	apps_vram_write(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	apps_vram_store(VRAM_ID_SYSPARAMS);
	return 0;
}

/**
********************************************************
* touch panel message dispath.
********************************************************
*/
enum{
	TOUCH_S_IDLE=0,
	TOUCH_S_DOWN=1,
	TOUCH_S_MOVE=2,
	TOUCH_S_TIMEOUT=3
};
static int touchsm = TOUCH_S_IDLE;
static int xsave=0,ysave=0;
static int x_down_save=0,y_down_save=0,down_position=0;
static int touch_timer_id = -1;

/** 20ms for timer */
#define TOUCH_TIMER_INTERVAL    20
#define TOUCH_POS_INTERVAL      15
#define TOUCH_SLIDE_INTERVAL    1


#define CALIBRATE_FILE_PATH "/mnt/vram/pointercal"
int cali_params[7]={1,0,0,0,1,0,1};

int set_touch_calibrate_default()
{
	cali_params[0] = 1;
	cali_params[1] = 0;
	cali_params[2] = 0;
	cali_params[3] = 0;
	cali_params[4] = 1;
	cali_params[5] = 0;
	cali_params[6] = 1;

	return 0;
}

int get_touch_calibrate_params()
{
	int pcal_fd;
	char pcalbuf[200];
	int index;
	char *tokptr;
	
	pcal_fd = open(CALIBRATE_FILE_PATH,O_RDONLY);
	if(pcal_fd < 0 ){
		printf("open calibrate file failed\n");
		set_touch_calibrate_default();
		return -1;
	}
	if(read(pcal_fd,pcalbuf,200)<0){
		printf("read calibrate file failed\n");
		set_touch_calibrate_default();
		close(pcal_fd);
		return -1;
	}
	printf("get_touch_calibrate_params, ok\n");
	cali_params[0] = atoi(strtok(pcalbuf," "));
	
	index=1;
	while(index<7) {
		tokptr = strtok(NULL," ");
		if(*tokptr!='\0') {
			cali_params[index] = atoi(tokptr);
			printf("cali_params[%d]: %d\n", index, cali_params[index]);
			index++;
		}
	}

	close(pcal_fd);

	return 0;
}
static int _translate_touch_pos(int *x, int *y)
{
	int xtemp,ytemp;

	xtemp = *x;
	ytemp = *y;
//	printf("before _translate_touch_pos: x: %d, y: %d\n", xtemp, ytemp);
	*x =( cali_params[2] +cali_params[0]*xtemp + cali_params[1]*ytemp ) / cali_params[6];
	*y =( cali_params[5] +cali_params[3]*xtemp +cali_params[4]*ytemp ) / cali_params[6];
//	printf("after _translate_touch_pos: x: %d, y: %d\n", *x, *y);
	return 0;
}


static inline void _set_touch_stat(int stat)
{
	touchsm = stat;
}

static inline int _get_touch_stat()
{
	return touchsm;
}

static inline void _set_touch_pos(int x, int y)
{
	xsave = x;
	ysave = y;
}

static inline void _set_touch_down_pos(int x, int y)
{
	x_down_save = x;
	y_down_save = y;
}

static inline int _get_touch_down_pos()
{
	int x,y;
	
	x = x_down_save;
	y = y_down_save;
//	_translate_touch_pos(&x, &y);

	return (x<<16) | y;
}

static inline int _is_touch_pos_changed(int x,int y)
{
	if( abs(x-xsave)>TOUCH_POS_INTERVAL){
		return 1;
	}

	if( abs(y-ysave)>TOUCH_POS_INTERVAL){
		return 1;
	}

	return 0;
}
static int _delete_touch_timer()
{
#if 0
	if(touch_timer_id > 0){
		am_timer_del(touch_timer_id);
		touch_timer_id = -1;
	}
#endif	
	return 0;

}

void _touch_timer_handle(void *param)
{
#if 0
	int pos;
	
	if((touch_timer_id) >0 && (_get_touch_stat()==TOUCH_S_TIMEOUT)){
		_set_touch_stat(TOUCH_S_IDLE);
		pos = (xsave<<16) | ysave;
		//printf("====mouse up1:");
		SWF_Message(NULL, SWF_MSG_LBUTTONUP, (void *)pos);
		SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	}
	_delete_touch_timer();
#endif
}

static int _create_touch_timer()
{
#if 0
	if(touch_timer_id <= 0){
		touch_timer_id = am_timer_create(TOUCH_TIMER_INTERVAL,_touch_timer_handle,NULL);
		if(touch_timer_id <= 0){
			touch_timer_id = -1;
		}
	}
#endif	
	return 0;
}





/**
* @brief maps the touch panel message to mouse message.
* @param msg: the touch panel message.
*
* @return: 0 for sucess or failed.
*/

int fui_translate_touch_msg(struct am_sys_msg *msg)
{		
	int x,y;
	int pos;
	static int down_count=0;

	/** may be nonsense */
	if(msg->type != SYSMSG_TOUCH){
		return -1;
	}
	
	/** current x and y position */
	x = (msg->dataload & 0xffff);
	y = ((msg->dataload & 0xffff0000)>>16)&0xffff;
	
	if((x&(1<<15)) == (1<<15))
	{
		x = x & ~(1<<15);
		x = -x;
	}
	if((y&(1<<15)) == (1<<15))
	{
		y = y & ~(1<<15);
		y = -y;
	}	
	_translate_touch_pos(&x, &y);
	pos = (x<<16) | y;

#if 1
	if(msg->subtype == TOUCH_ACTION_DOWN){
		//printf("### touch down,x=%d,y=%d\n",x,y);
	}
	else if(msg->subtype == TOUCH_ACTION_UP){
		//printf("### touch up,x=%d,y=%d\n",x,y);
	}
	else if(msg->subtype == TOUCH_ACTION_SLIDE){
		//printf("### touch slide,x=%d,y=%d\n",x,y);
	}	
#endif

	switch(_get_touch_stat()){
		case TOUCH_S_IDLE:
			if(msg->subtype == TOUCH_ACTION_DOWN){
				_set_touch_stat(TOUCH_S_DOWN);
				_set_touch_pos(x,y);
				_set_touch_down_pos(x,y);
				SWF_Message(NULL, SWF_MSG_LBUTTONDOWN, (void *)_get_touch_down_pos());
				SWF_Message(NULL, SWF_MSG_PLAY, NULL);
				//printf("====mouse down:%d,%d\n",x,y);
			}
		break;

		case TOUCH_S_DOWN:
			if(msg->subtype == TOUCH_ACTION_DOWN){
				_set_touch_pos(x,y);
				_set_touch_down_pos(x,y);
				down_count = 0;
			}
			else if(msg->subtype == TOUCH_ACTION_UP){
				//_set_touch_stat(TOUCH_S_TIMEOUT);
				_set_touch_stat(TOUCH_S_IDLE);
				_set_touch_pos(x,y);
				down_count = 0;
				SWF_Message(NULL, SWF_MSG_LBUTTONUP, (void *)_get_touch_down_pos());
				SWF_Message(NULL, SWF_MSG_PLAY, NULL);
				/** set a timer to send up */
				//_create_touch_timer();
				//printf("====mouse up:%d,%d\n",x,y);
			}
			else if(msg->subtype == TOUCH_ACTION_SLIDE){
				down_count++;
				if(down_count>=TOUCH_SLIDE_INTERVAL){
					down_count = 0;
					_set_touch_stat(TOUCH_S_MOVE);
					_set_touch_pos(x,y);
				}
				else{
					_set_touch_pos(x,y);
				}
			}
		break;

		case TOUCH_S_MOVE:
			if(msg->subtype == TOUCH_ACTION_DOWN){
				_set_touch_stat(TOUCH_S_DOWN);
				_set_touch_pos(x,y);
				_set_touch_down_pos(x,y);
				printf("error state \n");
			}
			else if(msg->subtype == TOUCH_ACTION_UP){
				_set_touch_stat(TOUCH_S_IDLE);
				_set_touch_pos(x,y);
				//printf("====mouse up:%d,%d\n",x,y);
				SWF_Message(NULL, SWF_MSG_LBUTTONUP, (void *)pos);
				SWF_Message(NULL, SWF_MSG_PLAY, NULL);
			}
			else if(msg->subtype == TOUCH_ACTION_SLIDE){
				if(_is_touch_pos_changed(x,y)){
					//printf("====mouse move:%d,%d\n",x,y);
					SWF_Message(NULL, SWF_MSG_LDRAG, (void *)pos);
					SWF_Message(NULL, SWF_MSG_PLAY, NULL);
					_set_touch_pos(x,y);
				}
				
			}
		break;

		case TOUCH_S_TIMEOUT:
			if(msg->subtype == TOUCH_ACTION_DOWN){
				_delete_touch_timer();
				_set_touch_stat(TOUCH_S_DOWN);
				_set_touch_pos(x,y);
			}
			else{
				_delete_touch_timer();
				_set_touch_stat(TOUCH_S_IDLE);
				_set_touch_pos(x,y);
			}
		break;

		default:
			
		break;
		
	}


	Mouse_Move(0, x, y,  IMAGE_WIDTH_E, IMAGE_HEIGHT_E);
	
	return 0;
}

static int mouse_x_save = 100;
static int mouse_y_save = 100;
#define MMAX(a,b) (((a) > (b)) ? (a) : (b))
#define MMIN(a,b) (((a) < (b)) ? (a) : (b))

int fui_translate_mouse_msg(struct am_sys_msg *msg)
{
	static int inited = 0;
	int pos = msg->dataload;
	int x ;//= GetMouseX();
	int y ;//= GetMouseY();
	int x_off = pos>>16;
	int y_off = (pos<<16)>>16;

	if(inited){
		x = mouse_x_save;
		y = mouse_y_save;
	}
	else{
		x = GetMouseX();
		y = GetMouseY();
		mouse_x_save = x;
		mouse_y_save = y;
		inited = 1;
	}

	if(msg->subtype == SWF_MSG_MOVE 
		|| msg->subtype >= SWF_MSG_LDRAG)
	{
		x+=x_off*2;
		y+=y_off*2;

		mouse_x_save = x;
		mouse_y_save = y;
		mouse_x_save = MMAX(mouse_x_save, 0);
		mouse_y_save = MMAX(mouse_y_save, 0);
		mouse_x_save = MMIN(mouse_x_save, IMAGE_WIDTH_E-1);
		mouse_y_save = MMIN(mouse_y_save, IMAGE_HEIGHT_E-1);
		
		Mouse_Position_Change(0, x, y,  IMAGE_WIDTH_E, IMAGE_HEIGHT_E);
		//Mouse_Move(0, x, y,  IMAGE_WIDTH_E, IMAGE_HEIGHT_E);
		//printf("in SWF_MSG_MOVE---x=%d,y=%d\n",x,y);
	}

	if(msg->subtype != SWF_MSG_WHEEL)
	{
		///x = GetMouseX();
		///y = GetMouseY();
		x = mouse_x_save;
		y = mouse_y_save;
		pos = (x<<16) | (y&0xffff);
	}
	
	//printf("---t=%d,x=%d,y=%d,p1=%d,p2=%d\n",msg->subtype,x,y,(pos>>16)&0xffff,pos&0xffff);
	SWF_Message(NULL,msg->subtype,(void*)pos);
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
int mouse_draw_mainloop(int msgid)
{
	int fdmouse;
	int nread,nwr;
	char ps2data[4];
	struct am_sys_msg msg;
	int pos,x,y,x_off,y_off;
	static int prev_x,prev_y;
	
	fdmouse = open("/dev/mice",O_RDONLY);
	if(fdmouse<0){
		printf("open mouse device failed\n");
		return 0;
	}

	prev_x = GetMouseX();
	prev_y = GetMouseY();
	
	while(1)
	{
		nread=read(fdmouse, ps2data, sizeof(ps2data));
		if(nread==sizeof(ps2data))
		{
			_mouse_data_translate(ps2data,&msg);
			pos = msg.dataload;
			///x = GetMouseX();
			///y = GetMouseY();
			x = prev_x;
			y = prev_y;
			x_off = pos>>16;
			y_off = (pos<<16)>>16;
			
			if(msg.subtype == SWF_MSG_MOVE 
				|| msg.subtype >= SWF_MSG_LDRAG)
			{
				x+=x_off*2;
				y+=y_off*2;

				prev_x = x;
				prev_y = y;
				prev_x = MMAX(prev_x, 0);
				prev_y = MMAX(prev_y, 0);
				prev_x = MMIN(prev_x, IMAGE_WIDTH_E-1);
				prev_y = MMIN(prev_y, IMAGE_HEIGHT_E-1);
				
				Mouse_Move(0, x, y,  IMAGE_WIDTH_E, IMAGE_HEIGHT_E);
			}
			msgsnd(msgid, &msg, sizeof(struct am_sys_msg),IPC_NOWAIT);
		}
		
		
	}
	return 0;
	
}

