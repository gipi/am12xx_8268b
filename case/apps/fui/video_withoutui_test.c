#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <dlfcn.h>
#include <memory.h>
#include "swf_ext.h"
#include "video_engine.h"
#include "fui_common.h"
#include "sys_cfg.h"
#include "display.h"
#include "apps_vram.h"
#include "lcm_op.h"
#include "system_info.h"
#include "osapi.h"
#include "sys_buf.h"
#include <fcntl.h>
#include <sys/poll.h>

#define MAX_FILE_NUM	3

char filename[MAX_FILE_NUM][64]=
{
	"/mnt/udisk/qq.avi",
	"/mnt/udisk/qq.avi",
	"/mnt/udisk/qq.avi",
};

int fileindex=0;

typedef void *(*FUNC_OPEN)(void *param);
typedef int (*FUNC_CMD)(void *handle, video_cmd_s *cmd);
typedef int (*FUNC_CLOSE)(void *handle,void *param);

static FUNC_OPEN DecOpen = NULL;
static FUNC_CMD DecCmd = NULL;
static FUNC_CLOSE DecClose = NULL;

void **mq_buf = NULL;
void *pq = NULL;
unsigned int preview_flag=0;


unsigned int loopflag=0;
static unsigned long heap_bus_addr;
static int heap_size = _VIDEO_HEAP1_SIZE;
static void *heap_mem_handle = NULL;
static unsigned long heap_mem_ptr;
file_info_s media_file_info;
extern void* deinst;
typedef enum EVENT_KEY_S
{
	PLAY,
	PAUSE,
	STOP,
	SEEK_UP,
	SEEK_BACK,
	SEEK_CONTINUE_UP,
	SEEK_CONTINUE_BACK,
	NEXT,
	PREV,
}EVENT_KEY_E;

typedef enum {
	KEY_TYPE,
	VP_QUIT,
	VP_RETURN_LIST,
}EVENT_TYPE_E;

typedef struct EVENT_MSG_S
{
	EVENT_TYPE_E type;
	EVENT_KEY_E  keymsg;
}EVENT_MSG_T;


int send_event_msg(void *handle, EVENT_MSG_T *pmsg)
{
	EVENT_MSG_T *pevent = NULL;
	if(!handle || !pmsg)
		return -1;
	pevent = OSmalloc(sizeof(EVENT_MSG_T));
	if(!pevent)
		return -1;
	*pevent = *pmsg;
	OSQPost(handle, pevent);	
	return 0;
}

int receive_event_msg(void *handle, EVENT_MSG_T *pmsg)
{
	EVENT_MSG_T *pevent = NULL;

	if(!handle || !pmsg)
		return -1;
	pevent = (EVENT_MSG_T *)OSQPend(handle, 0);	
	if (pevent)
	{
		*pmsg = *pevent;
		OSfree(pevent);
	}
	return 0;
}

int open_lcm()
{
	DE_config ds_conf;

	de_init(&deinst);
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);

	screen_output_data.screen_output_width = ds_conf.input.width;
	screen_output_data.screen_output_height = ds_conf.input.height;
	screen_output_data.screen_output_mode = 16;
	screen_output_data.screen_output_true = 1;
	
	ds_conf.input.enable=0;

	
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}

int insmod_2d()
{
	int err;
	char cmd[256];

	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"driver_load_2d.sh");
	err = system(cmd);

	return err;
}
void key_proc_cmd(int key)
{
	EVENT_MSG_T msg;
	int quit = 0;
	switch(key)
	{
		case 0x125:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = STOP;
				send_event_msg(pq, &msg);
			}
			quit = 1;
			break;
		case 0x126:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = PLAY;
				send_event_msg(pq, &msg);
			}
			break;
		case 0x127:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = PAUSE;
				send_event_msg(pq, &msg);
			}
			break;
		case 0x128:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = SEEK_BACK;
				send_event_msg(pq, &msg);
			}
			break;
		case 0x11b:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = SEEK_UP;
				send_event_msg(pq, &msg);
			}
			break;
		case 0x10d:
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = SEEK_CONTINUE_UP;
				send_event_msg(pq, &msg);
			}
			break;
		case 'c':
		case 'C':
			if(pq)
			{
				msg.type = KEY_TYPE;
				msg.keymsg = SEEK_CONTINUE_BACK;
				send_event_msg(pq, &msg);
			}
			break;	
		default:
			OSprintf("invalid command:%c\n", key);
	}
}

void get_palyer_msg(notify_msg_e msg)
{
	switch(msg){
		case NO_SUPPORT_MSG:
			OSprintf("file not support\n");
			break;
		case NEED_STOP_MSG:
			{
				EVENT_MSG_T event;
				event.type = VP_QUIT;
				send_event_msg(pq, &event);
				break;
			}
		case STOP_OK_MSG:
			{
				EVENT_MSG_T event;
				event.type = VP_RETURN_LIST;
				send_event_msg(pq, &event);
				break;
			}
		case PREVIEW_OK_MSG:
			{
				OSprintf("preview ok\n");
				break;
			}
		default:
			break;
	}
}


static int read_packet(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)OSfread(buf, sizeof(char), buf_size, opaque);
}

static int write_packet(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)OSfwrite(buf, sizeof(char), buf_size, opaque);
}

static int file_seek_set(void *opaque, int64 offset)
{
	return OSfseek64_set(opaque, offset);
}

static int file_seek_end(void *opaque, int64 offset)
{
	return OSfseek64_end(opaque, offset);
}

static int64 get_pos(void *opaque)
{
	return OSftell64(opaque);
}


static int _test_video_get_seekable(void *opaque)
{
	return 1;
}

int sysbuf_phy_viraddr(unsigned int* phy_addr,unsigned int* vir_addr,unsigned long size)
{
	struct mem_dev share_heap;
	int memfd;
	int err;
	share_heap.request_size = size;
	share_heap.buf_attr = CACHE;
	
	memfd = open("/dev/sysbuf",O_RDWR);
	if(memfd == -1)
	{
		printf("open /dev/sysbuf error\n");
		return -1;
	}
	
	err = ioctl(memfd,MEM_GET,&share_heap);
	if(err == -1){
		printf("fui get basic heap error\n");
		close(memfd);
		return -1;
	}
	*vir_addr = share_heap.logic_address;
	*phy_addr = share_heap.physic_address;
	printf("vir = %x,phy = %x\n",share_heap.logic_address,share_heap.physic_address);
	close(memfd);
	return 0;
}
int video_play_thread(void)
{
	void *handle=NULL;
	video_cmd_s cmd1, *cmd=&cmd1;
	void *mem = NULL;
	void *phmem = NULL;
	int ret,mem_size;
	
	void *dllhandle=NULL;
	memset(cmd, 0, sizeof(video_cmd_s));
	
	dllhandle = OSDLOpen("libvideo_player.so");
	if(dllhandle)
	{
		DecOpen = (FUNC_OPEN)OSDLGetProcAddr(dllhandle,"video_dec_open");
		DecCmd = (FUNC_CMD)OSDLGetProcAddr(dllhandle,"video_dec_cmd");
		DecClose = (FUNC_CLOSE)OSDLGetProcAddr(dllhandle,"video_dec_close");	
	}
	else
		OSprintf("can not open video_player.so\n");

	handle = DecOpen(NULL);
	if (!handle)
	{
		goto end;
	}
	mq_buf = (void**)OSmalloc(sizeof(void*)*2048);
	pq = OSQCreate(mq_buf, 2048);
play:	
	cmd->con = V_SET_NOTIFY_FUNC;
	cmd->par.notify_func =get_palyer_msg;	
	DecCmd(handle, cmd);

	cmd->con = V_SET_FILE;
	cmd->par.set_file.fileflag =0;
	OSstrcpy(cmd->par.set_file.filename, filename[fileindex]);	
	cmd->par.set_file.f_io.open = OSfopen;
	cmd->par.set_file.f_io.read = (void*)read_packet;
	cmd->par.set_file.f_io.write = (void*)write_packet;
	cmd->par.set_file.f_io.seek_set = (void*)file_seek_set;
	cmd->par.set_file.f_io.seek_end = (void*)file_seek_end;
	cmd->par.set_file.f_io.tell = get_pos;
	cmd->par.set_file.f_io.close = OSfclose;	
	cmd->par.set_file.f_io.get_length = NULL;
	cmd->par.set_file.f_io.set_stop = NULL;
	cmd->par.set_file.f_io.get_seekable = _test_video_get_seekable;
	
	memset(&media_file_info,0,sizeof(file_info_s));
	cmd->par.set_file.pInfo = &media_file_info;	
	DecCmd(handle, cmd);

	cmd->con = V_SET_PARAM;
	
	cmd->par.set_par.frame_w = screen_output_data.screen_output_width; //800
	cmd->par.set_par.frame_h = screen_output_data.screen_output_height; //600;
	cmd->par.set_par.display_mode = VIDEO_DISPLAY_MODE_LETTER_BOX;
	cmd->par.set_par.pix_fmt = VIDEO_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;

	if(preview_flag)
		cmd->par.set_par.is_preview=1;
	else
		cmd->par.set_par.is_preview=0;

	if(!loopflag)
		sysbuf_phy_viraddr((unsigned int*)&heap_bus_addr, (unsigned int*)&heap_mem_ptr,heap_size);
	cmd->par.set_par.p_heap.h_b_size = 1;	
	cmd->par.set_par.p_heap.vir_addr[0] = (unsigned char*)heap_mem_ptr;
	cmd->par.set_par.p_heap.bus_addr[0] = (unsigned int)heap_bus_addr;
	cmd->par.set_par.p_heap.size[0] = heap_size;
	DecCmd(handle, cmd);
	
	if(preview_flag)
	{
		prev_entry p_e[4];
		unsigned int i;
		
		cmd->con  = V_GET_PERVIEW;
		cmd->par.prev_par.prev_num = 4;
		
		for(i=0;i<4;i++)
		{
			p_e[i].pre_time =0+2000*i;
			p_e[i].prev_w = 96;
			p_e[i].prev_h = 96;
			p_e[i].vir_addr = OSmalloc(96*96*2);
			cmd->par.prev_par.pprev[i] = &p_e[i];
		}
		DecCmd(handle, cmd);	
		
		while(preview_flag)
		{
			OSSleep(20);	
		}
		
		OSprintf("pre w:%d h:%d\n",p_e[0].actual_pre_w,p_e[0].actual_pre_h);
		
		goto play;
	}
	
	if(1)
	{
		cmd->con = V_PLAY;
		DecCmd(handle, cmd);
	}
	
	while(1)
	{
		EVENT_MSG_T event;
		int incr;

		receive_event_msg(pq, &event);	
		printf("receive cmd:%d\n",event.keymsg);
		switch(event.type)
		{
			case KEY_TYPE:
			{
				switch(event.keymsg)
				{
					case PLAY:
						cmd->con = V_PLAY;
						DecCmd(handle, cmd);
						break;
					case PAUSE:
						cmd->con = V_PAUSE;
						DecCmd(handle, cmd);
						break;
					case STOP:
						cmd->con = V_STOP;
						DecCmd(handle, cmd);
						break;
					case SEEK_UP:
						incr = 200;
						goto do_seek;
					case SEEK_BACK:	
						incr = -200;
					do_seek:
						{
							unsigned int cur_time;
							cmd->con = V_GET_CUR_TIME;
							DecCmd(handle, cmd);
							cur_time = cmd->par.cur_time;
							cmd->con = V_SEEK_SINGLE;
							cmd->par.seek_single_time = cur_time+incr;
							OSprintf("-------------seek time[%d+%d]\n",cur_time,incr);
							if(cmd->par.seek_single_time < 0)
								cmd->par.seek_single_time = 0;
							DecCmd(handle, cmd);
							break;
						}
					case SEEK_CONTINUE_UP:
						incr = 200;
						goto do_continue_seek;
					case SEEK_CONTINUE_BACK:
						incr = -200;
						goto do_continue_seek;
					do_continue_seek:
						{
							cmd->con = V_SEEK_CONTINUE;
							cmd->par.seek_continue_step = incr;
							DecCmd(handle, cmd);
							break;
						}
					default:
						break;
				}
				break;
			}
			case VP_QUIT:
				cmd->con = V_STOP;
				DecCmd(handle, cmd);
				break;
			case VP_RETURN_LIST:
				loopflag = 1;
				fileindex++;
				fileindex = fileindex % MAX_FILE_NUM;
				goto play;
			default:
				break;
		}
	}
end:
	OSprintf("main go to end\n");
	
	if(handle)
	{
		cmd->con = V_TERMINAL;
		DecCmd(handle, cmd);
		DecClose(handle,NULL);
	}
	if(pq)
		OSQDel(pq);
	if(mq_buf)
		OSfree(mq_buf);
	if(dllhandle)
		OSDLClose(dllhandle);
	return 0;
}

int video_test_main(int argc, char *argv[])
{
	int parent_msg_id;
	int rpipe;
	int flag;
	void *ptask=NULL;
	int err;
	struct pollfd fd;
	struct am_sys_msg msg;
	int rdnr=0;
	int ret;
	
	struct timeval value;
	unsigned int milisec;
	
	
	if(argc > 1){
		if(argc != 4){
			printf("error parameters\n");
			exit(1);
		}
		parent_msg_id = (int)strtol(argv[1],NULL,10);
		rpipe = (int)strtol(argv[2],NULL,10);
	}
	else{
		printf("fui arg err\n");
		exit(1);
	}

	fd.fd = rpipe;
	fd.events = POLLIN;

	/** insmod 2d */
	//insmod_2d();

	/** set lcm */
	if(open_lcm() == -1){
		printf("open lcm error\n");
		exit(1);
	}

	read_swfkey_cfgfile("/am7x/case/data/keymap/globalswfkey.bin");

	ptask = OSTaskCreate((void*)video_play_thread, NULL, 0, 0);
	OSTaskResume(ptask);

	while(1){
		err = poll(&fd,1,1);
		if(err != -1){
			if(err == 0){
				/** poll timeout */
				//printf("swf message timeout\n");
			}
			else{
				/** information received */
				if(fd.revents & POLLIN){
					rdnr = read(fd.fd,&msg,sizeof(struct am_sys_msg));
					if(rdnr == sizeof(struct am_sys_msg)){
						switch(msg.type){
							case SYSMSG_KEY:
							{
								int key;
								if((key = swf_key_read(&msg)) != SWF_MSG_NULL){
									printf("fui key == 0x%x\n",key);
									key_proc_cmd(key);
								}
							}
							break;
						}
					}
				}
			}

			
		}
	}
	
	if(ptask)
		OSTaskDel(ptask);
	return 0;
}

