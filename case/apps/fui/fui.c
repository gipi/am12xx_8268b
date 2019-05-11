#include <stdio.h>
#include <stdlib.h>
#include <file_list.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include "swfdec.h"
#include "fui_common.h"
#include "swf_types.h"
#include "sys_msg.h"
#include "sys_buf.h"
#include "display.h"
#include "sys_cfg.h"
#include "media_hotplug.h"
#include "apps_vram.h"
#include <sys/msg.h>
#include "am7x_dac.h"
#include "buf_encoder.h"
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <ucontext.h>
#include <sys_pmu.h>
#include "sys_gpio.h"
#include "ezcast_public.h"
#if EZMUSIC_ENABLE
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif
#endif
/**
* for cache segment fault signals.
*/
#define FUI_DEBUG_EN

/**
* Clear key msg when there are too many.
*/
///#define FUI_CLEAR_KEY_MSG
#define FUI_READ_MSG_EACH_TIME 10

/**
* for video test without FUI
*/
//#define VIDEO_TEST_WITHOUTUI

/**
* for FUI frame rate controll enable/disable.
*/
#ifndef FUI_TIME_DEBUG
#define FUI_FRAMERATE_ENABLE
#endif

#ifdef FUI_FRAMERATE_ENABLE

#define FUI_FRAMERATE_PROCESS_INDEPENDENT
int framerate_fds0[2];
int framerate_fds1[2];

#endif


#ifdef FUI_DEBUG_EN
#define __USE_GNU
#include <link.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <signal.h>
#include <ucontext.h>
#include <errno.h>

#endif

unsigned long swf_heap_logic_start=0;
unsigned long swf_heap_physic_start=0;
void *deinst=NULL;
int fui_image_width=800,fui_image_height=480;
screen_output_param screen_output_data;
int board_poweroff=0x07;
int board_poweron=28;
//int ir_poweroff=0x10;
int ir_poweroff=0x08;
int ir_poweron=0x19;
int fui_sys_fd[2];

#ifdef FUI_DEBUG_EN

#define CONFIG_FUI_DEBUG_RESET_SYSTEM 1

int wireStopMirrorCallback();

extern void createQRCodeForWire();

#if CONFIG_FUI_DEBUG_RESET_SYSTEM
#include <sys/mman.h>
static int fui_system_reset()
{
	/**
	* set watch dog and reset.
	*/
	#define WD_CTL (0xb0140000+0x14)
	int addr=WD_CTL;
	int fd;
	int size;
	unsigned int reg;
	int offset = addr - (addr&0xfffff000);
	int delay=1000;

	fprintf(stderr, "set system reset,addr=0x%x,offset=0x%x,mask=0x%x\n",addr,offset,addr&0xfffff000);

	fd = open("/dev/mem", O_RDWR | O_SYNC);	
	
	if(fd<0){
		fprintf(stderr, "open/dev/mem error\n");
		return -1;
	}
	else{
		fprintf(stderr, "open /dev/mem success %d\n",fd);
	}

	reg = (unsigned int)mmap(0, offset+8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr&0x1ffff000);
	if(reg == 0){
		fprintf(stderr, "mmap error when set watch dog\n");
		close(fd);
		return -1;
	}
	else{
		fprintf(stderr, "mmap 0x%x\n",reg);
	}
	fprintf(stderr, "watch dog reg=0x%x\n",reg+offset);
	*(unsigned int *)(reg+offset) = 0x74|(2<<1);
	while(delay>0){
		delay--;
	}
	fprintf(stderr, "watch dog value=0x%x\n",*(unsigned int *)(reg+offset));
	
	*(unsigned int *)(reg+offset) = 0x61|(2<<1);
	*(unsigned int *)(reg+offset) = 0x30|(2<<1);

	fprintf(stderr, "wait system restart .........\n");
	while(1);

	return 0;
	
}
#endif

/**
* for debug for segment fault.
*/
static void fui_signal_handler(int signum, siginfo_t* siginfo, ucontext_t *lpContext)
{
	if (signum == SIGIO || signum == SIGPIPE)
	{ //[Sanders.130423] EZ Stream error handling.
		printf("[FUI]: catch exception at signum(%d)!!\n", signum);
	}
	else
	{
		mcontext_t *mc = &lpContext->uc_mcontext;

		{//[Joan.20160222] for Analytics exception
			char exception[32];
			sprintf(exception,"0x%x",mc->pc);
			ezFuiAnalyticsReport(exception);
			sleep(5);
		}
		
		fprintf(stderr, "[FUI]: catch exception at 0x%x\n", mc->pc);
		fprintf(stderr, "siginfo->si_addr == 0x%x\n", siginfo->si_addr);
		fprintf(stderr, "stack = 0x%08x\n", lpContext->uc_stack.ss_sp);
		fprintf(stderr, "$29 = 0x%08x\n", mc->gregs[29]);

		fui_dl_show();
		
#if CONFIG_FUI_DEBUG_RESET_SYSTEM
		fui_system_reset();
#endif

		exit(1);
	}
}

static int register_segv_handler(int sig, void *handler)
{
	int ret = -EINVAL;
	sigset_t signal_set;
	if (sigemptyset(&signal_set) >= 0) {
		struct sigaction sa = {
			.sa_handler  = handler,
			.sa_mask     = signal_set,
			.sa_flags    = SA_SIGINFO,
			.sa_restorer = NULL
		};
		ret = sigaction(sig, &sa, NULL);
		if( ret < 0 )
			fprintf(stderr, "failed to install signal hander.\n");
	}
	return ret;
}

static int fui_dl_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	int j;
	
	printf("name=%s (%d segments)\n", info->dlpi_name,info->dlpi_phnum);
	printf("\t\t baseaddr 0x%x\n", (void *) (info->dlpi_addr));
	
	for (j = 0; j < info->dlpi_phnum; j++){
		printf("\t\t header %2d: address=%10p\n", j,(void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr));
	}

	return 0;
}

int fui_dl_show()
{
	dl_iterate_phdr(fui_dl_callback, NULL);

	return 0;
}

#endif


inline int gui_get_image_width()
{
	return fui_image_width;
}

inline int gui_get_image_height()
{
	return fui_image_height;
}

unsigned long fui_get_bus_address(unsigned long logicaddr)
{
	return swf_heap_physic_start + (logicaddr - swf_heap_logic_start);
}

unsigned long fui_get_virtual_address(unsigned long buaaddr)
{
	return swf_heap_logic_start + (buaaddr - swf_heap_physic_start);
}
static int fui_set_context()
{
	SWF_CONTEXTINFO ctxInfo;
	SWF_HEAPINFO heaps[3];
	SWF_FONTINFO fonts[3];
	struct mem_dev basic_heap,share_heap;
	int memfd;
	int err;
	static char font0_path[64],font1_path[64];

	/** 
	* get heap from system buffer 
	*/
	share_heap.request_size = HEAP_SHARE_SIZE + HEAP_BASIC_SIZE;
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


	/** 
	* for lcm physical address transformation 
	*/
	swf_heap_logic_start = share_heap.logic_address;
	swf_heap_physic_start = share_heap.physic_address;

	ctxInfo.heaps = heaps;
	ctxInfo.heaps[0].heap_addr = swf_heap_logic_start;
	ctxInfo.heaps[0].heap_size = HEAP_BASIC_SIZE;
	ctxInfo.heaps[0].heap_type = HEAP_TYPE_BASIC;
	ctxInfo.heaps[0].heap_base_virtual_addr = swf_heap_logic_start;
	ctxInfo.heaps[0].heap_base_phy_addr = swf_heap_physic_start;

	ctxInfo.heaps[1].heap_addr = swf_heap_logic_start + HEAP_BASIC_SIZE;
	ctxInfo.heaps[1].heap_size = HEAP_SHARE_SIZE;
	ctxInfo.heaps[1].heap_type = HEAP_TYPE_SHARE;
	ctxInfo.heaps[1].heap_base_virtual_addr = swf_heap_logic_start;
	ctxInfo.heaps[1].heap_base_phy_addr = swf_heap_physic_start;
	ctxInfo.heaps[2].heap_addr = 0;

	ctxInfo.defaultFunc.decode_bitmap = (void*)fui_decode_bitmap;
	ctxInfo.defaultFunc.play_audio = (void*)fui_play_audio;
	ctxInfo.defaultFunc.release = (void*)fui_stop_audio;

	ctxInfo.fonts = fonts;
	sprintf(font0_path,"%s/%s",AM_CASE_DAT_DIR,"U16.bin");
	sprintf(font1_path,"%s/%s",AM_CASE_DAT_DIR,"U24.bin");
	ctxInfo.fonts[0].font_path = font0_path;
	ctxInfo.fonts[0].font_size = 16;
	ctxInfo.fonts[1].font_path = font1_path;
	ctxInfo.fonts[1].font_size = 24;
	ctxInfo.fonts[2].font_path = NULL;

	/*
	* ok, we let the flash player to manage the frame buffers.
	*/
	ctxInfo.frameBuffer[0] = 0;
	ctxInfo.frameBuffer[1] = 0;
#ifndef FRAMECOPY_RECT_ENABLE
	ctxInfo.frameNumber = 4;
#else
	ctxInfo.frameNumber = 2;
#endif
	ctxInfo.frameWidth  = IMAGE_WIDTH_E;
	ctxInfo.frameHeight = IMAGE_HEIGHT_E; 

	SWF_Context_Init((void *)&ctxInfo);

	return 0;
	
}

static int fui_open_lcm()
{
	DE_config ds_conf;
	struct sysconf_param sys_cfg_data;

	de_init(&deinst);
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);

	screen_output_data.screen_output_width = ds_conf.input.width;
	screen_output_data.screen_output_height = ds_conf.input.height;
	screen_output_data.screen_output_mode = 16;
	screen_output_data.screen_output_true = 1;
	
	ds_conf.input.enable=0;

	/**
	* save the image width and height.
	*/
#ifndef FUI_TIME_DEBUG
	fui_image_height = ds_conf.input.height;
	fui_image_width = ds_conf.input.width;
#else
	fui_image_height = 480;
	fui_image_width = 800;
#endif

	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);

	if(apps_vram_read(VRAM_ID_SYSPARAMS,(void*)&sys_cfg_data,sizeof(struct sysconf_param))!=0){
		printf("%s,%d:Get environment paras err\n",__FILE__,__LINE__);
		return -1;
	}
	else
		set_back_light(sys_cfg_data.backlight_strength);
	
	return 0;
}

/**
static int fui_insmod_2d()
{
	int err;
	char cmd[256];

	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"driver_load_2d.sh");
	err = system(cmd);

	return err;
}
*/

/**
* for test purpose.
* @brief swf-player pratical frame rate statistics.
*/
/**
static void fui_framerate_statistic()
{
	static int cnt=0;
	struct timeval tv;
	int err;
	static int sec=0;
	static int usec=0;
	static int total_msec=0;


	err = gettimeofday(&tv,NULL);
	if(err == -1){
		cnt = 0;
		total_msec = 0;
		return;
	}

	if((sec==0) && (usec==0)){
		sec = tv.tv_sec;
		usec = tv.tv_usec;
		cnt = 0;
		total_msec = 0;
		return;
	}
	else{
		cnt++;
		total_msec += (tv.tv_sec-sec)*1000+(tv.tv_usec-usec)/1000;
		sec = tv.tv_sec;
		usec = tv.tv_usec;
	}

	if(cnt==5){
		printf("rate == %d\n",1000/(total_msec/cnt));
		cnt = 0;
		total_msec = 0;
	}

	return;
}
*/
static int fui_apps_vram_init()
{
	struct sysconf_param syscfg;
	int err;
	
	/** load vram library */
	if(apps_vram_init()<0){
		printf("fui set vram default value\n");
		err = apps_vram_set_default();
	}
	else{
		err = apps_vram_read(VRAM_ID_SYSPARAMS, (void *)&syscfg, sizeof(struct sysconf_param));
	}

	return err;
}

static void fui_out(void)
{
	apps_vram_release();
}

static int fui_system_resource_init()
{
	struct sysconf_param sys_cfg_data;
	int sys_volume;
	fui_apps_vram_init();
	sys_volume_init();
	sys_volume_ctrl(_VOLUME_CTRL_GET,(void *)&sys_volume);
	
	apps_vram_read(VRAM_ID_SYSPARAMS,(void*)&sys_cfg_data,sizeof(struct sysconf_param));
	if(sys_cfg_data.sys_volume==100){
		sys_cfg_data.sys_volume=0;
		apps_vram_write(VRAM_ID_SYSPARAMS,(void*)&sys_cfg_data,sizeof(struct sysconf_param));
		apps_vram_store(VRAM_ID_SYSPARAMS);
	}
	if(sys_volume!=sys_cfg_data.sys_volume)
	{
		sys_volume=sys_cfg_data.sys_volume;
		sys_volume_ctrl(_VOLUME_CTRL_SET,(void *)&sys_volume);
		
	}
	if(sys_cfg_data.is_gloval_valid){
		board_poweroff = sys_cfg_data.glo_val.nums[5];
		board_poweron = sys_cfg_data.glo_val.nums[6];
		ir_poweroff = sys_cfg_data.glo_val.nums[7];
		ir_poweron = sys_cfg_data.glo_val.nums[8];
		printf("Board Power Key OFF is 0x%x, ON is %d\n",board_poweroff,board_poweron);
		printf("IR Power Key OFF is 0x%x, ON is 0x%x\n",ir_poweroff,ir_poweron);
	}
	return 0;
}
static int fui_load_swf(char* swfname)
{
	/** add first swf */
	int flag;
	char swfpath[64];
	SWF_DEC * inst;
	SWF_INSTINFO info;
	SWF_RECT window = {0, 0, IMAGE_WIDTH_E-1, IMAGE_HEIGHT_E-1};
	
	sprintf(swfpath,"%s/%s",AM_CASE_SWF_DIR,swfname);
	flag = SWF_LBOX_FLAG | SWF_STANDALONE;// |SWF_DEBUG |SWF_ECHO
	inst = SWF_AddInst(swfpath, &window, flag, &info);
	if(inst == NULL){
		printf("failed to open main.swf\n");
		exit(1);
	}
	return 0;
	
}

static int alarm_on_notice(struct am_sys_msg *msg_get)
{
	set_alarm_on_id((int)msg_get->dataload);
	SWF_Message(NULL, SWF_MSG_KEY_RING, NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	return 0;
}

int swapflag = 1;//0: no swap; 1: swap

int FUI_setSwapFlag(int flag)
{
	swapflag = flag;
	return 0;	
}

unsigned long _fui_get_cur_framebuf()
{
	DE_config ds_conf;
	unsigned long addr=0;
	de_get_config(deinst,&ds_conf,DE_CFG_IN);
	addr = (int)ds_conf.input.bus_addr;
	return fui_get_virtual_address(addr);
}

static int _fui_snapshot_save(int key)
{
	if(key==SWF_MSG_KEY_SNAPSHOT){
		char filename[128]="";
		rtc_date_t rtc_date;
		rtc_time_t rtc_time;
		unsigned long framebuf_addr=0;
		int filesize=0;
		int w=0,h=0;
		if(tm_get_rtc(&rtc_date,&rtc_time)!=0){
			printf("%s,%d:Get rtc Date Error!",__FILE__,__LINE__);
			return -1;
		}
		sprintf(filename,"%s%04d%02d%02d%02d%02d%02d.jpg","/mnt/udisk/",rtc_date.year,
				rtc_date.month,rtc_date.day,rtc_time.hour,rtc_time.min,rtc_time.sec);
		
		framebuf_addr = _fui_get_cur_framebuf();
		w = gui_get_image_width();
		h = gui_get_image_height();
		printf("Save Snapshot file=%s,bufaddr=0x%x,w=%d,h=%d\n",filename,framebuf_addr,w,h);
		//if(jpeg_encode((void*)framebuf_addr,filename,3,&filesize,w,h,4,1)!=0){
		if(jpeg_encode(filename,(void*)framebuf_addr,NULL,NULL,w*2,4,0,0,w,h,3,&filesize,1)!=0){
			printf("%s,%d:Save Snapshot Error!\n",__FILE__,__LINE__);
			return -1;
		}
		else{
			printf("%s,%d:Save Snapshot OK Size=0x%x!\n",__FILE__,__LINE__,filesize);
			return 0;
		}
			
	}
	return 0;
}

void _deal_test_key(int key)
{
	int effect = 0;
	int rtn=0;

	if(key==0x142)//PREV Key
		subtitle_set_groupidx(1);
	if(key==0x146)//REV Key
		subtitle_set_groupidx(0);
	

	if(key==0x144)///NEXT Key
		subtitle_set_subidx(1);
	if(key==0x143)///FWD Key
		subtitle_set_subidx(0);
	
	if(key==0x128){
		unsigned int mode;
		#if 0
		/* background effect test**/
		static int rotatedegree=1;
		effect = get_photo_background_effect();
		effect ++;
		if(effect>5)
			effect = 0;
		set_photo_background_effect(5);
		printf("Effect ====%d",effect);
		#endif

		#if 0
		/**rotation test**/
		img_store_photo_exif("/mnt/udisk/1.jpg",0,rotatedegree);
		if(rotatedegree==1)
			rotatedegree=2;
		else if(rotatedegree==2)
			rotatedegree = 5;
		else if(rotatedegree==5)
			rotatedegree = 1;
		#endif

		#if 0
		/**Udisk format test**/
		printf("Format Udisk!!!sssssStart\n");
		{
			int *test;	
			test = (int*)malloc(22); 
			if(test){
				memset(test,0,24);
			
				printf("$$$$$$$$$$$$$$$$$$$$$$test=0x%x",test);
				free(test);
				*test=0x121212;
			}
			
		}
		//udisk_format_d();
		//send_msg_to_keydriver();
		
		#endif
		//osdengine_test();
	}
}

static void _fui_deal_special_key(int key)
{
	//fui_deal_vol_key(key);	
	_fui_snapshot_save(key);
	_deal_test_key(key);
}

static void check_power_key(struct am_sys_msg *msg)
{
	static int long_press = 0;
	static int press_down = 0;
	int key_value,key_type;
	int val = 0;
	key_value = msg->dataload & 0xFF;
	key_type = msg->dataload >> 24;
	

	if(key_value==ir_poweroff || key_value==board_poweroff){
		if(key_type==2){
			press_down = 1;
			long_press = 0;
		}
		printf("   press_down=%d  long_press=%d  key_type=%d\n",press_down,long_press,key_type);
		if(press_down==1 && key_type==3){
			long_press++;
		}
		else if(key_type==1){
			printf("number of long press is %d\n",long_press);
#if EZMUSIC_ENABLE
			if(long_press>=2){
				printf("--------------------------------------------------------------\n");
				printf("   WiFi Music:Power key is pressed!\n");
				printf("--------------------------------------------------------------\n");
				SWF_Message(NULL, SWF_MSG_KEY_EXT_POWER, NULL);
				SWF_Message(NULL, SWF_MSG_PLAY, NULL);		
				
				wifi_subdisplay_end();				
				
				#if MODULE_CONFIG_AUDIO_LED
				#if MODULE_CONFIG_AUDIO_EN
				get_gpio(MODULE_CONFIG_AUDIO_EN, &val);
				if(0==val)
					set_gpio(MODULE_CONFIG_AUDIO_LED,1);
				#endif
				#endif
				
				#if MODULE_CONFIG_SPDIF2_1_LED
				#if MODULE_CONFIG_SPDIF_EN
				get_gpio(MODULE_CONFIG_SPDIF_EN, &val);
				if(0==val)				
					set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
				#endif
				#endif
				
				#if MODULE_CONFIG_SPDIF5_1_LED
				#if MODULE_CONFIG_SPDIF_EN
				get_gpio(MODULE_CONFIG_SPDIF_EN, &val);
				if(0==val)			
					set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
				#endif
				#endif
				
				#if MODULE_CONFIG_I2S_LED
				#if MODULE_CONFIG_I2S_EN
				get_gpio(MODULE_CONFIG_I2S_EN, &val);
				if(0==val)			
					set_gpio(MODULE_CONFIG_I2S_LED,1);
				#endif	
				#endif	
				sleep(5);
				set_gpio(GPIO_WIFI_ON_LED,1);//WIFI-LED OFF	
				set_gpio(GPIO_POWER_ON_LED,1);//PWR-LED OFF				
				
			}
#else
			if(long_press>15){
				printf("number of long press is %d, > 15\n",long_press);
				if(key_value==ir_poweroff){
					printf("now is in IR POWER\n");
					//auto_power_off(IRE_DC_MODE);
				}
				else if(key_value==board_poweroff){
					printf("now is in BOARD POWER\n");
					//auto_power_off(EXT_DC_MODE);
				}				
				/* Mos: fix issue https://mantis.actions-micro.com/view.php?id=17677 */
                system("reboot");
				//SWF_Message(NULL, SWF_MSG_KEY_EXT_POWER, NULL);
				//SWF_Message(NULL, SWF_MSG_PLAY, NULL);
			}
#endif

			press_down = 0;
			long_press = 0;
		}
	}

}

#ifdef FUI_FRAMERATE_ENABLE

static int _fui_get_current_highest_framerate()
{
	int framerate = 30;

	framerate = SWF_GetCurretHighestFramerate();
	if(framerate > 0){
		return framerate;
	}
	else{
		return 0;
	}
}
static int _fui_create_framerate_control()
{
	int ret;
	pid_t pid;

	/**
	* framerate_fds0: a pipe from parent to child
	* framerate_fds1: a pipe from child to parent
	*/

	if(pipe(framerate_fds0) < 0){
		printf("This should not happen for frame-rate control situations!!!!!!");
		exit(1);
	}

	if(pipe(framerate_fds1) < 0){
		printf("This should not happen for frame-rate control situations!!!!!!");
		exit(1);
	}

	pid = fork();
	if(pid == -1){
		printf("create framerate error,exit()\n");
		exit(-1);
	}
	else if(pid == 0){
		/**
		* child process.
		*/
#ifdef FUI_FRAMERATE_PROCESS_INDEPENDENT
		char rpipe[16];
		char wpipe[16];
		char full_path[32];
		int ret;
		
		close(framerate_fds0[1]);
		close(framerate_fds1[0]);

		sprintf(rpipe,"%d",framerate_fds0[0]);
		sprintf(wpipe,"%d",framerate_fds1[1]);
		sprintf(full_path,"%s","/am7x/bin/framectrl.app");
		ret = execl(full_path,"framectrl.app",rpipe,wpipe,NULL);
		if(ret == -1){
			printf("create frame control process error\n");
		}

		return 0;
#else
		fd_set wfds,rfds;
		struct timeval tv;
		int retval;
		int filedes[2];
		char dummy=0;
		int maxfd;
		int framerate=30;

		close(framerate_fds0[1]);
		close(framerate_fds1[0]);
		
		if(pipe(filedes) < 0){
			printf("framerate child process pipe error\n");
			goto _FRAMERATE_OUT;
		}

		close(filedes[1]);

		maxfd = filedes[0];

		if(framerate_fds0[0] > maxfd){
			maxfd = framerate_fds0[0];
		}

		while(1){

			FD_ZERO(&wfds);
			FD_ZERO(&rfds);
			FD_SET(filedes[0], &wfds);
			FD_SET(framerate_fds0[0], &rfds);

			tv.tv_sec = 0;
			tv.tv_usec = 1000*(1000/framerate);

			retval = select(maxfd+1,&rfds,&wfds, NULL, &tv);

			if(retval==0){
				/** timeout happens*/
				write(framerate_fds1[1],(void *)&dummy,sizeof(char));
			}
			else if(retval > 0){
				if(FD_ISSET(framerate_fds0[0],&rfds)){
					int nread;
					int rate;
					/**
					* adjust the framerate.
					*/
					nread = read(framerate_fds0[0],&rate,sizeof(int));
					if(nread == sizeof(int)){
						framerate = rate;
						///printf("do auto framerate adjustment,%d\n",framerate);
					}
				}
			}
		}

_FRAMERATE_OUT:
		return 0;
#endif

	}
	else{
		/** 
		* parent process.
		*/
		close(framerate_fds0[0]);
		close(framerate_fds1[1]);
		
		return 0;
	}
	
	return 0;
}
#endif

static int fui_system_init(){
	FILE *fp = NULL;
	int ret;

	fp = fopen("/tmp/pthsystem_pipe_fd", "r");
	if(fp < 0){
		printf("[%s-%d] open pipe tmp file error\n", __func__, __LINE__);
		perror("ERROR");
		fui_sys_fd[0] = fui_sys_fd[1] = -1;
		return -1;
	}

	ret = fread((void *)fui_sys_fd, sizeof(int), 2, fp);
	if(ret <= 0){
		printf("[%s-%d] read pipe tmp file error\n", __func__, __LINE__);
		perror("ERROR");
		fui_sys_fd[0] = fui_sys_fd[1] = -1;
		return -1;
	}
	printf("read pipe: %d, write pipe: %d\n", fui_sys_fd[0], fui_sys_fd[1]);

	return 0;
}

#define CMD_MAX_LEN		256
pthread_mutex_t fui_system_lock;

static int fui_mutex_init(){

	pthread_mutex_init(&fui_system_lock, NULL);

	return 0;
}
/*
	fix fui_cmd bug:
		for slow system cmd , fui -> pthsystem  takes some times,
		better wait  to  wait cmd end,  such as  cmd: "wpa_supplicant" has clild cmd
		(RTL871X: set bssid:85f4de5a  ioctl[SIOCSIWAP]: Operation not permitted)
		fui_sysyem  may return  while  child cmd still uncompleted.
		argv:
		int waittime: sleep time /s
*/
int fui_system(char *cmd,int waittime ){
	char buff[CMD_MAX_LEN+5];
	int *len = (int *)&buff[1];
	int ret = -1;
	int empty_lock;

	if(fui_sys_fd[1] < 0){
		printf("[%s-%d] pipe error[%d], please fix me\n", __func__, __LINE__, fui_sys_fd[1]);
		return -1;
	}

	printf("fui_system wait\n");	
	pthread_mutex_lock(&fui_system_lock);
	
	buff[0] = 'l';
	*len = strlen(cmd);
	if(*len >= CMD_MAX_LEN){
		printf("[%s-%d] cmd is too length[%d], please fix me\n", __func__, __LINE__, *len);
		ret = -1;
		goto __END__;
	}
	snprintf(&buff[5], CMD_MAX_LEN, "%s", cmd);
	ret = write(fui_sys_fd[1], buff, sizeof(buff));
	if(ret <= 0){
		printf("[%s-%d] write command fail\n", __func__, __LINE__);
		perror("ERROR");
		ret = -1;
		goto __END__;
	}

	memset(buff, 0, sizeof(buff));
	ret = read(fui_sys_fd[0], buff, sizeof(buff));
	if(ret < 0){
		printf("[%s-%d] read return value fail\n", __func__, __LINE__);
		perror("ERROR");
		ret = -1;
		goto __END__;
	}
	if(buff[0] == 'r'){
		int *val = &buff[1];
		ret = *val;
		goto __END__;
	}
	
	ret = -1;
__END__:
	pthread_mutex_unlock(&fui_system_lock);
	if(waittime>0){
		printf("slow system cmd,wait %d s\n!!!!",waittime);
		sleep(waittime);
	}
	printf("fui_system end\n");
	return ret;
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
pthread_mutex_t watchdog_mutex 	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t watchdog_cond 	= PTHREAD_COND_INITIALIZER;
#define IOCTL_WATCHDOG 25
static void wait_period (int period)
{
	struct timeval now;
	struct timespec outtime;
	pthread_mutex_lock(&watchdog_mutex);
	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + period;
	outtime.tv_nsec = 0;
	printf("wait to time out\n");
	pthread_cond_timedwait(&watchdog_cond, &watchdog_mutex, &outtime);
	pthread_mutex_unlock(&watchdog_mutex);
}

void *
soft_watchdog_routine(data)
	void *data;
{
	int fd;
	
	printf("Start Watchdog!\n");
	while(1)
	{
		printf("Watch Dog: Sleep 30s.\n");
		/*Send msg to scaler*/
		fd = open("/dev/uartcom",O_RDWR);
		if(fd < 0){
			printf("cant open uartcom device\n");
		}else{
			ioctl(fd,IOCTL_WATCHDOG,NULL); //for watchdog
			close(fd);
		}
		/*Wait period*/
		wait_period ((int)data);
	}
	return (NULL);
}

static int soft_watchdog_create()
{
	pthread_t tid;
	int ret;
	
	ret = pthread_create(&tid, NULL, (void *)soft_watchdog_routine, (void *)10);
	if(0 != ret) {
		printf("Watchdog dead.\n");
		return -1;
	}

	return 0;
}
#endif

#if FUN_WIRE_ENABLE
char gpio26 = 1;
int isAM8251W(){

#if IS_SNOR_FLASH
	return 0;
#endif

	if(gpio26 == 0)
		return 0;
	else
		return 1;
}

/*
**	GPIO30 is Quick-Charge switch key.
**	GPIO31 is Quick-Charge hardware control pin, high is mirror mode, low is Quick-Charge mode.
**	GPIO25 is LED1(Mirror mode) control pin, low is LED light.
**	GPIO29 is LED2(QC mode) control pin, low is LED light.
*/
int isQCMode()
{
	int val = !(reg_readl(0xb01c0008)&(0x1<<31));
	return val;
}
	
void QCKeyCheck()
{
	static int gpio30 = 1;
	if(isAM8251W() != 0){
		char gpio = 1;
		get_gpio(30, &gpio);
		if(gpio30 != gpio){
			gpio30 = gpio;
			if(gpio30 == 0){
				int val = isQCMode();
				set_gpio(31, (char)val);

				if(val)
				{
					set_gpio(29, 1);
					set_gpio(25, 0);
				}
				else
				{
					set_gpio(25, 1);
					set_gpio(29, 0);
				}
				
				set_gpio(94, 0);
				__usleep(1000000);
				set_gpio(94, 1);
			}
		}
	}
}

void ezwireInit()
{
	EZCASTLOG("EZWIRE_TYPE: %d, AM8252N_MIRAPLUG: %d\n", EZWIRE_TYPE, AM8252N_MIRAPLUG);
	//ezcastWriteDevName("EZWire");

#if (EZWIRE_TYPE == AM8251_EZWIRE)
	if(am_soc_get_chip_id() == CHIP_ID_AM8252){
		printf("\n***********************************************************\n");
		EZCASTWARN("Do not support this IC[%d]!!!\n", am_soc_get_chip_id());
		printf("***********************************************************\n");
		while(1){
			sleep(1);
		}
	}
#endif
	get_gpio(26, &gpio26);
	EZCASTLOG("gpio26: %d\n", gpio26);

#if EZWIRE_TYPE == AM8251_EZDOCKING		// Set mirror mode, disable Quick-Charge default.
	set_gpio(31, 1);
	set_gpio(29, 1);
	set_gpio(25, 0);
#endif

	adbAudioInit();
	system("insmod /lib/modules/2.6.27.29/am7x_hcd_next.ko");
#if defined(MODULE_CONFIG_USB1_DISABLE) && (MODULE_CONFIG_USB1_DISABLE==1)
	system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
#endif
	usleep(100*1000);
	system("insmod /lib/modules/2.6.27.29/am7x_udc.ko");
#if (EZWIRE_TYPE != AM8252N_MIRAWIRE && EZWIRE_TYPE != AM8252_MIRALINK && \
		EZWIRE_TYPE != AM8252N_MIRALINK && EZWIRE_TYPE != AM8252N_MIRALINE && EZWIRE_TYPE != AM8256_MIRALINE)
	usleep(100*1000);
	system("modprobe /lib/modules/2.6.27.29/snd-usb-audio.ko");
#endif

#if (EZWIRE_TYPE == AM8252N_MIRAWIRE || EZWIRE_TYPE == AM8252_MIRALINK ||  \
		EZWIRE_TYPE == AM8252N_MIRALINK ||  EZWIRE_TYPE == AM8252N_MIRALINE ||  \
		EZWIRE_TYPE == AM8256_MIRALINE ||  EZWIRE_TYPE == AM8252N_MIRAPLUG)		// close some clock.
#ifndef CMU_DEVCLKEN
	#define CMU_DEVCLKEN 0xB0010080
#endif

	int val = reg_readl(CMU_DEVCLKEN);
#if EZWIRE_TYPE == AM8252_MIRALINK
	val &= ~(0x1 << 10);
#else
	val &= ~(0x1 << 9);
#endif
	val &= ~(0x1 << 11);
	val &= ~(0x1 << 18);
	val &= ~(0x1 << 19);
	val &= ~(0x1 << 28);
	reg_writel(CMU_DEVCLKEN, val);
#ifndef LCD_DAC_CTL
#define LCD_DAC_CTL	0xb00f0028
#endif
	val = reg_readl(LCD_DAC_CTL);
	val &= ~0x1;
	reg_writel(LCD_DAC_CTL, val);
#endif
	
#if EZWIRE_USB_DEVICE_ENABLE	// Only EZWire and EZDocking support PC connect.
	{
		system("insmod /lib/modules/2.6.27.29/g_ether.ko");
		system("ifconfig usb0 192.168.203.1 up");
		system("udhcpd /etc/udhcpd_usb.conf");
		ezwireAddUsbPort(1);
	}
#endif

	system("thttpd -C /etc/thttpd.conf -u root");

	//ezConfigSetDevname("ezcast", strlen("ezcast"));
	//ezConfigSetDlnaId("ezcast");
	//ezConfigSetAirplayId("ezcast");
	ezFuiSetAudioLineIn(0);
#if 0//EZWIRE_TYPE == 3
	int mfd = open("/tmp/MiraWirePlus", O_CREAT|O_RDWR);
	if(mfd >= 0)
		close(mfd);
#endif

#if EZWIRE_CAPTURE_ENABLE
	if(access("/mnt/vram/ezcast/PLUGPLAYMODE", F_OK) != 0)
	{
		setWirePlugModeDefault();
	}
	ezFuiWireStopMirrorCallbackSet(wireStopMirrorCallback);
#endif

}
#endif

#if EZCAST_ENABLE
void judge_am8258B()
{
#if (AM_CHIP_MINOR == 8258)
	if(am_soc_get_chip_id() != CHIP_ID_AM8258B){
		printf("\n***********************************************************\n");
		EZCASTWARN("It is not 8258B,do not support this IC[%d]!!!\n", am_soc_get_chip_id());
		printf("***********************************************************\n");
		while(1){
			sleep(1);
		}
	}

#endif
}
void productInit()
{
	int ret = -1;
	
#if LAN_ONLY
	get_netlink_status();
#endif	//LAN_ONLY

	ezRemoteMsgInit();
	ezwire_set_devname(DEVNAME);

	ezCastSetRouterCtlEnable(1);

	printf("\n\n\n##########Init Wifi Remote Control thread##########\n\n\n");
	ret=wifi_remote_control_init();
	if(ret==0)
		printf("wifi remote start success!!\n");
	else
		printf("wifi remote start fail!!\n");
	create_websetting_server();
	htmlSettingStart();
	createQRCodeForWire();
}
#endif

//extern int btvd_work_start(void);
int main(int argc, char *argv[])
{
	int parent_msg_id;
	int rpipe,key_pipe;
	int flag;
	SWF_DEC * inst;
	SWF_INSTINFO info;
	SWF_RECT window = {0, 0, IMAGE_WIDTH_E-1, IMAGE_HEIGHT_E-1};
	int err;
	struct pollfd fd;
	struct am_sys_msg msg;
	int rdnr=0;
	int ret;
	char mainswf[64];
	int usbfd;
	unsigned char usb_status[4];
	int frame ;
	
	struct timeval value;
	unsigned int milisec;
	int inter=0, counter=0, counter1=0, inter1=0;

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
	soft_watchdog_create();
#endif
#ifdef FUI_FRAMERATE_ENABLE
	fd_set rfds;
	int max_select_fd=-1;
	static int auto_framerate_cnt=0;
	static int rate_changed=0;
	static int prev_framerate = 30;
#endif

	static int showbuffer = -1;

    /**
    * For SWF timer serializing. We use a pipe to do so.
    * When a timer happends, we write the message to the pipe and then
    * the fui main thread reads the message from the pipe.
    */
    int timer_serialize_pipe[2];
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		Probox_init();
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8074) || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
  	system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
	usleep(100*1000);
	system("modprobe /lib/modules/2.6.27.29/snd-usb-audio.ko");
	printf("insmod am7x_hcd for projector");
	//system("insmod /lib/modules/2.6.27.29/am7x_hcd_next.ko");

#endif

#if 1
	struct am_bus_priority prio_tmp;

	am_soc_get_bus_priority(&prio_tmp);
	prio_tmp.special_dma = 3;
	prio_tmp.mac=0;
	prio_tmp.ahb_bus = 2;
	prio_tmp.graph_2d=4;
	prio_tmp.de = 8;//8
	prio_tmp.dac= 9;//6
	prio_tmp.vdc= 5;
	prio_tmp.axi_bus = 7;  // 1
	am_soc_set_bus_priority(&prio_tmp);
#endif

#ifdef VIDEO_TEST_WITHOUTUI
	video_test_main(argc,(char*)argv);
#endif

	if(argc > 1){
		if(argc != 4){
			printf("error parameters\n");
			exit(1);
		}
		parent_msg_id = (int)strtol(argv[1],NULL,10);
		rpipe = (int)strtol(argv[2],NULL,10);
		key_pipe = (int)strtol(argv[3],NULL,10);
	}
	else{
		printf("fui arg err\n");
		exit(1);
	}

	fd.fd = rpipe;
	fd.events = POLLIN;
	fui_system_init();

    /**
    * Open the timer serialize pipe.
    */
    if (pipe(timer_serialize_pipe) == -1){
        timer_serialize_pipe[0] = -1;
        timer_serialize_pipe[1] = -1;
    }
    else {
        if (timer_serialize_pipe[0] > max_select_fd) {
            max_select_fd = timer_serialize_pipe[0];
        }
    }

#ifdef FUI_DEBUG_EN
	/** register debug handler */
	///fui_dl_show();

	register_segv_handler(SIGSEGV, fui_signal_handler);
	register_segv_handler(SIGILL, fui_signal_handler);
	register_segv_handler(SIGBUS, fui_signal_handler);
//[Sanders.130423] EZ Stream error handling.
	register_segv_handler(SIGIO, fui_signal_handler);
	register_segv_handler(SIGPIPE, fui_signal_handler);
#endif

	/**
	* calibrate the clkdly for change sdram pll
	**/
	//pll_calibrate_clkdly();

	/**
	* initialize the system resources.
	*/
	fui_system_resource_init();

#ifdef ROTATE_SENSOR_EN
	rotatesensor_detect_thread_create();
#endif
	/** 
	* insmod 2d 
	*/
	//fui_insmod_2d();

	/** 
	* set lcm 
	*/
	if(fui_open_lcm() == -1){
		printf("open lcm error\n");
		exit(1);
	}

	/** 
	* init swf-player context 
	*/
	if(fui_set_context()==-1){
		printf("set contex error\n");
		exit(1);
	}

    if (timer_serialize_pipe[1] > 0) {
        SWF_SetTimerSerializeFD(timer_serialize_pipe[1]);
    }

	/**
	* open the display thread if we separate display from fui main thread.
	*/
#ifdef INDEPENDENT_DISPLAY_EN
	{
		struct display_callback cb;

		cb.put_display_task = put_display_task;
		cb.get_current_display_addr = get_current_display_addr;
		
		open_display_workflow();
		SWF_RegisterDisplayCallback(&cb);
	}
#endif
	
	/** 
	* init system res 
	*/
	if(system_res_init()!=0){
		printf("system resource init error\n");
	}
	
	/** 
	* register external engine 
	*/
	Swfext_Register();

	/**
	* initialize the key mapping.
	*/
	read_swfkey_cfgfile("/am7x/case/data/keymap/globalswfkey.bin");

	/**
	* create the mouse show process.
	*/
#if SYS_MOUSE_USED
	pid_t pid;
	Mouse_init(100,100);
	pid = fork();
	if(pid<0)
	{
		printf("fork mouse move process erro\n");
		exit(0);
	}
	if(pid==0)
	{
		mouse_draw_mainloop(parent_msg_id);
		exit(0);
	}
#endif

#if EZCAST_ENABLE
	judge_am8258B();
	#if EZWIRE_ENABLE	
	ezwireInit();
	read_andorid_table_file();
	#endif	// EZWIRE_ENABLE

	#if LAN_ONLY || EZWIRE_ENABLE || (MODULE_CONFIG_EZCASTPRO_MODE==8074)
	printf("----fui productInit\n");
	productInit();
	#endif		// LAN_ONLY || EZWIRE_ENABLE

#endif	// EZCAST_ENABLE

	/**
	* load the main swf
	*/
	fui_load_swf("main.swf");
	
	/** 
	* create hotplug thread 
	*/
	hotplug_open_work();

	/*
	* register the exit function when fui out.
	*/
	atexit(fui_out);

	/**
	* alarm initialization
	*/
	alarm_init((void*)parent_msg_id);

	/**
	* audio dynamic library open.
	*/
	_audio_dll_open();
 
	/**
	* set the seed for SWF random()
	*/
	gettimeofday(&value,NULL);
	milisec = (unsigned int)(value.tv_sec*1000 + value.tv_usec/1000);
	srand(milisec);

#ifdef FUI_FRAMERATE_ENABLE	
	/**
	* create the frame-rate controll process.
	*/
	
	_fui_create_framerate_control();

    if (framerate_fds1[0] > max_select_fd) {
    	max_select_fd = framerate_fds1[0];
    }

	if (rpipe > max_select_fd) {
		max_select_fd = rpipe;
	}

	if (key_pipe > max_select_fd) {
		max_select_fd = key_pipe;
	}

#endif	

	ezFuiMain();

#if EZMUSIC_ENABLE  || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
    /* Mos: Clear ezmusicinfo.json at start, to avoid shutdown during autoplay,
    *  http://220.128.123.30/mantis/view.php?id=17306
    */
    Write_default_info_TOJson();
#endif

#if defined(MODULE_CONFIG_FLASH_TYPE) && MODULE_CONFIG_FLASH_TYPE != 0
	int fdw = open("/proc/sys/vm/drop_caches", O_RDWR);
	if(fdw >= 0)
	{
		char val[2] = "1";
		write(fdw, val, sizeof(val));
		close(fdw);
	}
#endif
	/** 
	* main loop 
	*/
	while(1){

#ifdef FUI_TIME_DEBUG
		gettimeofday(&value,NULL);
		milisec = (unsigned int)(value.tv_sec*1000 + value.tv_usec/1000);
#endif

		FD_ZERO(&rfds);
		FD_SET(framerate_fds1[0], &rfds);
		FD_SET(rpipe, &rfds);
		FD_SET(key_pipe, &rfds);
        
        if (timer_serialize_pipe[0] > 0) {
            FD_SET(timer_serialize_pipe[0], &rfds);
        }
		
		if(select(max_select_fd+1,&rfds,NULL,NULL,NULL)>0){
			int already_read_nr = 0;
			/**
			* Receive message from manager.
			* We read at most five message for each time.
			*/
			if(FD_ISSET(key_pipe,&rfds)){
				do{
					rdnr = read(key_pipe,&msg,sizeof(struct am_sys_msg));
					already_read_nr++;
					if(rdnr == sizeof(struct am_sys_msg)){
						switch(msg.type){
							case SYSMSG_KEY:
							{
								int key;
								check_power_key(&msg);
								if((key = swf_key_read(&msg)) != SWF_MSG_NULL){
									printf("fui key == 0x%x \n",key);
									_fui_deal_special_key(key);
									if(key != SWF_MSG_KEY_EXT_POWER){
										SWF_Message(NULL, key, NULL);
										///SWF_Message(NULL, SWF_MSG_PLAY, NULL);
									}
								}
							}
							break;

							default:								
							break;

						}
					}
					else{
						break;
					}
					
				}while(already_read_nr<FUI_READ_MSG_EACH_TIME);

			}
			
			if(FD_ISSET(rpipe,&rfds)){
				do{
					rdnr = read(rpipe,&msg,sizeof(struct am_sys_msg));
					if(rdnr == sizeof(struct am_sys_msg)){
						switch(msg.type){
							
						case SYSMSG_TOUCH:
						{
							fui_translate_touch_msg(&msg);
						}
						break;
							
					#if SYS_MOUSE_USED
						case SYSMSG_MOUSE:
							fui_translate_mouse_msg(&msg);
						break;
					#endif
						
						case SYSMSG_CARD:
							hotplug_put_msg(msg);
						break;

						case SYSMSG_USB:
							hotplug_put_msg(msg);
						break;
							
						case SYSMSG_ALARM:
						{
							printf("@@@@@Perfect Alarm On ID=%d@@@@@\n",msg.dataload);
							alarm_on_notice(&msg);
						}
						break;
							
						default:
								
						break;
						}
					}
					else{
						break;
					}
			
				}while(0);
			}
				
			/**
			* receive frame rate controll message.
			*/
			if(FD_ISSET(framerate_fds1[0],&rfds)){
				int rt;
				char dummy;
				int current_framerate = 30;

				if(rate_changed){
					int nread;
					char *buf;

					buf = malloc(1024);
					if(buf){
						do{
							nread = read(framerate_fds1[0],buf,1024);
						}while(nread>=1024);
						free(buf);
					}
					else{
						read(framerate_fds1[0],&dummy,sizeof(char));
					}
					
					rate_changed =0;
				}
				else{
					read(framerate_fds1[0],&dummy,sizeof(char));
				}
				rt = SWF_Message(NULL, SWF_MSG_PLAY, NULL);
				if(rt == -1){
					printf("swf message full\n");
				}

				/**
				* do auto framerate.
				*/
				auto_framerate_cnt++;
				if(auto_framerate_cnt >= 10){
					auto_framerate_cnt = 0;
					current_framerate = _fui_get_current_highest_framerate();
					if(current_framerate > 0){
						if(current_framerate >=100){
							current_framerate = 120;
						}

						if(current_framerate != prev_framerate){
							prev_framerate = current_framerate;
							rate_changed=1;
							write(framerate_fds0[1],&current_framerate,sizeof(int));
						}
					}
				}
			}

            /**
            * serializing timer message
            */
            if ((timer_serialize_pipe[0] > 0) && FD_ISSET(timer_serialize_pipe[0],&rfds)){
                SWF_TimerEventSerialize te;
                int ste;
                
                ste = read(timer_serialize_pipe[0],&te,sizeof(SWF_TimerEventSerialize));
                if (ste == sizeof(SWF_TimerEventSerialize)) {
                    SWF_Message(te.s, te.msg, te.param);
                }
            }
			
		}
		else{
			continue;
		}

#if (EZWIRE_ENABLE && EZWIRE_TYPE == AM8251_EZDOCKING)	// Check quick charge switch key, and switch it.
		QCKeyCheck();
#endif

		/**
		* dispath messages
		*/
		while((ret = SWF_Dispatch()) != SWF_MSG_NULL && ret != SWF_MSG_EXIT)
		{
			if(ret == SWF_MSG_PLAY && swapflag == 1)
			{
				frame = SWF_Prepare(NULL);

#ifndef INDEPENDENT_DISPLAY_EN
				if(showbuffer != frame)
				{
				#if CONFIG_SWFDEC_DROPFRAME_EN
					if(frame != 1){
						fui_show(frame, deinst);
					}
				#else
					fui_show(frame, deinst);
					showbuffer = frame;
				#endif
				}
#endif
			#ifdef USE_NEW_VIDEO_OSD
				if(!flashengine_is_new_video_osd_mode()){
				#if CONFIG_SWFDEC_DROPFRAME_EN
					if(frame != 1){
						SWF_Update(NULL);
					}
					else{
						///printf("drop frame\n");
					}
				#else
					SWF_Update(NULL);
				#endif
				}
			#else /** USE_NEW_VIDEO_OSD */
			#if CONFIG_SWFDEC_DROPFRAME_EN
				if(frame != 1){
					SWF_Update(NULL);
				}
				else{
					///printf("drop frame\n");
				}
			#else /** CONFIG_SWFDEC_DROPFRAME_EN */
				SWF_Update(NULL);
			#endif /** CONFIG_SWFDEC_DROPFRAME_EN */
				
			#endif /** USE_NEW_VIDEO_OSD */
			}
		}
		
	}
	
	//fui_audio_engine_close();
	_audio_dll_close();
	
	return 0;
}

int FUI_Render_CurrFrame()
{
	int frame;
	frame = SWF_Prepare(NULL);
	fui_show(frame, deinst);
	SWF_Update(NULL);
	//SWF_SetGUIFlag(1);

	return 0;
}




