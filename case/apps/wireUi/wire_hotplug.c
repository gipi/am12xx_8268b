#include <stdio.h>
#include <errno.h>
#include <libusb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

#include "wire_hotplug.h"
#include "wire_log.h"
#include "sys_cfg.h"
#include "sys_msg.h"
#include "wire_player.h"
#include "wire_config.h"
#include "wire_ota.h"
#include "imirror.h"
#include "wire_factory_test.h"
#include <dlfcn.h>
#include "am7x_dac.h"
#include "lcm_op.h"
#include "wire_osd.h"
#include "wire_heap.h"
#include "osapi.h"

#define OSDICON_PALETTE		"/am7x/case/data/osdIconPalette.plt"
#define OSDPROMPT_PATH		"/am7x/case/data/OSD_RGB_Default.bin"
#define OSDSHOW_DELAY_TIME	(15*60)		// seconds
#define OSDSHOW_DELAY_TIME2	(30*60)		// seconds
#define OSDSHOW_TIME		(10)		// seconds

#define HUB_DEV			0x09
#define IPHONE_VENDOR	0x5ac

struct imirrorData_s{
	void *audioHandle;
	int oldVolume;
};

struct audioSineInfo_s {
	time_t audioStartTime;
	int osdHidTime;
	char audioInit;
	char sineReceive;
};
static struct audioSineInfo_s audioSineInfo;

typedef struct EZ_DISPLAY_MEM_S EZ_EISPLAY_MEM_T;
struct EZ_DISPLAY_MEM_S
{
	void *pHeap;
	unsigned long phy_addr;
	void *log_ptr;
	int size;
};
static EZ_EISPLAY_MEM_T ezDisplayMemObj;
static int ezDisplayMemObjInit = 0;

static struct am_sys_msg am_msg;
static int hotplug_handle_thread;
static M_HOTPLUG_WORK mh_work;
static pthread_t hotplug_tid;
static int usb_stat_next;
static int dVender = -1;
static int ValeriaStart = 0;
static pthread_t tid = 0;
static int tidFlag = 0;
#if 1
int adbStartFlag = 0;
int lastVolume = -1;
int adbNoAudioFlag = 0;
static int aoa_to_adb_flag=0;
#endif

static int osdIconOn = 0;

static void releaseOsdPicPrompt()
{
	if(osdIconOn)
	{
		osdengine_release_osd();
		osdIconOn = 0;
	}
}

static int showOsdPicPrompt(const char *path, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	unsigned int scr_width = 0;
	unsigned int src_height = 0;
	
	if(osdIconOn)
	{
		releaseOsdPicPrompt();
	}
	
	getScreenSize(&scr_width, &src_height);
	
	int osd_x = x;//(x*scr_width)/1920;
	int osd_y = y;// (y*src_height)/1080;
	int osd_w =w;//(w*scr_width)/1920;
	int osd_h = h;//(h*src_height)/1080;
	osd_w = (osd_w/64 + 1)*64;
	EZCASTLOG("path: %s, [%d:%d:%d:%d]\n", path, x, y, w, h);
	EZCASTLOG("screen: [%d:%d]\n", scr_width, src_height);	
	EZCASTLOG("[%d, %d, %d, %d]\n", osd_x, osd_y, osd_w, osd_h);

	osdengine_init_osd(osd_x, osd_y, osd_w, osd_h, DE_PIX_FMT_OSDBIT4MODE, OSDICON_PALETTE);
	osdengine_set_alpha(8);
	osdengine_enable_osd();
	osdengine_show_icon(0, 0, path, 1);

	osdIconOn = 1;
}

static int AudioCallback(int cmd, void *para)
{
	#if 0
	if(1==cmd)
	{
		EZCASTLOG("Please Open OSD\n");
		showOsdPicPrompt(OSDPROMPT_PATH, 100, 980, 800, 80);
	}
	else if(0==cmd)
	{
		EZCASTLOG("Please Close OSD\n");
		releaseOsdPicPrompt();
	}
	#else
	if(0 == cmd)
	{
		EZCASTLOG("Sine wave received!\n");
		audioSineInfo.sineReceive = 1;
		if(osdIconOn)
		{
			releaseOsdPicPrompt();
		}
	}
	#endif
	
	return 0;
}

void audioSineCheck()
{
	if(audioSineInfo.audioInit && !audioSineInfo.sineReceive)
	{
		time_t curTime = time(NULL);
		if(osdIconOn && ((curTime - audioSineInfo.audioStartTime) >= OSDSHOW_TIME))
		{
			releaseOsdPicPrompt();
			audioSineInfo.audioStartTime = time(NULL);
			audioSineInfo.sineReceive = 1;
		}
		else if(!osdIconOn && ((curTime - audioSineInfo.audioStartTime) >= audioSineInfo.osdHidTime))
		{
			unsigned int scr_width=0;
			unsigned int src_height=0;
			unsigned int x=0;
			unsigned int y=0;
			getScreenSize(&scr_width, &src_height);
			x=(scr_width-800)/2;
			y=(src_height-80)*7/8;
			showOsdPicPrompt(OSDPROMPT_PATH, x, y, 800, 80);
			audioSineInfo.audioStartTime = time(NULL);
			
			if(audioSineInfo.osdHidTime != OSDSHOW_DELAY_TIME2)
				audioSineInfo.osdHidTime = OSDSHOW_DELAY_TIME2;
		}
	}
	else if(osdIconOn)
	{
		releaseOsdPicPrompt();
	}
}

static int hotplug_sem_init(sem_t *sem, int pshared, unsigned int value)
{
	int ret;

	ret = sem_init(sem, pshared, value);
    if (ret != 0)
    {
        WLOGE("Semaphore initialization failed\n");
		return -1;
    }
	return 0;
}

static void hotplug_sem_destroy(sem_t *sem)
{
	sem_destroy(sem);
}

static void hotplug_sem_wait(sem_t *sem)
{
	int err;

__PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto __PEND_REWAIT;	
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return;
}

static void hotplug_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

bool is_wire_usb_event(int fd, fd_set *fds)
{
	if(fd < 0)
		return false;
	if(FD_ISSET(fd, fds)){
		WLOGI("select keyevent!\n");
		return true;
	}
	return false;
}
static int hotplug_input_msg(struct am_sys_msg msg)
{
	if(hotplug_handle_thread == 0){
		WLOGE("media hotplug thread not start\n");
		return -1;
	}
	hotplug_sem_wait(&mh_work.lock_in);
	WLOGI("Put Msg wp=%d,Msgtype=0x%x,subtype=0x%x,dataload=0x%x\n",mh_work.wp,msg.type,msg.subtype,msg.dataload);
	if(msg.subtype==HOST_USB_OUT)
		set_usb_out_flag(1);
	else
		set_usb_out_flag(0);
	mh_work.msgfifo[mh_work.wp] = msg;
	mh_work.wp++;
	if(mh_work.wp >= MEDIA_MSG_FIFO_LEN){
		mh_work.wp = 0;
	}
	hotplug_sem_post(&mh_work.lock_out);
	return 0;
}

int wire_usb_handler(int fd)
{
	int rdnr=0;

	rdnr = read(fd, &am_msg, sizeof(struct am_sys_msg));
	if(rdnr == sizeof(struct am_sys_msg)){
		switch(am_msg.type){
			case SYSMSG_USB:
				hotplug_input_msg(am_msg);
				break;
			default:
				break;
		}
	}
	return 0;
}
static void *checkLoop(void *arg)
{
	int i = 0;

	tidFlag = 1;
	
	while(i < 60&&tidFlag==1){		//times according to udhcpc -t 10 &
		WLOGI();
		if(access("/tmp/amgw.txt",F_OK) == 0){
			#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
			ezwireDrawWindow(WIRE_UI_ARROW_CONNECT, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
			#endif
			#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
			show_local_version();
			#endif
			ezwireDrawFlip();
			if((access(GETOTAURL_OTACONF,F_OK))==0){
				WLOGI("%s is ok!\n",GETOTAURL_OTACONF);
				#if ( EZWIRE_TYPE==MIRALINE)
				ezFuiRemoteStart();
				#endif
			}
			return NULL;
		}
		sleep(1);
		i++;
	}
	return NULL;
}

static int checkGetIP()
{

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&tid, &attr, checkLoop, NULL);

	return 0;
}
#if MODULE_CONFIG_ADB_MIRROR_ONLY!=1
#define IDEVICEINFO_SIMPLE_FILE_TYPE	"/tmp/ideviceinfo.ProductType"
#define IDEVICEINFO_SIMPLE_FILE_VERSION	"/tmp/ideviceinfo.ProductVersion"
static int isiPhoneX()
{
	int i = 0;
	system("ps");
	for(i=0; i<40; i++)
	{
		int ret = system("ideviceinfo -s > "IDEVICEINFO_SIMPLE_FILE_TYPE);
		EZCASTLOG("ret: %d\n", ret);
		if(ret == 0)
			break;
	}
	system("cat "IDEVICEINFO_SIMPLE_FILE_TYPE);
	if(access(IDEVICEINFO_SIMPLE_FILE_TYPE, F_OK) == 0)
	{
		int fd = open(IDEVICEINFO_SIMPLE_FILE_TYPE, O_RDONLY);
		if(fd >= 0)
		{
			char buff[1024];
			int ret = read(fd, buff, sizeof(buff));
			close(fd);
			if(ret > 0)
			{
				char *s = strstr(buff, "ProductType:");
				if(s != NULL)
				{
					char *e = strchr(s, '\n');
					if(e != NULL)
						*e = '\0';
					EZCASTLOG("product: %s\n", s);
					char *r = strstr(s, "iPhone10,3");
					if(r != NULL)
						return 1;
					r = strstr(s, "iPhone10,6");
					if(r != NULL)
						return 1;
				}
			}
		}
	}

	return 0;
}

static int getImirrorSetupMode()
{
	int ret=0,mode=0;
	char callbuf[24]={0};
	FILE *fp = NULL;
	fp=fopen("/mnt/vram/ezcast/DEFAULTMODE","r");
	if(fp!=NULL)
	{
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		mode=atoi(callbuf);
		if(mode>0)
			mode=mode-1;
		return mode;
	}
	else
		return  IMIRROR_MODE_DOCKING;

}

static imirror_mode_t getImirrorMode()
{
	char DockingModeDevices_iPhone[][32] = {
		"iPhone10,3",
		"iPhone10,6",
		"iPhone10,2",
		"iPhone10,5",
		"iPhone9,2",
		"iPhone9,4",
		"iPhone8,2",
		"iPhone7,1",
	};
	char CameraModeDevices_iPad[][32] = {
		"iPad6,3",
		"iPad6,4",
		"iPad6,7",
		"iPad6,8",
		"iPad7,1",
		"iPad7,2",
		"iPad7,3",
		"iPad7,4",
	};
	int i = 0;
	char *r = NULL;
	//system("ps");
	
		
	if(access(IDEVICEINFO_SIMPLE_FILE_TYPE, F_OK) == 0)
		unlink(IDEVICEINFO_SIMPLE_FILE_TYPE);
	if(access(IDEVICEINFO_SIMPLE_FILE_VERSION, F_OK) == 0)
		unlink(IDEVICEINFO_SIMPLE_FILE_VERSION);
	
	for(i=0; i<40; i++)
	{
		int ret = system("ideviceinfo -k ProductType > "IDEVICEINFO_SIMPLE_FILE_TYPE);
		EZCASTLOG("ret: %d\n", ret);
		if(ret == 0)
		{
			system("ideviceinfo -k ProductVersion > "IDEVICEINFO_SIMPLE_FILE_VERSION);
			break;
		}
	}
	//system("cat "IDEVICEINFO_SIMPLE_FILE_TYPE);
	if(access(IDEVICEINFO_SIMPLE_FILE_VERSION, F_OK) == 0)
	{
		int fd = open(IDEVICEINFO_SIMPLE_FILE_VERSION, O_RDONLY);
		if(fd >= 0)
		{
			char buff[1024];
			int ret = read(fd, buff, sizeof(buff));
			close(fd);
			if(ret > 0)
			{
				EZCASTLOG("version: %s\n", buff);

				int v1=0, v2=0, v3=0;
				sscanf(buff, "%d.%d.%d", &v1, &v2, &v3);
				EZCASTLOG("v1: %d, v2: %d, v3: %d\n", v1, v2, v3);
				if(v1 > 0 && v1 < 10)
					return IMIRROR_MODE_CAMERA;
			}
		}
	}
	if(access(IDEVICEINFO_SIMPLE_FILE_TYPE, F_OK) == 0)
	{
		int fd = open(IDEVICEINFO_SIMPLE_FILE_TYPE, O_RDONLY);
		if(fd >= 0)
		{
			char buff[1024];
			int ret = read(fd, buff, sizeof(buff));
			close(fd);
			if(ret > 0)
			{
				EZCASTLOG("product: %s\n", buff);

				r = strstr(buff, "iPad");
				if(r != NULL)
				{
					#if 0
					for(i=0; i<sizeof(CameraModeDevices_iPad)/sizeof(CameraModeDevices_iPad[0]); i++)
					{
						//EZCASTLOG("CameraModeDevices_iPad[%d]: %s\n", i, CameraModeDevices_iPad[i]);
						r = strstr(buff, CameraModeDevices_iPad[i]);
						if(r != NULL)
							return IMIRROR_MODE_CAMERA;
					}

					return IMIRROR_MODE_DOCKING;
					#else
					return IMIRROR_MODE_CAMERA;
					#endif
				}
				if(getImirrorSetupMode()==IMIRROR_MODE_CAMERA)
				{
					
					for(i=0; i<2; i++)//iphone x do not support IMIRROR_MODE_CAMERA
					{
						r = strstr(buff, DockingModeDevices_iPhone[i]);
						if(r != NULL)
							return IMIRROR_MODE_DOCKING;
					}
									
					return IMIRROR_MODE_CAMERA;
				}
					
				#if 0
					for(i=0; i<sizeof(DockingModeDevices_iPhone)/sizeof(DockingModeDevices_iPhone[0]); i++)
					{
						//EZCASTLOG("DockingModeDevices_iPhone[%d]: %s\n", i, DockingModeDevices_iPhone[i]);
						r = strstr(buff, DockingModeDevices_iPhone[i]);
						if(r != NULL)
							return IMIRROR_MODE_DOCKING;
					}
				#else
					return IMIRROR_MODE_DOCKING;
				#endif
			}
		}
	}

	return IMIRROR_MODE_CAMERA;
}

static INITAUDIO	InitAudio = NULL;
static SETAUDIOCALLBACK	SetAudioCallBack = NULL;

static int wire_initAudio(int SampleRate, int ChannelsPerFrame, int BitsPerChannel)
{
	int ret = -1;
	
	if(InitAudio)
	{
		ret = InitAudio(SampleRate, ChannelsPerFrame, BitsPerChannel);
		
#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		if(SetAudioCallBack)
		{
			SetAudioCallBack(AudioCallback);
			
			audioSineInfo.osdHidTime = OSDSHOW_DELAY_TIME;
			audioSineInfo.audioStartTime = time(NULL);
			audioSineInfo.audioInit = 1;
			audioSineInfo.sineReceive = 0;
		}
#endif
	}

	return ret;
}

static void hantroClose()
{
	releaseOsdPicPrompt();
	
	wire_HantroClose();
}

static void decodeClose()
{
	releaseOsdPicPrompt();
	
	decode_close();
}

static int initWirePlayer(wireDecodeInfo_t *decode)
{
	struct imirrorData_s *imirrorData = NULL;
	int vol = 38;

	if(decode == NULL)
	{
		WLOGE("decode is NULL.\n");
		return -1;
	}
	
	audioSineInfo.audioInit = 0;
	audioSineInfo.sineReceive = 0;

	imirrorData = (struct imirrorData_s *)malloc(sizeof(struct imirrorData_s));
	if(imirrorData == NULL)
	{
		WLOGE("imirror data malloc fail.\n");
		perror("ERROR");
		return -1;
	}
	
	int fd = open("/dev/DAC",2);
	if (fd < 0) {
		printf("open dac error when get\n");
	}
	else
	{
		int err = ioctl(fd,DACIO_GET_VOLUME,(unsigned char *)&vol);
		if(err < 0){
			WLOGE("dac get volume error\n");
		}
		
		imirrorData->oldVolume = vol;
		
		vol = 38;
		
		err = ioctl(fd,DACIO_SET_VOLUME,(unsigned char *)&vol);
		if(err < 0){
			WLOGE("dac get volume error\n");
		}
		
		close(fd);
	}

	imirrorData->audioHandle = dlopen("libaudio_player.so", RTLD_LAZY | RTLD_LOCAL);
	if(imirrorData->audioHandle == NULL)
	{
		WLOGE("open libaudio_player.so fail.\n");
		perror("ERROR");
		return -1;
	}

	decode->userData = imirrorData;
	decode->width = 1280;
	decode->height = 720;
	decode->malloc = malloc;
	decode->free = free;
	InitAudio = OSDLGetProcAddr(imirrorData->audioHandle, "InitAudio");
	SetAudioCallBack = OSDLGetProcAddr(imirrorData->audioHandle, "SetAudioCallBack");
	decode->sendAudioBuff = OSDLGetProcAddr(imirrorData->audioHandle, "SendAudioBuf");
	decode->uninitAudio = OSDLGetProcAddr(imirrorData->audioHandle, "UninitAudio");
	decode->initAudio = wire_initAudio;
	decode->initVideo = wire_HantroOpen;
	decode->uninitVideo = hantroClose;
	decode->sendVideoBuff = wire_HantroPlay;
	decode->sendVideoCodec = wire_SetFrameSPSPPS;

	decode->limitResolutionProduct = (getStreamingDecodeMemorySize() / 7 / 3);
	decode->initStreaming = decode_open;
	decode->uninitStreaming = decodeClose;
	decode->streamingSeek = decodeSeek;
	decode->streamingSetAudioTime = decodeSetAudioTime;
	decode->streamingResume = decodeResume;
	decode->streamingPause = decodePause;

	//decode->mode = IMIRROR_MODE_DOCKING;
	decode->mode = getImirrorMode();
	WLOGE("Set mirror mode to %s\n", (decode->mode == IMIRROR_MODE_CAMERA)?"IMIRROR_MODE_CAMERA":"IMIRROR_MODE_DOCKING");
	return 0;
}

static void uninitWirePlayer(wireDecodeInfo_t *decode)
{
	if(decode == NULL)
	{
		WLOGE("decode is NULL.\n");
		return;
	}

	struct imirrorData_s *imirrorData = decode->userData;

	if(imirrorData != NULL)
	{
		if(imirrorData->audioHandle != NULL)
			dlclose(imirrorData->audioHandle);
		imirrorData->audioHandle = NULL;

		if(imirrorData->oldVolume >= 0)
		{
			int fd = open("/dev/DAC",2);
			if (fd < 0) {
				printf("open dac error when get\n");
			}
			else{
				int err = ioctl(fd,DACIO_SET_VOLUME,(unsigned char *)&imirrorData->oldVolume);
				if(err < 0){
					printf("dac get volume error\n");
				}
				
				close(fd);
			}
		}
		free(imirrorData);
	}

	decode->userData = NULL;

	decode->initAudio = NULL;
	decode->sendAudioBuff = NULL;
	decode->uninitAudio = NULL;
	InitAudio = NULL;
	SetAudioCallBack = NULL;
	
	audioSineInfo.audioInit = 0;
	audioSineInfo.sineReceive = 0;
	audioSineInfo.osdHidTime = OSDSHOW_DELAY_TIME;
	
	decode->initVideo = NULL;
	decode->uninitVideo = NULL;
	decode->sendVideoBuff = NULL;
	decode->sendVideoCodec = NULL;
}


static int wirePlayerCallback(int cmd, void *retVal)
{
	int ret;
	
	switch(cmd)
	{
		case PLUGPLAY_CMD_IDEVICEREAD:
			WLOGT("sync start\n");
			system("sync");
			WLOGT("sync stop\n");
			break;
		case PLUGPLAY_CMD_ISMIRRORREAD:
			ret = ezwireMainUiHide();
			if(ret == 0)
				return 1;
			else
				return 0;
			break;
		case PLUGPLAY_CMD_MIRRORSTOP:
			{
				if(retVal == NULL)
				{
					WLOGE("retVal is NULL.\n");
					return -1;
				}
				wireDecodeInfo_t *decode = (wireDecodeInfo_t *)retVal;
				uninitWirePlayer(decode);
				system("usbmuxd --exit");
				//osdengine_enable_osd();
				setWireMirrorStatus(WIRE_IPHONE_PLUG_STOP);
				ezwireDrawDefaultBg();
				#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
				show_local_version();
				#endif
				ezwireDrawFlip();
				break;
			}
		case PLUGPLAY_CMD_INIT_DECODER:
			{
				if(retVal == NULL)
				{
					WLOGE("retVal is NULL.\n");
					return -1;
				}
				wireDecodeInfo_t *decode = (wireDecodeInfo_t *)retVal;
				initWirePlayer(decode);
				setWireMirrorStatus(WIRE_IPHONE_PLUG_START);
				break;
			}
		default:
			break;
	}
	
	return 0;
}

void EZDisplayMemRelease(void);

static int EZDisplayMemInit(int size, void **log_pptr, unsigned long *phy_addr)
{
	int ret = -1;
	
	if( !ezDisplayMemObjInit && size > 0 )
	{
		memset( &ezDisplayMemObj, 0, sizeof(EZ_EISPLAY_MEM_T) );
		
		do
		{
			ezDisplayMemObj.pHeap = (void *)wire_MemoryInit((unsigned int)size);
			if( !ezDisplayMemObj.pHeap )
			{
				printf(" [%s - %d] failed to wire_MemoryInit, FIXME!\n", __FUNCTION__, __LINE__);
				break;
			}
			
			ezDisplayMemObj.log_ptr = OSHmalloc(ezDisplayMemObj.pHeap, size, &ezDisplayMemObj.phy_addr);
			if( !ezDisplayMemObj.log_ptr )
			{
				printf(" [%s - %d] failed to OSHmalloc, FIXME!\n", __FUNCTION__, __LINE__);
				break;
			}
			
			if( log_pptr )
			{
				(*log_pptr) = ezDisplayMemObj.log_ptr;
			}
			
			if( phy_addr )
			{
				(*phy_addr) = ezDisplayMemObj.phy_addr;
			}
			 
			ezDisplayMemObj.size = size;
			
			printf(" [%s - %d] phy_addr@(%lu), log_ptr@(0x%08x), ok!\n", __FUNCTION__, __LINE__, (*phy_addr), (*log_pptr));
			
			ret = 0;
		} while(0);
	}
	
	ezDisplayMemObjInit = 1;
	
	if( ret )
	{
		EZDisplayMemRelease();
	}
	
	return (ret);
}

void EZDisplayMemRelease(void)
{
	if( ezDisplayMemObjInit )
	{
		if( ezDisplayMemObj.log_ptr )
		{
			OSHfree( ezDisplayMemObj.pHeap, ezDisplayMemObj.log_ptr );
			ezDisplayMemObj.log_ptr = NULL;
		}
		
		if( ezDisplayMemObj.pHeap )
		{
			wire_MemoryRelease( ezDisplayMemObj.pHeap );
			ezDisplayMemObj.pHeap = NULL;
		}
		
		printf(" [%s - %d] ok!\n", __FUNCTION__, __LINE__);
		
		ezDisplayMemObjInit = 0;
	}
}

static void WifiDisplayStart(void)
{
	ezFuiMain();
	
	ezConfigSetUIFn(&ezwireDrawDefaultBg, &ezwireDrawFlip);
	ezFuiWifiStart(0, 0, NULL, &EZDisplayMemInit, &EZDisplayMemRelease);
	ezFuiRemoteSetOtaCallback(otaEntry);
}

static void WifiDisplayStop(void)
{
	ezFuiWifiStop();
	ezConfigSetUIFn(NULL, NULL);
}

static int iphone_in(libusb_device *dev)
{
	WLOGI("iphone in\n");

	if(access("/tmp/eth_usb_stop", F_OK) == 0){
		unlink("/tmp/eth_usb_stop");
	}

	if(isWirePlugMode() == 1)
	{	int ret = imirror_ValeriaCheck(dev);
		printf("ret = %d\n",ret);
		if(ret != 0)
		{
			imirror_ValeriaSwitch(dev, 1);
		}
		else
		{
			system("killall -9 usbmuxd");
			system("usbmuxd --enable-exit");
			imirror_ValeriaStart(wirePlayerCallback);
			ValeriaStart = 1;
		}
	}else{
		#if (EZWIRE_TYPE==MIRAPLUG)
		WifiDisplayStart();
		#endif
		setWireSetupStatus(WIRE_IPHONE_SETUP_START);
		system("killall -9 usbmuxd");
		system(AM_CASE_SC_DIR"/eth_usb.sh iphone in &");
		checkGetIP();
	}
	return 0;
}

static int iphone_out()
{
	WLOGI("iphone out\n");
	#if 1//AM_CHIP_MINOR == 8258
	system("rm /tmp/usbmuxd*");
	#endif
	if(getWireSetupStatus()==WIRE_IPHONE_SETUP_START)
	{
		setWireSetupStatus(WIRE_IPHONE_SETUP_STOP);
		#if (EZWIRE_TYPE==MIRAPLUG)
		WifiDisplayStop();
		#endif
	}
	system(AM_CASE_SC_DIR"/eth_usb.sh iphone out");
	WLOGI("tidFlag=%d\n",tidFlag);
	if(tidFlag == 1){
		#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		ezwireDrawWindow(WIRE_UI_ARROW_DISCONNECT, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
		#endif
		#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
		show_local_version();
		#endif
		ezwireDrawFlip();
		if(pthread_kill(tid,0) == 0){
			WLOGI();
			pthread_cancel(tid);
			pthread_join(tid,NULL);
		}
		tidFlag = 0;
	}
	if(ValeriaStart){
		imirror_ValeriaStop();
		ValeriaStart = 0;
	}
	return 0;
}
#endif

int initLibUsbHandle()
{
	int ret;

	ret = libusb_init(NULL);

	return ret;
}

void deinitLibUsbHandle()
{
	libusb_exit(NULL);
}
void setDefaultVol()
{
	sys_volume_ctrl(_VOLUME_CTRL_GET, &lastVolume);
	
	if(lastVolume == _VOLUME_LEVEL_MUTE)
		lastVolume = 0;
	
	int newVol = 14;
	sys_volume_ctrl(_VOLUME_CTRL_SET, &newVol);
}
void startAdbStream()
{
     	printf("startAdbStream\n");
	setAndroidAdbStatus(WIRE_ANDROID_PLUG_START);
	system(AM_CASE_SC_DIR"/usb_process.sh host_adb_start");
	adbStartFlag = 1;
	setDefaultVol();
}
void adb_stop(){
	printf("adb_stop\n");
	adbStartFlag = 0;
	setAndroidAdbStatus(WIRE_ANDROID_PLUG_STOP);
	system(AM_CASE_SC_DIR"/usb_process.sh host_adb_stop");
	delete_warn();
	#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
	ezwireDrawWindow(WIRE_UI_ARROW_DISCONNECT, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
	#endif
	ezwireDrawFlip();
	#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
	show_local_version();
	#endif
}

#define AOA_CTRL_OUT (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define AOA_CTRL_IN (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)

#define AOA_PROTO_MANUFACTURE_INDEX		(0)
#define AOA_PROTO_MODEL_INDEX 			(1)
#define AOA_PROTO_DESCRIPTION_INDEX		(2)
#define AOA_PROTO_VERSION_INDEX			(3)
#define AOA_PROTO_URI_INDEX				(4)
#define AOA_PROTO_SERIAL_INDEX			(5)
#define AOA_REQ_PROTOCOL				(51)
#define AOA_REQ_SETPROTO				(52)
#define AOA_REQ_ACCESSORY				(53)
#define AOA_REQ_REGISTER_HID			(54)
#define AOA_REQ_UNREGISTER_HID			(55)
#define AOA_REQ_SET_HID_REPORT			(56)
#define AOA_SEND_HID_EVENT				(57)
#define AOA_REQ_AUDIO					(58)

#define AOA_MANUFACTURE_STRING		("Actions, Inc.")
#define AOA_MANUFACTURE_STRING_URL	("Actions%2C%20Inc.")
#define AOA_MODEL_STRING			("Usb displayer")
#define AOA_MODEL_STRING_URL		("Usb%20displayer")
#define AOA_DESCRIPTION_STRING		("EZCast")
#define AOA_VERSION_STRING			("1.0")
#define AOA_URI_STRING				("https://www.ezcast.com/qrcode/usb.php")
#define AOA_SERIAL_STRING			("12345678-001")

typedef struct libusb_device_descriptor libusb_device_descriptor;
typedef struct libusb_config_descriptor libusb_config_descriptor;
typedef struct libusb_interface libusb_interface;
typedef struct libusb_interface_descriptor libusb_interface_descriptor;

const char* formatLibUsbError(int errcode)
{
	const char* result = NULL;
	struct {
		int code;
		const char* string;
	} err_table[] = {
		{ LIBUSB_SUCCESS, "LIBUSB_SUCCESS" },
		{ LIBUSB_ERROR_IO, "LIBUSB_ERROR_IO" },
		{ LIBUSB_ERROR_INVALID_PARAM, "LIBUSB_ERROR_INVALID_PARAM" },
		{ LIBUSB_ERROR_ACCESS, "LIBUSB_ERROR_ACCESS" },
		{ LIBUSB_ERROR_NO_DEVICE, "LIBUSB_ERROR_NO_DEVICE" },
		{ LIBUSB_ERROR_NOT_FOUND, "LIBUSB_ERROR_NOT_FOUND" },
		{ LIBUSB_ERROR_BUSY, "LIBUSB_ERROR_BUSY" },
		{ LIBUSB_ERROR_TIMEOUT, "LIBUSB_ERROR_TIMEOUT" },
		{ LIBUSB_ERROR_OVERFLOW, "LIBUSB_ERROR_OVERFLOW" },
		{ LIBUSB_ERROR_PIPE, "LIBUSB_ERROR_PIPE" },
		{ LIBUSB_ERROR_INTERRUPTED, "LIBUSB_ERROR_INTERRUPTED" },
		{ LIBUSB_ERROR_NO_MEM, "LIBUSB_ERROR_NO_MEM" },
		{ LIBUSB_ERROR_NOT_SUPPORTED, "LIBUSB_ERROR_NOT_SUPPORTED" },
		{ 13, "LIBUSB_ERROR_OTHER" },
		{ 0, NULL },
	};

	if ((errcode <= 0) && (errcode >= LIBUSB_ERROR_NOT_SUPPORTED)) {
		result = err_table[0-errcode].string;
	} else {
		result = err_table[13].string;
	}
	return result;
}

int getProtocol(libusb_device_handle *dev, uint16_t* protocol) {
	return libusb_control_transfer(dev,
		AOA_CTRL_IN | LIBUSB_RECIPIENT_DEVICE, // bmRequestType
		AOA_REQ_PROTOCOL, // bRequest
		0, // value
		0, // index
		(uint8_t*) protocol, // data buffer
		2,    // 2 byte
		500); // timeout 500ms
}

int setProto(libusb_device_handle *dev, int idx,const char* str)
{
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_SETPROTO,
			0,
			idx,
			(unsigned char*)str,
			strlen(str) + 1,
			500); // timeout
}

int switchToAccessoryMode(libusb_device_handle *dev)
{
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_ACCESSORY,
			0,
			0,
			NULL,
			0,
			500);
}

int setAudioMode(libusb_device_handle *dev, int mode)
{
	int value = !!mode;
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_AUDIO,
			value,
			0,
			NULL,
			0,
			500);
}

static int getApkDownloadUrl(char *url, unsigned int size)
{
	char vendor[128];
	char id[32];
	char version[128];
	char strVal[1024];
	int ret = -1;

	ret = ota_get_vendor(vendor, sizeof(vendor));
	if(ret < 0)
	{
		EZCASTWARN("Get vendor name fail.");
		snprintf(vendor, sizeof(vendor), "unknown");
	}
	
#if EZWIRE_ENABLE
	ret = am_soc_get_chip_id_high_32bit_to_string(id, sizeof(id));
#else
	ret = get_mac_address_info(id, MAC_WLAN0);
#endif
	if(ret < 0)
	{
		EZCASTWARN("Get ChipID/MAC fail.");
		snprintf(id, sizeof(id), "unknown");
	}

	snprintf(url, size, "%s?model=%s&fw_version=%s&ota_vendor=%s&manufacture=%s&version=%s&mac_address=%s", \
					AOA_URI_STRING, (char *)AOA_MODEL_STRING_URL, ota_get_local_version(), vendor, (char *)AOA_MANUFACTURE_STRING_URL, AOA_VERSION_STRING, id);


	EZCASTLOG("url: %s\n", url);
	return 0;
}

static int switchAccessoryMode(libusb_device *dev, int audioSupport)
{
	int ret = 0;
	int rc = 0;
	libusb_device_descriptor desc = {0};
	rc = libusb_get_device_descriptor(dev, &desc);
	if(rc < 0)
	{
		EZCASTWARN("libusb_get_device_descriptor fail<%s>.\n", strerror(errno));
		return rc;
	}
	EZCASTLOG("found a device, %04x:%04x\n", desc.idVendor, desc.idProduct);

	libusb_device_handle* handle = NULL;

	int err = 0;
	err = libusb_open(dev, &handle);
	if (err < 0) {
		ret = -1;
		EZCASTWARN("open device failed, errcode:%d, description:%s\n",
				err, formatLibUsbError(err));
		goto __exit__;
	}

	uint16_t protocol;
	err = getProtocol(handle, &protocol);
	if (err < 0) {
		ret = -2;
		EZCASTWARN("it is not android-powerd device, errcode:%d, description:%s\n",
				err, formatLibUsbError(err));
		goto __end__;
	}

#if 1
	char url[512];
	getApkDownloadUrl(url, sizeof(url));
	setProto(handle, AOA_PROTO_MANUFACTURE_INDEX, AOA_MANUFACTURE_STRING);
	setProto(handle, AOA_PROTO_MODEL_INDEX, AOA_MODEL_STRING);
	setProto(handle, AOA_PROTO_DESCRIPTION_INDEX, AOA_DESCRIPTION_STRING);
	setProto(handle, AOA_PROTO_VERSION_INDEX, AOA_VERSION_STRING);
	setProto(handle, AOA_PROTO_URI_INDEX, url);
	setProto(handle, AOA_PROTO_SERIAL_INDEX, AOA_SERIAL_STRING);
#endif

	EZCASTLOG("[%s-%d] protocol: %d, audioSupport: %d\n", __func__, __LINE__, protocol, audioSupport);
	if (protocol == 2) {
		setAudioMode(handle, audioSupport);
	}
	switchToAccessoryMode(handle);

__end__:
	if (handle)
		libusb_close(handle);
__exit__:
	return ret;
}

void switchToAoa(libusb_device *dev)
{
       printf("switchToAoa adbNoAudioFlag=%d\n",adbNoAudioFlag);
	if(adbNoAudioFlag == 1 || access(AOA_DISABLE, F_OK) == 0)
	{
		adbNoAudioFlag = 0;
		int fd = open(AUDIO_UNSUPPORT_FLAG, O_CREAT);
		if(fd >= 0)
			close(fd);	
		startAdbStream();
	}
	else
	{
		system("killall adb");
		//system(AM_CASE_SC_DIR"/eth_usb.sh aoa");
		int ret=switchAccessoryMode(dev, 1);
		if(ret!=0)
			system("touch /tmp/aoa_not_support");
	}
}

static int switchToAccessory()
{
	size_t count = 0;
	libusb_device **list = NULL;
	int rc = 0;
	libusb_device* dev = NULL;
	int ret = -1;
	int i = 0, j = 0;
	size_t idx = 0;
	struct libusb_config_descriptor *config = NULL;
	const struct libusb_interface_descriptor *altsetting = NULL;
	const struct libusb_interface *interface = NULL;

	count = libusb_get_device_list(NULL, &list);
	if(count <= 0)
	{
		EZCASTWARN("libusb_get_device_list fail<[%d]%s>\n", count, strerror(errno));
		return -1;
	}

	for (idx = 0; idx < count; ++idx) {
		libusb_device *device = list[idx];
		libusb_device_descriptor desc = {0};
		rc = libusb_get_device_descriptor(device, &desc);
		if(rc < 0)
		{
			EZCASTWARN("libusb_get_device_descriptor fail<[%d]%s>\n", rc, strerror(errno));
			return -1;
		}
		EZCASTLOG("[%s-%d] desc.idVendor: 0x%04x, desc.idProduct: 0x%04x, desc.bDeviceClass: 0x%04x\n", __func__, __LINE__, desc.idVendor, desc.idProduct, desc.bDeviceClass);
		if(desc.idVendor == 0x18d1 && (desc.idProduct >= 0x2D00 && desc.idProduct <= 0x2D05))		// is aoa mode
			continue;

		if (desc.bDeviceClass != HUB_DEV)
		{
			ret = libusb_get_active_config_descriptor(device, &config);
			if(ret != 0 || config == NULL){
				EZCASTWARN("Get config discriptor fail!\n");
				continue;
			}
			int desNum = 0;
			for(i=0; i<config->bNumInterfaces; i++){
				interface = &config->interface[i];
				for(j=0; j<interface->num_altsetting; j++){
					altsetting = &interface->altsetting[j];
					desNum++;
					EZCASTLOG("bInterfaceClass: 0x%02x, bInterfaceSubClass: 0x%02x, bInterfaceProtocol: 0x%02x\n", altsetting->bInterfaceClass, altsetting->bInterfaceSubClass, altsetting->bInterfaceProtocol);
				}
			}
			if(config != NULL)
				libusb_free_config_descriptor(config);
			config = NULL;
			//EZCASTLOG("[%s-%d]\n", __func__, __LINE__);
			int ret=switchAccessoryMode(device, 1);
			if(ret!=0)
				system("touch /tmp/aoa_not_support");
			//if(desNum > 1)
			{
				dev = device;
				//break;
			}
		}
	}

	return 0;
}

int setAdbNoAudio(int val)
{
	printf("adbStartFlag: %d, adbNoAudioFlag: %d, val: %d\n", adbStartFlag, adbNoAudioFlag, val);
	
	if(adbNoAudioFlag != !!val)
	{
		adbNoAudioFlag = !!val;
		printf("adbNoAudioFlag: %d\n", adbNoAudioFlag);
		
		if(adbStartFlag == 0)
			return 0;
		
		if(adbNoAudioFlag == 0)
		{
			switchToAccessory();
		}
		/*else
		{
			set_gpio(94, 0);
			__usleep(1000000);
			set_gpio(94, 1);
		}*/
	}
	return 0;
}
int adbAudioInit()
{
	if(access(AOA_DISABLE, F_OK) == 0)
		adbNoAudioFlag = 1;
	else
		adbNoAudioFlag = 0;

	return adbNoAudioFlag;
}
#define ADB_MANUFACTURER_FILE	"/tmp/adb_product.manufacturer"
#define ADB_MODEL_FILE	"/tmp/adb_product.model"
#define ADB_VERSION_FILE	"/tmp/adb_product.version"
#define DEVICE_TABLE_FILE "/mnt/vram/ezcast/device_table.txt"
#define LIBUSB_CLASS_ADB	0x42
#define ADB_DEVICEINFO_FILE	"/tmp/adb_deviceinfo"
static int  device_num=0;
static unsigned int  cur_idProduct=0,cur_idVendor=0;
static unsigned char cur_Manufacturer[64]={0};
static unsigned char cur_Model[64]={0};
static unsigned char cur_Version[64]={0};
typedef struct device_id_s
{
	unsigned int vid;		//
	unsigned int pid;		//
	char manufacturer[64];
	char model[64];
	char version[64];	//the android version
	int num;
}device_id_t;
typedef struct aoa_blacklist_s
{
	char manufacturer[64];
	char model[64];
}aoa_blacklist_t;

#define BUF_DEVICE_LEN 50

device_id_t *Devices_id_android;

static aoa_blacklist_t aoa_blacklist[]={
	
	{"Amazon","KFSAWI"},
	{"LG","LG-D802"},
	{"OPPO","OPPO R9tm"},
	{"HTC","HTC_M9pw"},
	{"vivo","vivo Y67"},
	{"samsung","SM-N9002"},
	{"samsung","SM-G960"},
	{"Sony","D6503"},
	
};
int read_device_num()
{
	EZCASTLOG("device_num=%d\n",device_num);
	return device_num;

}


static int write_andorid_table_file(char *buf)
{
	FILE *fd = NULL;
	EZCASTLOG("write_andorid_table_file len=%d %s\n",strlen(buf),buf);
	if(access(DEVICE_TABLE_FILE,F_OK)==0)
	{
		fd = fopen(DEVICE_TABLE_FILE, "a+");
	}
	else
		fd = fopen(DEVICE_TABLE_FILE, "wb+");
		
	if(fd!=NULL)
	{
		EZCASTLOG("\n");
		fwrite(buf,1, strlen(buf),fd);
		EZCASTLOG("\n");
		fflush(fd);
		EZCASTLOG("\n");
		int fd_write = fileno(fd);
		if(fsync(fd_write)==-1){
			EZCASTLOG("flush Error!\n");
			return 0;
		}
		fclose(fd);
	}
	return 1;

}
#define MAXBSTZE 1024
static int file_wc(const char *filename)
{
        int fd;
        int linect = 0;
        char buf[MAXBSTZE];
        int len;
        char *p = NULL;
        if(filename) {
                if((fd = open(filename, O_RDONLY)) < 0) {
                        fprintf(stderr, "open %s\n",strerror(errno));
                        return -1;
                }

                while(1) {
                        if((len = read(fd,buf,sizeof(buf))) == -1) {
                            return -1;
                        }
                        if(len == 0){
                            break;
                        }
                        for( p = buf; len--; ){
                            if( *p == '\n' ){
                               ++linect;
                            }
                            ++p;
                        }
                }
		 close(fd);
        }

        return linect;

}
 int read_andorid_table_file()  //Devices_id_android
{

	int fd=-1;
	char buf[4096]={0};
	char buf1[256]={0};
	char *locoate=NULL;
	char *p=NULL;
	char *p_next=NULL;
	char *p_child=NULL;
	int i=0;
	int ret=0;
	int len=0,buf_len=0,max_len=0;
	int line=0;
	static int old_line=0;
	EZCASTLOG("read_andorid_table_file\n");
	if(access(DEVICE_TABLE_FILE,F_OK)!=0)
		return 0;
	line=file_wc(DEVICE_TABLE_FILE);
	EZCASTLOG("old_line=%d   line=%d \n",old_line,line);
	if(line==0)
		return 0;
	if(old_line!=line)
	{
		old_line=line;
		if(Devices_id_android==NULL)
		{
			EZCASTLOG("malloc buffer\n");
			Devices_id_android= (device_id_t *)malloc(line*sizeof(device_id_t));
		}
		else
		{
			EZCASTLOG("free  buffer\n");
			free(Devices_id_android);
			Devices_id_android= (device_id_t *)malloc(line*sizeof(device_id_t));
		}
	}
	memset(Devices_id_android,0,line* sizeof(device_id_t));
	fd = fopen(DEVICE_TABLE_FILE, "r");
	if(fd >= 0)
	{
		//EZCASTLOG("start to read_andorid_table_file,sizeof=%d\n",sizeof(buf));
		while(fread(buf1, 1, sizeof(buf1), fd)>0)
		{
			max_len=max_len+strlen(buf1);
			if(max_len<sizeof(buf))
				strncat(buf,buf1,strlen(buf1));
			else
			{
				fclose(fd);
				system("rm "DEVICE_TABLE_FILE);
				system("sync");
				EZCASTLOG("it is not enough memory,delete it\n");
				return 0;
				
			}
		}
		fclose(fd);
		buf_len=strlen(buf);
		//EZCASTLOG("buf_len=%d   buf=%s \n",buf_len,buf);
		locoate=buf;
		p=strtok(locoate,"\n");
		if(p!=NULL)
		{
			//EZCASTLOG("p=%s \n",p);
		}
		else
			EZCASTLOG("data null \n");
		
		while(p!=NULL)
		{
			len=len+strlen(p)+1;
			p_child=strtok(p,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("VID=");
				Devices_id_android[i].vid=atoi(p_child);
			//	printf("Devices_id_android[%d].vid=0x%4x\n",i,Devices_id_android[i].vid);
			}
			else
			{
				EZCASTLOG("read data er\nr");
				return 0;
			}
			p_child=strtok(NULL,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("PID=");
				Devices_id_android[i].pid=atoi(p_child);
			//	printf("Devices_id_android[%d].pid=0x%4x\n",i,Devices_id_android[i].pid);
			}
			else
			{
				EZCASTLOG("read data err\n");
				return 0;
			}
			p_child=strtok(NULL,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("MANUFACTURER=");
				//EZCASTLOG("MANUFACTURER  p_child=%s \n",p_child);
				if(strlen(p_child)<=sizeof(Devices_id_android[i].manufacturer))
					strncpy(Devices_id_android[i].manufacturer,p_child,strlen(p_child));
				//EZCASTLOG("Devices_id_android[%d].manufacturer=%s\n",i,Devices_id_android[i].manufacturer);
			}
			else
			{
				EZCASTLOG("read data err\n");
				return 0;
			}
			p_child=strtok(NULL,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("MODEL=");
				//EZCASTLOG("MODEL  p_child=%s \n",p_child);
				if(strlen(p_child)<=sizeof(Devices_id_android[i].model))
					strncpy(Devices_id_android[i].model,p_child,strlen(p_child));
				//EZCASTLOG("Devices_id_android[%d].model=%s\n",i,Devices_id_android[i].model);
			}
			else
			{
				EZCASTLOG("read data err\n");
				return 0;
			}
			p_child=strtok(NULL,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("VERSION=");
				//EZCASTLOG("VERSION  p_child=%s \n",p_child);
				if(strlen(p_child)<=sizeof(Devices_id_android[i].version))
					strncpy(Devices_id_android[i].version,p_child,strlen(p_child));
			//	EZCASTLOG("Devices_id_android[%d].version=%s\n",i,Devices_id_android[i].version);
			}
			else
			{
				EZCASTLOG("read data err\n");
				return 0;
			}
			p_child=strtok(NULL,",");
			if(p_child!=NULL)
			{
				p_child=p_child+strlen("NUM=");
				//EZCASTLOG("NUM  p_child=%s \n",p_child);
				Devices_id_android[i].num=atoi(p_child);
				//EZCASTLOG("Devices_id_android[%d].num=%d\n",i,Devices_id_android[i].num);
			}
			else
			{
				EZCASTLOG("read data err\n");
				return 0;
			}
			device_num=i+1;
			//EZCASTLOG("device_num=%d buf_len=%d len=%d\n",device_num,buf_len,len);
			if(buf_len<=len)
				return 1;
			p_next=&buf[len];
		//	EZCASTLOG("p_next=%s \n",p_next);
			p=strtok(p_next,"\n");
			if(p!=NULL)
			{
				//EZCASTLOG("p=%s\n",p);
			//	break;
			}
			else
			{
				EZCASTLOG("test  read data err\n");
				return 0;
			}
			i++;		
				
		}
		
	}
	else
		EZCASTLOG("open %s failde \n",DEVICE_TABLE_FILE);
	return 1;
		

}
static int modify_andorid_table_file(char * key, int num)
{
	char tmp[4096] = {0};
	char buf[4096] = {0};
	char buf1[256] = {0};
	char tmp2[256] = {0};
	char * locate1 = NULL;
	int fd=-1;
	int ret=0;
	int max_len=0;
	fd = fopen(DEVICE_TABLE_FILE, "r");
	while(fread(buf1, 1, sizeof(buf1), fd)>0)
	{
		max_len=max_len+strlen(buf1);
		if(max_len<sizeof(buf))
			strncat(buf,buf1,strlen(buf1));
		else
		{
			fclose(fd);
			system("rm "DEVICE_TABLE_FILE);
			system("sync");
			EZCASTLOG("it is not enough memory,delete it\n");
			return 0;
				
		}
	}
	//EZCASTLOG("buf=%s\n",buf);
	//EZCASTLOG("key=%s\n",key);

	locate1 = strstr(buf,key);
	if(locate1==NULL)
	{
		EZCASTLOG("can not  find key word on table file\n");
		return -1;
	}
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	memcpy(tmp,buf,strlen(buf)-strlen(locate1));
	snprintf(tmp2,sizeof(tmp2),"%s%d",key,num);
	//EZCASTLOG("tmp2=%s\n",tmp2);
	if(tmp2==NULL)
		return -1;
	int len1 = strlen(buf)-strlen(locate1);
	int len2 = strlen(tmp2);
	memcpy(tmp+len1,tmp2,len2);
	memcpy(tmp+len1+len2,locate2,strlen(locate2));
	memset(buf,0,strlen(buf));
	memcpy(buf,tmp,strlen(tmp));
	if(buf!=NULL){
		FILE *fp = NULL;
		fp =  fopen(DEVICE_TABLE_FILE, "wb+");
		if(fp == NULL){
			return -1;
		}
		fwrite(buf, 1, 4096, fp);
		fflush(fp);
		int fd_write = fileno(fp);
		fsync(fd_write);
		fclose(fp);
	}
	return 0;
}

static int is_on_device_list(int vid,int pid,char *manufacturer,char *model,char *ver)
{
	int i=0;
	for(i=0;i<device_num;i++)
	{
		//EZCASTLOG("cur_idVendor=0x%4x ,Devices_id_android[%d].vid=0x%4x  cur_idProduct=0x%4x ,Devices_id_android[%d].pid=0x%4x \n",cur_idVendor,i,Devices_id_android[i].vid,cur_idProduct,i,Devices_id_android[i].pid);
             	if((vid==Devices_id_android[i].vid)&&(pid==Devices_id_android[i].pid)&&(strstr(manufacturer,Devices_id_android[i].manufacturer)!=NULL)&&(strstr(model,Devices_id_android[i].model)!=NULL)&&(strstr(ver,Devices_id_android[i].version)!=NULL))
		{
			EZCASTLOG("It is in vid table file  list ,return 1 \n");
			return 1;
		}


	}
	return 0;
}

int is_support_aoa()
{
	int i=0;
	int ret=0;
	int fd=-1;
	char cur_manufacturer[64]="";
	char cur_model[64]="";
	char cur_version[64]="";
	int blacklist_len= sizeof(aoa_blacklist)/sizeof(aoa_blacklist_t);
	system("rm /tmp/adb_product*");
	system("ifconfig lo up");
	system("adb devices ");
	system("adb shell getprop ro.product.manufacturer > "ADB_MANUFACTURER_FILE);
	system("adb shell getprop ro.product.model > "ADB_MODEL_FILE);
	system("adb shell getprop ro.build.version.release > "ADB_VERSION_FILE);
	fd = open(ADB_MANUFACTURER_FILE, O_RDONLY);
	if(fd >= 0)
	{
		 ret = read(fd, cur_manufacturer, sizeof(cur_manufacturer));
		close(fd);
		EZCASTLOG("cur_manufacturer=%s \n",cur_manufacturer);
		if(ret==0||strstr(cur_manufacturer,"daemon not running")!=NULL)
		{
			EZCASTLOG("can not get manufacturer\n");	
			memset(cur_Manufacturer,0,sizeof(cur_Manufacturer));
			snprintf(cur_manufacturer,sizeof(cur_manufacturer),"uknow");
			strncpy(cur_Manufacturer,cur_manufacturer,strlen(cur_manufacturer));
			EZCASTLOG("cur_Manufacturer=%s",cur_Manufacturer);
		}
		else
		{
			if(cur_manufacturer!=NULL&&strlen(cur_manufacturer)>2)
			{
				memset(cur_Manufacturer,0,sizeof(cur_Manufacturer));
				strncpy(cur_Manufacturer,cur_manufacturer,strlen(cur_manufacturer)-2);
				EZCASTLOG("cur_Manufacturer=%s len=%d\n",cur_Manufacturer,strlen(cur_Manufacturer));
			}
			else
				EZCASTLOG(" get Manufacturer error\n");	
				
		}
	}
	else
	{
		EZCASTLOG("ADB get manufacturer failed \n");
		return 0;

	}
	fd = open(ADB_MODEL_FILE, O_RDONLY);
	if(fd >= 0)
	{
		ret = read(fd, cur_model, sizeof(cur_model));
		close(fd);
		EZCASTLOG("cur_model=%s \n",cur_model);
		if(ret==0||strstr(cur_model,"daemon not running")!=NULL)
		{
			EZCASTLOG("can not get model\n");	
			memset(cur_Model,0,sizeof(cur_Model));
			snprintf(cur_model,sizeof(cur_model),"uknow");
			strncpy(cur_Model,cur_model,strlen(cur_model));
			EZCASTLOG("cur_Model=%s",cur_Model);
		}
		else
		{
			if(cur_model!=NULL&&strlen(cur_model)>2)
			{
				memset(cur_Model,0,sizeof(cur_Model));
				strncpy(cur_Model,cur_model,strlen(cur_model)-2);
				EZCASTLOG("cur_Model=%s len=%d\n",cur_Model,strlen(cur_Model));
			}
			else
				EZCASTLOG(" get Model error\n");	
				
		}
	}
	else
	{
		EZCASTLOG("ADB get model failed \n");
		return 0;

	}
	fd = open(ADB_VERSION_FILE, O_RDONLY);
	if(fd >= 0)
	{
		 ret = read(fd, cur_version, sizeof(cur_version));
		 close(fd);
		EZCASTLOG("ret=%d cur_version=%s \n",ret,cur_version);
		if(ret==0||strstr(cur_version,"daemon not running")!=NULL)
		{
			EZCASTLOG("can not get version\n");	
			memset(cur_Version,0,sizeof(cur_Version));
			snprintf(cur_version,sizeof(cur_version),"uknow");
			strncpy(cur_Version,cur_version,strlen(cur_version));
			EZCASTLOG("cur_Version=%s",cur_Version);

		}
		else
		{
			if(cur_version!=NULL&&strlen(cur_version)>2)
			{
				memset(cur_Version,0,sizeof(cur_Version));
				strncpy(cur_Version,cur_version,strlen(cur_version)-2);
				EZCASTLOG("cur_Version=%s len=%d\n",cur_Version,strlen(cur_Version));
			}
			else
				EZCASTLOG(" get version error\n");	
				
		}
			
	}
	else
	{
		EZCASTLOG("ADB get version failed \n");
		return 0;

	}
	for(i=0;i<blacklist_len;i++)
	{
		//EZCASTLOG("cur_manufacturer=%s ,aoa_blacklist[%d].manufacturer=%s  cur_model=%s aoa_blacklist[%d].model=%s \n",cur_manufacturer,i,aoa_blacklist[i].manufacturer,cur_model,i,aoa_blacklist[i].model);
		if((strstr(cur_manufacturer,aoa_blacklist[i].manufacturer)!=NULL)&&(strstr(cur_model,aoa_blacklist[i].model)!=NULL))
		{
			EZCASTLOG("It is in aoa blacklist ,return 0 \n");
			return 0;

		}
		
	}
	/*if(strstr(cur_manufacturer,"OPPO")!=NULL)//|| (strstr(cur_manufacturer,"HUAWEI")!=NULL)
	{
		EZCASTLOG("It is OPPO ,not support aoa, return 0 \n");
		return 0;

	} */
 	for(i=0;i<device_num;i++)
	{
		//EZCASTLOG("cur_idVendor=0x%4x ,Devices_id_android[%d].vid=0x%4x  cur_idProduct=0x%4x ,Devices_id_android[%d].pid=0x%4x \n",cur_idVendor,i,Devices_id_android[i].vid,cur_idProduct,i,Devices_id_android[i].pid);
		
             	if((cur_idVendor==Devices_id_android[i].vid)&&(cur_idProduct==Devices_id_android[i].pid)&&(strstr(cur_version,Devices_id_android[i].version)!=NULL))
		{
			char kee_word[256]={0};
			snprintf(kee_word,sizeof(kee_word),"VID=%d,PID=%d,MANUFACTURER=%s,MODEL=%s,VERSION=%s,NUM=",Devices_id_android[i].vid,Devices_id_android[i].pid,Devices_id_android[i].manufacturer,Devices_id_android[i].model,Devices_id_android[i].version);
		//	EZCASTLOG("kee_word=%s\n",kee_word);
			if(Devices_id_android[i].num==0)
			{
				EZCASTLOG("it is NUM=0 ,try to use AOA mode \n");
 				return 1;
			}
			else if(Devices_id_android[i].num==10)
			{
				EZCASTLOG("10 times,ignore it,try to use AOA mode \n");
				modify_andorid_table_file(kee_word,0);
				read_andorid_table_file();
				return 1;
			}
			else
			{
				EZCASTLOG("It is in table file  list ,return 0 \n");
				int times=Devices_id_android[i].num+1;
				modify_andorid_table_file(kee_word,times);
				read_andorid_table_file();
				return 0;
			}
		}


	}
	EZCASTLOG("It is not in list \n");
	return 1;


}
static int swith_usb(int num)
{
	#define UOC_DISCONNECT                  1
	#define UOC_RECONNECT                    2
	int err=0;
	int fd = 0;
	if(num==0)
		fd=open("/proc/driver/uoc",2);
	else if(num==1)
		fd=open("/proc/driver/uoc1",2);
	if (fd < 0) {
		printf("open uoc error \n");
	}
	else
	{
		 err = ioctl(fd,UOC_DISCONNECT,NULL);
		 sleep(3);
		 err = ioctl(fd,UOC_RECONNECT,NULL);
		close(fd);
		aoa_to_adb_flag=1;
	}
}
static int clean_device_data(void)
{
	cur_idVendor=0;
	cur_idProduct=0;
	memset(cur_Manufacturer,0,sizeof(cur_Manufacturer));
	memset(cur_Model,0,sizeof(cur_Model));
	memset(cur_Version,0,sizeof(cur_Version));
}

static int android_in(libusb_device *dev){
	int i=0, j=0, ret = -1;
	int len=0,len_r=0;;
	int fd=-1;
	int currVender = 1;
	struct libusb_config_descriptor *config = NULL;
	struct libusb_device_descriptor desc;
	const struct libusb_interface_descriptor *altsetting = NULL;
	const struct libusb_interface *interface = NULL;
	char w_buf[256]={0};
	printf("Android in\n");
	ret = libusb_get_active_config_descriptor(dev, &config);
	if(ret != 0){
		printf("Get config discriptor fail!\n");
		return ret;
	}

	printf("num_altsetting: %d, bNumInterfaces: %d\n", config->interface->num_altsetting, config->bNumInterfaces);
	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		currVender = 1;
	}else{
		currVender = desc.idVendor;
	}
	printf("dVender: 0x%04x\n", currVender);
       printf("desc.idVendor: 0x%04x\n", desc.idVendor);
	printf("desc.idProduct: 0x%04x\n", desc.idProduct);
	// Check ADB+Audio+accessory mode
	if((desc.idVendor == 0x18d1) && \
		((desc.idProduct == 0x2d01) || ((desc.idProduct >= 0x2d00) && (desc.idProduct <= 0x2d05))))
	{
		printf("Android ADB\n");
		if(desc.idProduct <= 0x2d01){
			fd = open(AUDIO_UNSUPPORT_FLAG, O_CREAT);
			if(fd >= 0){
				close(fd);
			}
		}else{
			if(access(AUDIO_UNSUPPORT_FLAG, F_OK) == 0)
				unlink(AUDIO_UNSUPPORT_FLAG);
		}
		draw_warn_aoa();
		system("ifconfig lo up");
		ret = system("adb devices > "ADB_DEVICEINFO_FILE);
		fd = open(ADB_DEVICEINFO_FILE, O_RDONLY);
		if(fd >= 0)
		{
			char buf[1024]={0};
			char buf0[]="* daemon not running. starting it now on port 5037 *\
				* daemon started successfully *\
				List of devices attached";
			ret = read(fd, buf, sizeof(buf));
			close(fd);
			len=strlen(buf0)-2;
			len_r=strlen(buf);
			if(ret > 0)
			{
				if(strstr(buf, "offline")||(len_r<=len))//if(strstr(buf, "unauthorized")||strstr(buf, "offline"))
				{
					if(cur_idVendor==0&&cur_idProduct==0)
					{
						goto __END__;
					}
					EZCASTLOG("not support AOA ,ready to record it \n");
					if(is_on_device_list(cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version)==0)
					{
						snprintf(w_buf,sizeof(w_buf),"VID=%d,PID=%d,MANUFACTURER=%s,MODEL=%s,VERSION=%s,NUM=1\n",cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version);
						write_andorid_table_file(w_buf);
					}
					else
					{
						snprintf(w_buf,sizeof(w_buf),"VID=%d,PID=%d,MANUFACTURER=%s,MODEL=%s,VERSION=%s,NUM=",cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version);
						modify_andorid_table_file(w_buf,1);
					}
					read_andorid_table_file();
					#if defined(MODULE_CONFIG_USB1_DISABLE) &&MODULE_CONFIG_USB1_DISABLE==1
					swith_usb(0);
					#else
					swith_usb(1);
					#endif
					goto __END__;

				}
			}
		}
		startAdbStream();
		goto __END__;
	}
	for(i=0; i<config->bNumInterfaces; i++){
		interface = &config->interface[i];
		for(j=0; j<interface->num_altsetting; j++){
			altsetting = &interface->altsetting[j];
			EZCASTLOG("bInterfaceClass: 0x%02x, bInterfaceSubClass: 0x%02x, bInterfaceProtocol: 0x%02x\n", altsetting->bInterfaceClass, altsetting->bInterfaceSubClass, altsetting->bInterfaceProtocol);
			if((altsetting->bInterfaceClass == LIBUSB_CLASS_WIRELESS && altsetting->bInterfaceSubClass == LIBUSB_CLASS_AUDIO && altsetting->bInterfaceProtocol == 0x03) || \
				(altsetting->bInterfaceClass == LIBUSB_CLASS_COMM && altsetting->bInterfaceSubClass == 0x06) || \
				(currVender == 0x0bb4 && altsetting->bInterfaceClass == LIBUSB_CLASS_COMM && altsetting->bInterfaceSubClass == LIBUSB_CLASS_COMM)/*HTC*/)
			{
				printf("Android USB share\n");
				clean_device_data();
				#if (EZWIRE_TYPE==MIRAPLUG)
				WifiDisplayStart();
				#endif
				setAndroidSetupStatus(WIRE_ANDROID_SETUP_START);
				system(AM_CASE_SC_DIR"/usb_process.sh host_android_in");
				dVender = currVender;
				setDefaultVol();
				EZCASTLOG("dVender: 0x%04x\n", dVender);
				checkGetIP();
				goto __END__;
			}
			else if(altsetting->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC && altsetting->bInterfaceSubClass == LIBUSB_CLASS_ADB)
			{ 
				cur_idProduct=desc.idProduct;
				cur_idVendor=desc.idVendor;
				if(is_support_aoa()==0)
				{
					if(aoa_to_adb_flag==1)
					{
						draw_warn_adb();
						aoa_to_adb_flag=0;
					}
					adbNoAudioFlag=0;
					EZCASTLOG(" not support AOA ,start adb and mirror adbNoAudioFlag=%d \n",adbNoAudioFlag);
					system("touch /tmp/audio_time_not_support");
					dVender = currVender;
					startAdbStream();
					goto __END__;
				}
				EZCASTLOG(" support adb and try to Android AOA\n");
				dVender = currVender;
				switchToAoa(dev);	
				if(access("/tmp/aoa_not_support",F_OK)==0)
				{
					system("rm /tmp/aoa_not_support");
					if(cur_idVendor==0&&cur_idProduct==0)
					{
						goto __END__;
					}
					if(is_on_device_list(cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version)==0)
					{
						snprintf(w_buf,sizeof(w_buf),"VID=%d,PID=%d,MANUFACTURER=%s,MODEL=%s,VERSION=%s,NUM=1\n",cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version);
						write_andorid_table_file(w_buf);
					}
					else
					{
						snprintf(w_buf,sizeof(w_buf),"VID=%d,PID=%d,MANUFACTURER=%s,MODEL=%s,VERSION=%s,NUM=",cur_idVendor,cur_idProduct,cur_Manufacturer,cur_Model,cur_Version);
						modify_andorid_table_file(w_buf,1);
					}
					read_andorid_table_file();
					#if defined(MODULE_CONFIG_USB1_DISABLE) &&MODULE_CONFIG_USB1_DISABLE==1
					swith_usb(0);
					#else
					swith_usb(1);
					#endif
				}
				goto __END__;
				
			}
		}
	}

__END__:
	if(config != NULL)
		libusb_free_config_descriptor(config);
	return 0;
}
static int android_out(){
	printf("android_out\n");
	system(AM_CASE_SC_DIR"/eth_usb.sh android out");
	if(getAndroidSetupStatus()==WIRE_ANDROID_SETUP_START)
	{
		setAndroidSetupStatus(WIRE_ANDROID_SETUP_STOP);
		#if (EZWIRE_TYPE==MIRAPLUG)
		WifiDisplayStop();
		#endif
	}
	return 0;
}


static int libusb_handle(){
	libusb_device **devs;
	libusb_device *dev;
	struct libusb_device_descriptor desc;
	struct libusb_config_descriptor *config = NULL;
	int i = 0, r;
	ssize_t cnt;
	WLOGI("USB handle\n");

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

	//print_devs(devs);
	while ((dev = devs[i++]) != NULL) {
		r = libusb_get_active_config_descriptor(dev, &config);
		if(r != 0){
			WLOGE("Get config discriptor fail!\n");
			//return r;
		}
		WLOGI("num_altsetting: %d, bNumInterfaces: %d\n", config->interface->num_altsetting, config->bNumInterfaces);
		if(config->bNumInterfaces == 1 && config->interface->num_altsetting == 1){
			if(config->interface->altsetting->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE)
			{
				//dVender = -8;
				system(AM_CASE_SC_DIR"/usb_process.sh host_mass_in");
			}
		}
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return r;
		}
		WLOGI("%04x:%04x:%02x (bus %d, device %d)\n",
			desc.idVendor, desc.idProduct, desc.bDeviceClass,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));
		if(desc.bDeviceClass != HUB_DEV) {
			if(desc.idVendor == IPHONE_VENDOR){
				dVender = desc.idVendor;
				#if (MODULE_CONFIG_ADB_MIRROR_ONLY!=1)
				iphone_in(dev);
				#endif
			}else{
				#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)
				android_in(dev);
				#endif
			}
		}
	}
	
	libusb_free_device_list(devs, 1);
	
	return 0;
}
static int check_vram()   //return value >0  vram read only  , =0 rw
{
	FILE *fp = NULL;
	char buf[24]={0};
	int ret=0;
	system("mount | grep vram | grep -c ro > /tmp/vram_check_result");
	fp=fopen("/tmp/vram_check_result","r");
	if(fp!=NULL)
	{
		memset(buf,0,sizeof(buf));
		ret = fread(buf, 1, sizeof(buf), fp);	
		fclose(fp);
		ret=atoi(buf);
		if(ret>0) //value >0  vram read only  ,format vram
		{
			system("echo 1 > /tmp/reset.txt");
			system("sync");
		}
		return ret;
	}		
		
	return 0;
}
static int wire_deal_usb_msg(struct am_sys_msg *msg)
{

	char app_path[50];
	// process ralink dongle switch problem,beacause wo should block usb message to avoid load or unload wifi driver 
	//Block_RalinkMessageForSwitchFunction(msg);
	printf("<%s %d>msg->subtype=====%x,msg->dataload=====%x reserved:%0x\n",__FUNCTION__,__LINE__,msg->subtype,msg->dataload,msg->reserved[0]);
	sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
	switch(msg->subtype){
		case DEVICE_USB_IN:
			break;
		case DEVICE_USB_OUT:
			break;
		case HOST_USB_IN:
			if(msg->dataload == HOST_RAW_NEXT || msg->dataload == HOST_RAW)
			{
				libusb_handle();
				return 0;
			}
			switch(msg->dataload){
				case HOST_RAW:
					sprintf(app_path,"%s %s",app_path,"host_raw_in");
					break;
			
				case HOST_RAW_NEXT:  
					sprintf(app_path,"%s %s",app_path,"host_raw_in1");
					break;
		
				case HOST_MASS_STORAGE:
					sprintf(app_path,"%s %s",app_path,"host_mass_in");
#if  EZCAST_ENABLE
					//usbplugin_afterup=boot_complete_status();
#endif
					break;
					
				case HOST_MASS_STOARGE_SCANOK:
					//identy_usb_storage();
					//sprintf(app_path,"%s %s",app_path,"host_mass_scanok");
					break;
			
				case HOST_USB_HID:
					sprintf(app_path,"%s %s",app_path,"host_hid_in");
					//_inform_mouse_in();
					break;
				case HOST_USB_UVC:
					sprintf(app_path,"%s %s",app_path,"host_uvc_in");
					break;
				case HOST_WLAN:
					sprintf(app_path,"%s %s",app_path,"host_sta_in");
					//dongle_in(msg->reserved[0]);  //before svn8515 store vidpid in reserverd[0] 
					//dongle_in(msg->reserved[1]);     //after svn8515 store vidpid in reserverd[1]
					break;
				case HOST_USB_BLUETOOTH:
					sprintf(app_path,"%s %s",app_path,"host_bt_in");
					break;
				default:
					break;
			}
			break;
		case HOST_USB_OUT:
			if(msg->dataload == HOST_RAW_NEXT || msg->dataload == HOST_RAW)
			{
				printf("dVender: 0x%04x\n", dVender);
				if(dVender == 0x5ac)
				{
					#if MODULE_CONFIG_ADB_MIRROR_ONLY!=1
					iphone_out();
					#endif
					dVender = -1;
					check_vram();//format vram or not 


					return 0;
				}else if(dVender > 0){
					android_out();
					dVender = -1;
				}else if(dVender == -8){
					;
				}else if(dVender ==-3){
					;
				}
				else{
					;
				}
				#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)
				adb_stop();
                            #endif
				check_vram();//format vram or not 
				return 0;
			}
			break;
		default:
			return 0; 
	}
	pid_t p_status;
	printf("app_path =%s\n",app_path);
	p_status = system(app_path);
	return 0;
}

static int procee_normal_usbstorage(int msg_type)
{
	int fd = -1;
	char call_buf[128]={0x00};
	sprintf(call_buf,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
	char buf[2048] ={0x00};
	printf(" %s %d msg_type:%x\n",__func__,__LINE__,msg_type);	
	if(access("/proc/bus/usb/devices",F_OK)==0){
		fd = open("/proc/bus/usb/devices",O_RDONLY);
		read(fd,buf,sizeof(buf));
		
		if(access("/proc/bus/usb/01",F_OK)==0&&(access("/proc/bus/usb/02",F_OK)==0)){
			printf("usb bus have two device\n");
		}
		else
		{
			if(NULL==strstr(buf,"Driver=rtl8192cu")&&access("/sys/module/8192cu",F_OK)==0){
				system("rmmod -f 8192cu");
			}
			
			if(NULL==strstr(buf,"Driver=rt2870")&&access("/sys/module/rt3070ap",F_OK)==0){
				system("modprobe -r rtnet3070ap");
			}else if(NULL==strstr(buf,"Driver=rt2870")&&access("/sys/module/rt5370sta",F_OK)==0){
				system("rmmod -f rt5370sta");
			}
#if 1
			if(msg_type==0xa0){
				if(access("/dev/sda",F_OK)==0&&(access("/dev/sdb",F_OK)!=0)){
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcd");
				}else if(access("/dev/sda",F_OK)!=0&&access("/dev/sdb",F_OK)==0){
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcd");
				}else{
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcdall");
				}
				system(call_buf);
			}

			if(msg_type==0xa1){
				if(access("/dev/sda",F_OK)==0&&(access("/dev/sdb",F_OK)!=0)){
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcdnext");
				}else if(access("/dev/sda",F_OK)!=0&&access("/dev/sdb",F_OK)==0){
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcdnext");
				}else{
					sprintf(call_buf,"%s %s",call_buf,"host_mass_out_hcdnextall");
				}
				system(call_buf);
			}
#endif	
		}
		
	}else{
		if(access("/sys/module/usb_storage",F_OK)==0){
			//printf("%s %d==========\n",__FUNCTION__,__LINE__);
			sprintf(call_buf,"%s %s",call_buf,"host_mass_out");
			system(call_buf);
		}else if(access("/sys/module/8192cu",F_OK)==0){
			system("rmmod -f 8192cu");
		}
	}
	return 0;
}

static int wire_usb_event_direction(int usb_event)
{
	int direct=-1;
	
	switch(usb_event){
		case DEVICE_USB_IN:
		case HOST_USB_IN:
			direct = IN;
		break;
		case DEVICE_USB_OUT:
		case HOST_USB_OUT:
			direct = OUT;
		break;

		default:
			direct = -1;
		break;
	}
	return direct;
}

static char * get_bdev_name(int udisknum)
{
	char *device_name=NULL;
	if(udisknum==0x0a)
		device_name="/dev/sda";
	else if(udisknum==0x0b)
		device_name="/dev/sdb";
	else if(udisknum==0x0c)
		device_name="/dev/sdc";
	else 
		device_name="/dev/sdd";

	return device_name;	
}

static int identy_usb_storage(int num,char *devname)
{
	int rtn=-1;
	int port_num=num;
	char sys_buf[128]={0x00};
	printf("<%s %d> port_num:%d device_name:%s\n",__func__,__LINE__,port_num,devname);
	if(port_num==USB_STORAGE_BUS_ONE){
		if(strncmp(devname,"/dev/sda",strlen("/dev/sda"))==0)
			sprintf(sys_buf,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","host_mass_scanok_sda_hcd");
		else
			sprintf(sys_buf,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","host_mass_scanok_sdb_hcd");
	}else{
		if(strncmp(devname,"/dev/sda",strlen("/dev/sda"))==0)
			sprintf(sys_buf,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","host_mass_scanok_sda_hcdnext");
		else
			sprintf(sys_buf,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","host_mass_scanok_sdb_hcdnext");
	}
	rtn=system(sys_buf);
	return rtn;
}

int get_usb_path(char *path)
{
	FILE *fp = NULL;
	int ret = -1;

	#define PATH_MAX_LEN		(64)
	
	if(path == NULL){
		printf("[%s][%d] -- Parameter error!!\n", __func__, __LINE__);
		return -1;
	}
	system("df -h | grep \"/dev/sd[a~b]\" | awk '{print $6}' > /tmp/test_tmp");
	fp = fopen("/tmp/test_tmp", "r");
	if(fp != NULL){
		char u_path[PATH_MAX_LEN];
		ret = fread(u_path, 1, PATH_MAX_LEN, fp);
		fclose(fp);
		unlink("/tmp/test_tmp");
		char *end = strchr(u_path, '\n');
		if(end != NULL){
			//printf("[%s][%d] -- %d\n", __func__, __LINE__, end-1);
			char *end_r = strchr(u_path, '\r');
			if(end_r != NULL){
				end = (end < end_r)?end:end_r;
			}
			int len = end - u_path;
			memcpy(path, u_path, len);
			path[len] = '\0';
		}else{
			snprintf(path, PATH_MAX_LEN, "%s", u_path);
		}
		printf("[%s][%d] -- usb path: %s\n", __func__, __LINE__, path);
		if(ret > 0)
			return 0;
		else
			return -1;
	}

	return -1;
}

static int do_process_usb_next(struct am_sys_msg msg)
{
	char *device_name = NULL;
	
	WLOGI("<01> type=0x%x,subtype=%x,dataload=%x,usb_stat_next=%d,reserved[0]=0x%x\n",msg.type,msg.subtype,msg.dataload,usb_stat_next,
		msg.reserved[0]);

		if(msg.subtype == DEVICE_USB_VENDOR){
			;
		}
		else if(msg.dataload==DEVICE_SUBDISP_2_MASS ||msg.dataload==DEVICE_SUBDISP_QUIT){
			;
		}
		else if(msg.dataload==DEVICE_DEBUG_2_MASS ||msg.dataload==DEVICE_DEBUG_QUIT){
			;
		}else if((msg.subtype==DEVICE_USB_IN || msg.subtype==DEVICE_USB_OUT) 
		&&(msg.dataload==DEVICE_CON_NODPDM || msg.dataload==DEVICE_POWER_ADAPTER)){
			return 0;
	}else{
		switch(usb_stat_next){
			case HOTPLUG_STAT_OUT:
				if((msg.type ==SYSMSG_USB) && (wire_usb_event_direction(msg.subtype) == IN)){
					if(msg.subtype==DEVICE_USB_IN){
						;
					}
					else if(msg.subtype== HOST_USB_IN ){
						#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
						ezwireDrawWindow(WIRE_UI_ARROW_CONNECTING, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
						#endif
						ezwireDrawFlip();
						if(wire_deal_usb_msg(&msg)!=0){
							printf("<01> insmod hcd fail\n");
							usb_stat_next = HOTPLUG_STAT_OUT;
							break;
						}
						usb_stat_next = HOTPLUG_STAT_IN_CHECKING;
						printf("<01> usb host in process\n");
					}
				}
				else{
				//	usleep(50000);
					procee_normal_usbstorage(msg.reserved[0]);
					wire_deal_usb_msg(&msg);
					printf("<01> usb out ok\n");
				}
				break;

			case HOTPLUG_STAT_IN_CHECKING:
				if((msg.type ==SYSMSG_USB) && (wire_usb_event_direction(msg.subtype) == OUT)){
					if(wire_deal_usb_msg(&msg)!=0)
						break;
					usb_stat_next = HOTPLUG_STAT_OUT;
					printf("<01> usb out process\n");
				}
				else if((msg.type ==SYSMSG_USB) && (wire_usb_event_direction(msg.subtype) == IN)){
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_MASS_STOARGE_SCANOK){
						device_name =get_bdev_name(msg.reserved[1]);	
						printf("%s %d device_name:%s\n",__FUNCTION__,__LINE__,device_name);
						identy_usb_storage(1,device_name);
						usleep(500);
						char path[32] = {0};
						char conf[128] = {0};
						get_usb_path(path);
						WLOGI("mount path is %s\n", path);
						OSsprintf(conf, "%s/%s", path,TEST_CONFIG_FILE);
						OSprintf(" ifname = %s\n", conf);
						if(access(conf,	F_OK) == 0){
							WLOGI("do factory test....");
							do_factory_test();
						}
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_HID){
						;
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_UVC){
						;
					}else {
						break;
					}
					/*simple client driver,fsm goes into STAT_IN*/
					if(msg.subtype==HOST_USB_IN&&(msg.dataload==HOST_USB_HID
						||msg.dataload==HOST_WLAN)){//for mouse or other not mass storage device
						;	
					}
					// For Bluetooth Dongle
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_BLUETOOTH){ 
						;
					}
				}
				else if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (wire_usb_event_direction(msg.subtype) == IN)){
					;
				}
				break;
			
			case HOTPLUG_STAT_IN:
				break;

			case HOTPLUG_STAT_OUT_CHECKING:
				break;

			default:
				break;
		}
	}
	return 0;
}

static void *hotplug_thread_handler(void *arg)
{
	struct am_sys_msg msg;

	while(1){

		hotplug_sem_wait(&mh_work.lock_out);
		msg = mh_work.msgfifo[mh_work.rp];
		mh_work.rp++;
		if(mh_work.rp >= MEDIA_MSG_FIFO_LEN){
			mh_work.rp = 0;
		}
		hotplug_sem_post(&mh_work.lock_in);		
		WLOGI("Get Msg rp=%d,Msgtype===0x%x,subtype=0x%x ,dataload=0x%x reserved=0x%0x\n",mh_work.rp,msg.type,msg.subtype,msg.dataload,msg.reserved[0]);
		switch(msg.type){
			case SYSMSG_USB:
				#if AM_CHIP_ID ==1213 ||AM_CHIP_ID==1220
				/*bq:1213 usb message process,this define two status;one is process usb0 ,
				*other is  process usb1;notice if the ic is not belong to 1213 ,_do_rocess_usb()will process it
				*/
				if(msg.reserved[0]==0xa0){
					do_process_usb_next(msg);
					//_do_process_usb(msg);
				}
				else if(msg.reserved[0]==0xa1){
					do_process_usb_next(msg);
				}
				#else
					//_do_process_usb(msg);
				#endif	
				break;
			default:
				break;
		}
	}
	return NULL;
}

int wire_hotplug_handle_thread()
{
	int ret;

	mh_work.rp = 0;
	mh_work.wp = 0;

	if(hotplug_sem_init(&mh_work.lock_out, 0, 0) == -1){
		WLOGE("hotplug out fifo lock sem init error\n");
		return -1;
	}	
	if(hotplug_sem_init(&mh_work.lock_in, 0, MEDIA_MSG_FIFO_LEN - 1) == -1){
		hotplug_sem_destroy(&mh_work.lock_out);
		WLOGE("hotplug in fifo sem init error\n");
		return -1;
	}

	ret = pthread_create(&hotplug_tid, NULL, hotplug_thread_handler, NULL);
	if(ret != 0){
		WLOGE("hotplug thread create error\n");
		hotplug_sem_destroy(&mh_work.lock_in);
		hotplug_sem_destroy(&mh_work.lock_out);
		return -1;
	}

	usb_stat_next = HOTPLUG_STAT_OUT;
	hotplug_handle_thread = 1;
	return 0;
}

