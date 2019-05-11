#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "media_hotplug.h"
#include "sys_msg.h"
#include "sys_cfg.h"
#include "swf_types.h"
#include "subdisplay.h"
#include "fui_common.h"
#include <fcntl.h>
#include <errno.h>
#include "system_info.h"
#include <sys/wait.h>    
#include <sys/types.h> 
#include "ezcast_public.h"
#include <libusb.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "am7x_dac.h"
#if (EZWIRE_CAPTURE_ENABLE != 0)
#include "imirror.h"
#include "wire_player.h"
#include <dlfcn.h>
#include "display.h"
struct imirrorData_s{
	void *audioHandle;
	int oldVolume;
};

#endif

#define	USB_PARTITION_DEV		"/dev/sda"
#define USB_TIMER_INTERVAL    1000
#define HUB_DEV			0x09
#define IPHONE_VENDOR	0x5ac

int  mtp_in_usb=0;
int  usbplugin_afterup=0; //0 plug in before Dongle boot,1 plug in after dongle boot
int  sdcardplugin_afterup=0;
extern int up_usbcard_loopbackplay_status;
static int initFlag = -1;

extern int get_bdev_partnums(const char *disk_device);
struct media_hotplug_work hotplug_work;
static pthread_t hotplug_tid;
int hotplug_work_inited = 0;
struct am_sys_msg 	bak_msg;//current msg backup,use it later
static int usb_timer_id=-1;
static int bad_disk=0;	/*flag for some bad disk*/
/** state machine control variables */
int card_stat;
int usb_stat;
int usb_stat_next;
int cf_stat;
int  id_hcd_ko=HCD_WIFI_DEFAULT;
int card_block_num=0;//when connecting pc, note which dev block should be used 
int cf_block_num=0; //when connecting pc, note which dev block should be used 

int pc_con=0;
int usbdisplay_sel=-1;
int UsbPort=0;
extern int sd_in;
extern int cardin_type;
extern int cf_in;
extern int wlan_in;
extern int bluetooth_in;
extern int multisector_num[2];
extern int identy_hcd_flag;
extern int udisk1_in;
extern int udisk2_in;

#define IDEVICEINFO_SIMPLE_FILE_TYPE	"/tmp/ideviceinfo.ProductType"
#define IDEVICEINFO_SIMPLE_FILE_VERSION	"/tmp/ideviceinfo.ProductVersion"


#define OSDICON_PALETTE		"/am7x/case/data/osdIconPalette.plt"
#define OSDPROMPT_PATH		"/am7x/case/data/OSD_RGB_Default.bin"
#define OSDSHOW_DELAY_TIME	(15*60)		// seconds
#define OSDSHOW_DELAY_TIME2	(30*60)		// seconds
#define OSDSHOW_TIME		(10)		// seconds


static int osdIconOn = 0;

static int is_hub_device = 0;

struct audioSineInfo_s {
	time_t audioStartTime;
	int osdHidTime;
	char audioInit;
	char sineReceive;
};
static struct audioSineInfo_s audioSineInfo;




#if FUN_WIRE_ENABLE
int dVender = -1;
int adbStartFlag = 0;
int lastVolume = -1;
static int aoa_to_adb_flag=0;
#endif

#if (EZWIRE_CAPTURE_ENABLE != 0)
int catptureStart = 0;
int ValeriaStartStatus = 0;
#endif

#if 1
extern struct mass_dev_t  double_dev[2];
#endif
//extern int change_modeing;

static int _update_usb_status(int media, int stat);
static int _inform_application();
static int _deal_usb_msg(struct am_sys_msg *msg);

#ifdef PNP_USE_WIFIDISPLAY
extern void ezFuiWirePlugplayConnect(void);

extern void ezFuiWirePlugplayDisconnect(void);

extern int ezFuiWirePlugplayPlay(unsigned char *stream, unsigned int length, unsigned long display_width, unsigned long display_height);

extern int ezFuiWireH264ParamSet(unsigned char *sps_pps, unsigned int length);

extern  void ezFuiWireStopMirrorCallbackSet(int (*cb_func)(void));
#endif

char dev_block[4][20]=
{
	{"/dev/sd_card_block"},
	{"/dev/ms_card_block"},
	{"/dev/xd_card_block"},
	{"/dev/cf_card_block"},
};
char dev_des[8][20]=
{
	{"/dev/sd_card_block"},
	{"/dev/ms_card_block"},
	{"/dev/xd_card_block"},
	{"/dev/cf_card_block"},
	
	{"/dev/sd_card_block1"},
	{"/dev/ms_card_block1"},
	{"/dev/xd_card_block1"},
	{"/dev/cf_card_block1"},
};

char disk_file[3][40] =
{
	{"/sys/devices/gadget/gadget-lun1/file"},
	{"/sys/devices/gadget/gadget-lun2/file"},
	{"/sys/devices/gadget/gadget-lun3/file"},
};

int check_connect_pc()
{
	return pc_con;
}

static int delete_usb_timer()
{
	if(usb_timer_id > 0){
		am_timer_del(usb_timer_id);
		usb_timer_id = -1;
	}
	return 0;

}

extern int is_video_stream;
extern int is_audio_stream;
extern int is_photo_stream;
extern int dmr_is_running;

static int special_deal_for_ez_stream()  
{  
#if 1	
	if(is_video_stream ||is_audio_stream || is_photo_stream){
		printf("special_deal_for_ez_stream\n");
		am_stream_stop(NULL); 
		//dlna_stop_work();
	}

#endif 	


    return 0;  
}  
//bluse: solution the scsi host generate dev node
char * get_bdev_name(int udisknum)
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
#if 1
static void usb_timer_handle(void *param)
{
	int fd;
	char *mass_name=NULL;
	int udisk_flag=0x0a;
	mass_name = get_bdev_name(udisk_flag);

	fd = open(mass_name,O_RDONLY);
	if(fd<0)
		printf("still bad disk\n");
	else{
		printf("can be access\n");
		if(!_deal_usb_msg(&bak_msg)){
			bad_disk = 0;
			multisector_num[0] = get_bdev_partnums(USB_PARTITION_DEV);
			_update_usb_status(HOST_MASS_STORAGE,bak_msg.subtype);
			_inform_application();
			memset(&bak_msg,0,sizeof(struct am_sys_msg));
			printf("multisector_num===%d\n",multisector_num[0]);
		}
		else
			usb_stat = HOTPLUG_STAT_IN;
		delete_usb_timer();
	}
	close(fd);

}

static int create_usb_timer()
{
	if(usb_timer_id <= 0){
		usb_timer_id = am_timer_create(USB_TIMER_INTERVAL,usb_timer_handle,NULL);
		if(usb_timer_id <= 0){
			usb_timer_id = -1;
		}
	}
	
	return 0;
}
#endif
static void _hotplug_sem_wait(sem_t * sem)
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
static void _hotplug_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

static int get_cards_block_num(int type)
{
	char tmp_disk_device[20];
	int block_num=0;
	switch(type){
		case SD_IN:
		case SD_OUT:
		case MMC_IN:
		case MMC_OUT:
			memcpy(tmp_disk_device,dev_block[0],20);
			break;
		case MS_IN:
		case MS_OUT:
		case MS_Pro_IN:
		case MS_Pro_OUT:
			memcpy(tmp_disk_device,dev_block[1],20);
			break;
		case XD_IN:
		case XD_OUT:
			memcpy(tmp_disk_device,dev_block[2],20);
			break;
		case CF_IN:
		case CF_OUT:
			memcpy(tmp_disk_device,dev_block[3],20);
			break;
		default:
			printf("%s,%d:Error Type=%d\n",__FILE__,__LINE__,type);
			return -1;
			break;
			
	}	
	block_num = get_card_mbrinfo(tmp_disk_device);
	return block_num;
}

int device_info_confirm(unsigned int dir,int type,int block_num)
{
	int fd;
	int n;
	char syscmd[50];
	char dev_name[40];
	char tmp_diskfile[40];
	char tmp_dev_des[20];
	memset(dev_name,0,sizeof(dev_name));
	memset(tmp_diskfile,0,40);
	memset(tmp_dev_des,0,20);
	switch(type){
		case SD_IN:
		case SD_OUT:
		case MMC_IN:
		case MMC_OUT:
			memcpy(tmp_dev_des,dev_des[0+block_num*4],20);
			memcpy(tmp_diskfile,disk_file[0],40);
			break;
		case MS_IN:
		case MS_OUT:
		case MS_Pro_IN:
		case MS_Pro_OUT:
			memcpy(tmp_dev_des,dev_des[1+block_num*4],20);
			memcpy(tmp_diskfile,disk_file[0],40);
			break;
		case XD_IN:
		case XD_OUT:
			memcpy(tmp_dev_des,dev_des[2+block_num*4],20);
			memcpy(tmp_diskfile,disk_file[0],40);
			break;
		case CF_IN:
		case CF_OUT:
			memcpy(tmp_dev_des,dev_des[3+block_num*4],20);
			memcpy(tmp_diskfile,disk_file[1],40);
			break;
		default:
			printf("%s,%d:Error Type=%d\n",__FILE__,__LINE__,type);
			break;
			
	}
	
	fd = open(tmp_diskfile,O_RDWR);
	if(fd<0)
	{
		printf("disk file info oen fail\n");
		return -1;
	}
	printf("%s,%d: DEV_NAME=%s\n",__FILE__,__LINE__,tmp_dev_des);
	if(dir==1)
	{	
		dev_name[0] = '"';
		strcat(dev_name, tmp_dev_des);
		dev_name[strlen(dev_name)] = '"';
		sprintf(syscmd,"echo  %s >%s",dev_name,tmp_diskfile);
	}
	else
	{
		dev_name[0] = '"';
		dev_name[1] = '"';
		sprintf(syscmd,"echo %s >%s",dev_name,tmp_diskfile);
	}
	close(fd);
	printf(":::%s\n",syscmd);	
	system(syscmd);
	return 0;
}

static int _inform_mouse_in()
{
	SWF_Message(NULL,SWF_MSG_KEY_USB_HID_IN,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);

	return 0;
}
static int _inform_mouse_out()
{
	SWF_Message(NULL,SWF_MSG_KEY_USB_HID_OUT,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);

	return 0;
}

int _inform_ezlauncher_in()
{
	SWF_Message(NULL,SWF_MSG_KEY_EZLAUNCHER_IN,NULL);
	return 0;
}

int _inform_ezlauncher_out()
{
	SWF_Message(NULL,SWF_MSG_KEY_EZLAUNCHER_OUT,NULL);
	printf("-----_inform_ezlauncher_out\n");
	return 0;
}
static int _inform_switch_aoa()
{
	SWF_Message(NULL,SWF_MSG_KEY_SWITCH_AOA,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	//printf("-----_inform_switch_aoa  0x%x\n",SWF_MSG_KEY_SWITCH_AOA);
	

	return 0;
}
static int _inform_switch_adb()
{
	SWF_Message(NULL,SWF_MSG_KEY_SWITCH_ADB,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	//printf("-----_inform_switch_adb  0x%x\n",SWF_MSG_KEY_SWITCH_ADB);
	return 0;
}

 int _inform_adb_stop()
{
	SWF_Message(NULL,SWF_MSG_KEY_ADB_STOP,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	//printf("-----_inform_adb_stop  0x%x\n",SWF_MSG_KEY_ADB_STOP);
	return 0;
}
static int _inform_application()
{
	SWF_Message(NULL,SWF_MSG_KEY_MEDIA_CHANGE,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);

	return 0;
}


int _inform_application_device_in()
{
	SWF_Message(NULL,SWF_MSG_KEY_USB_IN,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	return 0;
}

int _inform_application_device_out()
{
	SWF_Message(NULL,SWF_MSG_KEY_USB_OUT,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	pc_con = 0;
	return 0;
}

static int _update_card_status(struct am_sys_msg *msg)
{
	switch(msg->subtype){
		
		case SD_IN:
		case MMC_IN:
		case MS_IN:
		case MS_Pro_IN:
		case XD_IN:
			sd_in = 1;
			cardin_type = (msg->subtype&0x0F);
		break;

		case SD_OUT:
		case MMC_OUT:
		case MS_OUT:
		case MS_Pro_OUT:
		case XD_OUT:
			sd_in = 0;
			cardin_type = (msg->subtype&0x0F);
		break;

		case CF_IN:
			cf_in = 1;
		break;

		case CF_OUT:
			cf_in = 0;
		break;
		
		default:
			
		break;
	}
	
	return 0;
}
//extern int change_modeing;
static int _update_usb_status(int media, int stat)
{
	if(media == HOST_WLAN){
	
		if(stat == HOST_USB_IN){
			wlan_in = 1;
			fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		}
		else{
			wlan_in = 0;
		fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

		}
	}
	else if(media == HOST_USB_BLUETOOTH){
	
		if(stat == HOST_USB_IN){
			bluetooth_in = 1;
		}
		else{
			bluetooth_in = 0;
		}
	}
printf("[debug %s %d] >>>>>>>>>>>wlan_in=%d\n",__func__,__LINE__,wlan_in);
	return 0;
}

static int _update_usb_storage_device_status(int media, int stat,int hcd_num)
{
	if(hcd_num==1){
		if(media == HOST_MASS_STORAGE){
			if(stat == HOST_USB_IN){
				udisk1_in = 1;
			}
			else {				
				udisk1_in = 0;
			}
		}
	}else{
		if(media == HOST_MASS_STORAGE){
			if(stat == HOST_USB_IN){
				udisk2_in = 1;
			}
			else {
				udisk2_in = 0;
			}
		}
	}
	printf("%s %d udisk1_in:%d udisk2_in:%d\n",__FUNCTION__,__LINE__,udisk1_in,udisk2_in);
	return 0;
}

static void _backup_usb_msg(struct am_sys_msg *msg)
{
	bak_msg.type = msg->type;
	bak_msg.subtype = msg->subtype;
	bak_msg.dataload = msg->dataload;

	printf("%s,%d: type=0x%x,subtype=%x,dataload=%x\n",__FUNCTION__,__LINE__,bak_msg.type,bak_msg.subtype,bak_msg.dataload);
}
static int _deal_card_msg(struct am_sys_msg *msg)
{
	char app_path[80];
	char card_status[10];

	memset(app_path,0,sizeof(app_path));
	memset(card_status,0,sizeof(card_status));
	
	sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"card_process.sh");
	
	switch(msg->subtype){
		
		case SD_IN:
		case MMC_IN:
			sprintf(app_path,"%s %s",app_path,"sdin");
#if  EZCAST_ENABLE
			sdcardplugin_afterup=boot_complete_status();
#endif 
			break;
		case MS_IN:
		case MS_Pro_IN:
			sprintf(app_path,"%s %s",app_path,"msin");
			break;

		case SD_OUT:
		case MMC_OUT:
			_update_card_status(msg);
			sprintf(app_path,"%s %s",app_path,"sdout");
			break;
		case MS_OUT:
		case MS_Pro_OUT:
			sprintf(app_path,"%s %s",app_path,"msout");
			break;

		case XD_IN:
			sprintf(app_path,"%s %s",app_path,"xdin");
		break;

		case XD_OUT:
			sprintf(app_path,"%s %s",app_path,"xdout");
		break;
		
		case CF_IN:
			sprintf(app_path,"%s %s",app_path,"cfin");
		break;

		case CF_OUT:
			sprintf(app_path,"%s %s",app_path,"cfout");
		break;
		
		default:
			app_path[0]='\0';
			break;
	}

	if(pc_con)
		sprintf(card_status,"%s","pc_con");
	else 
		sprintf(card_status,"%s","pc_discon");

	sprintf(app_path,"%s %s",app_path,card_status);
	
	return system(app_path);
}

//bluse:add  to solution the normal plug in
int procee_normal_usbstorage(int msg_type)
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

//cdrom function
int find_iso_dmg_file(void)
{
	char call_buf[50];
	char call_buf1[50];
	FILE *fp=NULL;
	char *str_info = NULL;
	char  *dmg_iso_name=NULL;
	int ret =-1;
	sprintf(call_buf,"fuser -k /mnt/cdrom\numount /mnt/cdrom");
	system(call_buf); //process more start cdrom functions
	memset(call_buf,0x00,50);
	sprintf(call_buf,"find /mnt/cdrom/");
	fp = popen(call_buf,"r");
	if(fp==NULL){
		printf("open error\n");
		goto err_flag;
	}

        while(fgets(call_buf1, sizeof(call_buf1), fp) != NULL)
    	{
    		str_info = call_buf1;
		if(strstr(call_buf1,".iso") !=NULL ){
			dmg_iso_name=call_buf1;
			ret=0;
			break;
		}
        }
	printf("%s %d &====& filename=%s\n",__FUNCTION__,__LINE__,dmg_iso_name);

	pclose(fp);
	return ret;
	
err_flag:
	pclose(fp);
	printf(" %s process error\n",__FUNCTION__);
	dmg_iso_name=NULL;
	return -1;
}
// cancel the cdrom function
int cancel_cdrom_auto_func()
{
	int ret = -1;
	char call_buf[100]={0x00};
	strcpy(call_buf,"umount /mnt/cdrom");
	system(call_buf);

	strcpy(call_buf,"rmmod g_file_storage");
	ret = system(call_buf);
	if(ret != 0)
		goto err_flage;
	sleep(1); 
	//resume the udisk states
	strcpy(call_buf,"insmod /lib/modules/2.26.27.29/g_file_storage.ko file=/dev/partitions/udisk");
	system(call_buf);
	if(ret != 0)
		goto err_flage;
	
	return 0;
err_flage:
	printf("error happen\n");
	return -1;	
}
#ifdef CASE_USB_COMPSITE_DEVICE  //usb compsite device information
#define sysdbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define sysdbg_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define USB_COMPSITE_DEV_MAX 2
struct usb_compsite_dev_pool {
	int intf_total;
	char dev_name[64];  //store usb compsite device name
	char intface_class[16][64]; //store interface class value
};
struct compsite_dev_info_buf{
	char dev_info_buf[64];
};
struct usb_compsite_dev_info {
	struct usb_compsite_dev_pool dev_pool[USB_COMPSITE_DEV_MAX];
	int dev_total;
};
struct usb_compsite_dev_info usb_compsite;
void dump_the_usb_compsite_device_info(){
	int i=0;
	int j=0;
	for(i=0;i<usb_compsite.dev_total;i++)
		for(j=0;j<usb_compsite.dev_pool[i].intf_total;j++){
			printf("dev_index==%d,interface_index==%d,device_name=%s,interface_name=%s\n",i,j,
				usb_compsite.dev_pool[i].dev_name,usb_compsite.dev_pool[i].intface_class[j]);
		}
}
int _get_usb_compsite_device_info()
{
	int ret = -1;
	FILE *fp =NULL;
	char tmp_buf[128]={0};
	int usb_device_num= 0,j=0,intf_num=0;
	memset(&usb_compsite,0x00,sizeof(struct usb_compsite_dev_info));
	if(access("/proc/driver/usbcore",F_OK)==0){
		fp = fopen("/proc/driver/usbcore","r");
		if(fp ==NULL){
			printf("open proc/driver/usb_compsite error\n");
		}
		while(!feof(fp)){
			memset(tmp_buf,0x00,128);
			fgets(tmp_buf,128,fp);
			if(strstr(tmp_buf,"product")){
				strcpy(usb_compsite.dev_pool[usb_device_num].dev_name,tmp_buf+8);
				intf_num=0;
				//printf("debug====%s %d usb_device_num==%d,dev_name=%s\n",__FUNCTION__,__LINE__,usb_device_num,
				//usb_compsite.dev_pool[usb_device_num].dev_name);	
			}else if(strstr(tmp_buf,"binterfaceclass")){
				strcpy(usb_compsite.dev_pool[usb_device_num].intface_class[intf_num],tmp_buf+18);
				//printf("debug====%s %d usb_device_num=%d,intf_num=%d,interface_class=%s\n",__FUNCTION__,__LINE__,usb_device_num,intf_num,
				//	usb_compsite.dev_pool[usb_device_num].intface_class[intf_num]);
				intf_num++;
			}else if(strstr(tmp_buf,"total")){
				usb_compsite.dev_pool[usb_device_num].intf_total = intf_num;
				usb_device_num++;
				intf_num = 0;
			}
		}
		usb_compsite.dev_total = usb_device_num;
		fclose(fp);
	}
	else{
		printf("%s %d\n",__FUNCTION__,__LINE__);
	    ;//do nothing here
	}
	//dump_the_usb_compsite_device_info();
	return 0;
}
int _usb_compsite_device_total()
{
	return usb_compsite.dev_total;
}
int _usb_compsite_device_interface_total(int dev_num)
{
	return usb_compsite.dev_pool[dev_num].intf_total;
}
int _get_usb_compsite_device_name_info(int dev_idex,char cpt_dev_buf[64])
{
	memset(cpt_dev_buf,0x00,64);
	strncpy(cpt_dev_buf,usb_compsite.dev_pool[dev_idex].dev_name,strlen(usb_compsite.dev_pool[dev_idex].dev_name));
	return 0;
}
 int _get_usb_compsite_device_interfaceclass_info(int dev_idex,int interface_idex ,char cpt_dev_buf[64])
{
	int ret = -1;
	memset(cpt_dev_buf,0x00,sizeof(cpt_dev_buf));
	if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"01",2)==0){
		strcpy(cpt_dev_buf,"audio");
		ret = USB_COMPSITE_AUDIO;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"02",2)==0){
		strcpy(cpt_dev_buf,"comm");
		ret = USB_COMPSITE_COMM;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"03",2)==0){
		strcpy(cpt_dev_buf,"hid");
		ret = USB_COMPSITE_HID;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"05",2)==0){
		strcpy(cpt_dev_buf,"physical");
		ret = USB_COMPSITE_PHYSICAL;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"06",2)==0){
		strcpy(cpt_dev_buf,"still image");
		ret = USB_COMPSITE_STILL_IMAGE;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"07",2)==0){
		strcpy(cpt_dev_buf,"printer");
		ret = USB_COMPSITE_PRINTER;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"08",2)==0){
		strcpy(cpt_dev_buf,"mass storage");
		ret = USB_COMPSITE_MASS_STORAGE;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"09",2)==0){
		strcpy(cpt_dev_buf,"hub");
		ret = USB_COMPSITE_HUB;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"0a",2)==0){
		strcpy(cpt_dev_buf,"cdc data");
		ret = USB_COMPSITE_CDC_DATA;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"0b",2)==0){
		strcpy(cpt_dev_buf,"cscid");
		ret =USB_COMPSITE_CSCID;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"0d",2)==0){
		strcpy(cpt_dev_buf,"content sec");
		ret = USB_COMPSITE_CONTENT_SEC;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"0e",2)==0){
		strcpy(cpt_dev_buf,"video");
		ret = USB_COMPSITE_VIDEO;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"e0",2)==0){
		strcpy(cpt_dev_buf,"controller");
		ret =  USB_COMPSITE_WIRELESS_CONTROLLER;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"ef",2)==0){
		strcpy(cpt_dev_buf,"misc");
		ret = USB_COMPSITE_MISC;
	}else if(strncmp(usb_compsite.dev_pool[dev_idex].intface_class[interface_idex],"fe",2)==0){
		strcpy(cpt_dev_buf,"app spec");
		ret = USB_COMPSITE_APP_SPEC;
	}else {
			strcpy(cpt_dev_buf,"vendor spec");
			ret =USB_COMPSITE_VENDOR_SPEC;
		}

	printf("%s %d ret = %d\n",__FUNCTION__,__LINE__,ret);
	//memset(cpt_dev_buf,0x00,sizeof(cpt_dev_buf));
	printf("%s %d cpt_dev_buf= %s\n",__FUNCTION__,__LINE__,cpt_dev_buf);	
	//strcpy(cpt_dev_buf,usb_compsite.dev_pool[dev_idex].intface_class[interface_idex]);
	return ret;
}

int mtp_device_in=0;

void mtpfs_in_out_flag_change(){
	switch(mtp_in_usb){
		case 1:
			if(mtp_device_in)		//if the mtp device in
				udisk1_in=1;
			else{				//if the mtp device out
				udisk1_in=0;
				mtp_in_usb=0;
			}				
			break;
		case 2:
			if(mtp_device_in)
				udisk2_in=1;
			else{
				udisk2_in=0;
				mtp_in_usb=0;
			}	
			break;
		default:
			break;
	}
}

/*do process about the mtp in or out*/
void do_usb_process(char *cmd){
	char call_buf[256];

	memset(call_buf,0,sizeof(call_buf));
	sprintf(call_buf,"sh /am7x/case/scripts/usb_process.sh %s",cmd);
	printf("call_buf is =========== %s \n",call_buf);
	system(call_buf);	
}


int _choose_usb_compsite_device_function(int dev_num,int intf_num,int class_type)
{
	char call_buf[256];
	int ret = 0;
	switch(class_type){
	case 0x06:
#ifdef MODULE_CONFIG_MTP_ENABLE			//enable the mtp surpports
		printf("[debug info:%s,%d]:enable the mtp<<<<<<<<<<<<<<<<<<<\n",__FUNCTION__,__LINE__);
		do_usb_process("mtp_device_in");
		
		if(access("/mnt/mtpfs/Playlists",F_OK)==0)
		{
			//_update_usb_status(HOST_MASS_STORAGE,HOST_USB_IN);
			mtp_device_in=1;
			mtpfs_in_out_flag_change();			
			_inform_application();
		}
		else
		{
			printf("Mount MTP Device Fail!\n");
			do_usb_process("mtp_device_out");
		}
#else									//disable the mtp surpports
		printf("[debug info:%s,%d]:mtp is disable<<<<<<<<<<<<<<<<<<<\n",__FUNCTION__,__LINE__);
		ret =  -1;
#endif		
		break;
	case 0x08:
		//printf("=======");
		memset(call_buf,0x00,256);
		sprintf(call_buf,"insmod /lib/modules/2.6.27.29/usb-storage.ko");
		system(call_buf);
		break;
	case 0xff:
#ifdef MODULE_CONFIG_MTP_ENABLE			//enable the mtp surpports
		printf("[debug info:%s,%d]:enable the mtp<<<<<<<<<<<<<<<<<<<\n",__FUNCTION__,__LINE__);
		do_usb_process("mtp_device_in");

		if(access("/mnt/mtpfs/Playlists",F_OK)==0)
		{	
			mtp_device_in=1;
			mtpfs_in_out_flag_change();
			_inform_application();
		}
		else
		{
			printf("Mount MTP Device Fail!\n");
			do_usb_process("mtp_device_out");
		}	
#else									//disable the mtp surpports
		printf("[debug info:%s,%d]:mtp is disable<<<<<<<<<<<<<<<<<<<\n",__FUNCTION__,__LINE__);
		ret = -1;
#endif		
		break;
	default:		
		break;
	}
	return ret;
}
#endif  //CASE_USB_COMPSITE_DEVICE
enum {
	wifi_ra0_default=-1,
	wifi_ra0_client,
	wifi_ra0_client_identy,
	wifi_ra0_soft_identy,
	wifi_ra0_swich_identy,
	wifi_ra0_softap
}wifi_switch_status;

 int wifi_switch[2];
int identy_ra0_wifi_switch()
{
	char call_buf[10];
	char buf[256];
	FILE *wifi_ko=NULL;
	
	sprintf(call_buf,"lsmod");
	wifi_ko = popen(call_buf,"r");
	if(wifi_ko ==NULL){
		printf("<%s %d>popen error\n",__FUNCTION__,__LINE__);
		return wifi_ra0_default;
	}
	fread(buf, 1, 256, wifi_ko);
	if(strstr(buf,"rt5370sta")!= NULL){
	       //printf("%s %d ========rt5370sta\n",__FUNCTION__,__LINE__);
		wifi_switch[0]=wifi_ra0_client_identy;
	}else if(strstr(buf,"rtnet3070ap")!= NULL){
		printf("%s %d rtnet3070ap\n",__FUNCTION__,__LINE__);
		wifi_switch[1]=wifi_ra0_soft_identy;
	}
	
	pclose(wifi_ko);	
	if(access("/sys/module/rt5370sta",F_OK)==0){
		//printf("%s %d ========rt5370sta\n",__FUNCTION__,__LINE__);
		wifi_switch[0]=wifi_ra0_client_identy;
	}

	if(access("/sys/module/rtnet3070ap",F_OK)==0){
		//printf("%s %d ==============rtnet3070ap\n",__FUNCTION__,__LINE__);
		wifi_switch[1]=wifi_ra0_soft_identy;
	}
	
	printf("%s %d wifi_switch[0]=%d wifi_switch[1]=%d\n",__FUNCTION__,__LINE__,wifi_switch[0],wifi_switch[1]);
	if(wifi_switch[0]==wifi_ra0_client_identy || wifi_switch[1]==wifi_ra0_soft_identy){
		return wifi_ra0_swich_identy;
	}else{
	        return wifi_ra0_default;
	}
}
//extern int vid_pid_from_driver;
enum wifi_ra0_status{
	ra0_default=-1,
	ra0_dongle_in,
	ra0_dongle_out,
};
static int identy_wifi_out()
{
	int fp_wifi=-1;
	char wifi_buf[1024];
	int ret=-1;
	char *find_ra0=NULL;
	memset(wifi_buf,0x00,1024);
	if(access("/proc/bus/usb/devices",F_OK)==0){
		fp_wifi = open("/proc/bus/usb/devices",O_RDONLY);
		if(fp_wifi < 0){
			printf("<%s %d> open file error\n",__FUNCTION__,__LINE__);
			close(fp_wifi);
			return -1;
		}
		fsync(fp_wifi);
		ret=read(fp_wifi,wifi_buf, 1024);
		close(fp_wifi);
		//printf("[%s %d] wifi_buf:%s\n",__func__,__LINE__,wifi_buf);
		find_ra0 = strstr(wifi_buf,"Manufacturer=Ralink");
		if(find_ra0 !=NULL){
			printf("<%s %d>  find_ra0=%s\n",__FUNCTION__,__LINE__,find_ra0);
			return ra0_dongle_in;
		}
		else{
			printf("<%s %d> wifi dongle is out\n",__FUNCTION__,__LINE__);
			return ra0_dongle_out;
		}
	}
	else{
		return ra0_dongle_out;
	}
}
enum _usb_storage_in_flag{
	USB_STORAGE_DEFAULT = 0,
	USB_STORAGE_IN_HCD,
	USB_STORAGE_IN_HCD_NEXT,
	USB_STORAGE_OUT_HCD,
	USB_STORAGE_OUT_HCD_NEXT,
};
enum _usb_storage_bus_flag{
	USB_STORAGE_BUS_DEFAULT = 0,
	USB_STORAGE_BUS_ONE,
	USB_STORAGE_BUS_TWO,
};


int identy_usb_storage(int num,char *devname)
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

//process plug out usb mass storage device;umount /mnt/usb1
static void process_usb_storage_out(int num)
{
	int i=0;
	int port_num=num;
	char sys_buf[128]={0x00};
	if(port_num==0xa0){
		printf("port_num:0x%02x sector_num:%d\n",port_num,double_dev[0].sector_num);
		for(i=1; i <= double_dev[0].sector_num; i++)
			sprintf(sys_buf,"%s umount /mnt/usb%d\n",sys_buf,i);

		double_dev[0].sector_num = 0;
	}
	
	if(port_num==0xa1){
		printf("port_num:0x%02x sector_num:%d\n",port_num,double_dev[1].sector_num);
		for(i=1; i <= double_dev[1].sector_num; i++)
			sprintf(sys_buf,"%s umount /mnt/usb%d\n",sys_buf,i+5);
			
		double_dev[1].sector_num = 0;
	}
	printf(" <%s %d> call_buf:%s \n ",__func__,__LINE__,sys_buf);
	system(sys_buf);
	return;
}
static void Block_RalinkMessageForSwitchFunction(struct am_sys_msg *msg){
	int wifi_flage=0;
	int hcd_use_wifi;
	if(access("/sys/module/rt3070ap",F_OK)==0||access("/sys/module/rt5370sta",F_OK)==0){
		wifi_flage=identy_ra0_wifi_switch();
		printf("<%s %d> wifi_flage=%d id_hcd_ko=%d\n",__FUNCTION__,__LINE__,wifi_flage,id_hcd_ko);
		if(id_hcd_ko==HCD_USE_WIFI)
			hcd_use_wifi=0xa0;
		if(id_hcd_ko==HCD_NEXT_USE_WIFI)
			hcd_use_wifi=0xa1;
		if(wifi_flage ==3 &&(msg->reserved[0]==hcd_use_wifi)){
			int wifi_ra0=-1;;
			wifi_ra0=identy_wifi_out();
			printf("<%s %d> wifi_ra0=%d\n",__FUNCTION__,__LINE__,wifi_ra0);
			if(wifi_ra0 == ra0_dongle_out){
				char call_buf[128];
				int ret=-1;
				special_deal_for_ez_stream();
				OSSleep(500);
				if(access("/sys/module/rt3070ap",F_OK)==0){
					printf("debug %s %d remove wifi ralink driver\n",__func__,__LINE__);
					system("modprobe -r rtnet3070ap");
				}

				if(access("/sys/module/rt5370sta",F_OK)==0){
					printf("debug %s %d remove wifi ralink driver rt5370sta\n",__func__,__LINE__);
					system("rmmod -f rt5370sta");
				}
				OSSleep(1000);
				(id_hcd_ko==HCD_USE_WIFI)? sprintf(call_buf,"rmmod am7x_hcd"):sprintf(call_buf,"rmmod am7x_hcd_next");
				ret=system(call_buf);
				wifi_switch[0]=-1;
				wifi_switch[1]=-1;
				id_hcd_ko=HCD_WIFI_DEFAULT;
			}
			msg->subtype=0xff;
			msg->dataload=0xff;
		}
	}
}
#if FUN_WIRE_ENABLE

#if (EZWIRE_CAPTURE_ENABLE != 0)
static void releaseOsdPicPrompt()
{
	if(osdIconOn)
	{
		osdIconOn = 0;
		osdengine_release_osd();
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
	
	//getScreenSize(&scr_width, &src_height);
	scr_width=screen_output_data.screen_output_width;
	src_height=screen_output_data.screen_output_height;
	EZCASTLOG("scr_width: %d src_height:%d\n", scr_width, src_height);
	int osd_x = x;//(x*scr_width)/1920;
	int osd_y =y;// (y*src_height)/1080;
	int osd_w =w;// (w*scr_width)/1920;
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
		}
		else if(!osdIconOn && ((curTime - audioSineInfo.audioStartTime) >= audioSineInfo.osdHidTime))
		{
			unsigned int scr_width=screen_output_data.screen_output_width;
			unsigned int src_height=screen_output_data.screen_output_height;
			unsigned int x=(scr_width-800)/2;
			unsigned int y=(src_height-80)*7/8;
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

int wireStopMirrorCallback()
{
	EZCASTLOG("To stop mirror\n");
	imirror_ValeriaStop();
	system(AM_CASE_SC_DIR"/usb_process.sh host_adb_stop");
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
static int isiPhoneX();

typedef int (*INITAUDIO)(int , int , int );
typedef void (*SETAUDIOCALLBACK)(int (*)(int, void *));
static INITAUDIO	InitAudio = NULL;
static SETAUDIOCALLBACK	SetAudioCallBack = NULL;

static int wire_initAudio(int SampleRate, int ChannelsPerFrame, int BitsPerChannel)
{
	int ret = -1;
	if(InitAudio)
	{
		ret = InitAudio(SampleRate, ChannelsPerFrame, BitsPerChannel);
		
		if(SetAudioCallBack)
		{
			SetAudioCallBack(AudioCallback);
			audioSineInfo.osdHidTime = OSDSHOW_DELAY_TIME;
			audioSineInfo.audioStartTime = time(NULL);
			audioSineInfo.audioInit = 1;
			audioSineInfo.sineReceive = 0;
		}
	}

	return ret;
}

int initWireDecode(wireDecodeInfo_t *decode)
{
	struct imirrorData_s *imirrorData = NULL;
	int vol = 38;
	
	if(decode == NULL)
	{
		EZCASTWARN("decode is NULL.\n");
		return -1;
	}

	imirrorData = (struct imirrorData_s *)malloc(sizeof(struct imirrorData_s));
	if(imirrorData == NULL)
	{
		EZCASTWARN("imirror data malloc fail.\n");
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
			printf("dac get volume error\n");
		}
		
		imirrorData->oldVolume = vol;
		
		vol = 38;
		
		err = ioctl(fd,DACIO_SET_VOLUME,(unsigned char *)&vol);
		if(err < 0){
			printf("dac get volume error\n");
		}
		
		close(fd);
	}

	imirrorData->audioHandle = dlopen("libaudio_player.so", RTLD_LAZY | RTLD_LOCAL);
	if(imirrorData->audioHandle == NULL)
	{
		EZCASTWARN("open libaudio_player.so fail.\n");
		perror("ERROR");
		return -1;
	}

	decode->userData = imirrorData;
	decode->width = 1280;
	decode->height = 720;
	decode->malloc = malloc;
	decode->free = free;
	InitAudio = OSDLGetProcAddr(imirrorData->audioHandle, "InitAudio");
	//decode->initAudio = OSDLGetProcAddr(imirrorData->audioHandle, "InitAudio");
	SetAudioCallBack = OSDLGetProcAddr(imirrorData->audioHandle, "SetAudioCallBack");
	decode->sendAudioBuff = OSDLGetProcAddr(imirrorData->audioHandle, "SendAudioBuf");
	decode->uninitAudio = OSDLGetProcAddr(imirrorData->audioHandle, "UninitAudio");
	decode->initAudio = wire_initAudio;
#ifdef PNP_USE_WIFIDISPLAY
	decode->initVideo = ezFuiWirePlugplayConnect;
	decode->uninitVideo = ezFuiWirePlugplayDisconnect;
	decode->sendVideoBuff = ezFuiWirePlugplayPlay;
	decode->sendVideoCodec = ezFuiWireH264ParamSet;
#else
	decode->initVideo = wire_HantroOpen;
	decode->uninitVideo = wire_HantroClose;
	decode->sendVideoBuff = wire_HantroPlay;
	decode->sendVideoCodec = wire_SetFrameSPSPPS;
#endif	

	decode->limitResolutionProduct = (getStreamingDecodeMemorySize() / 6 / 3);
	decode->initStreaming = decode_open;
	decode->uninitStreaming = decode_close;
	decode->streamingSeek = decodeSeek;
	decode->streamingSetAudioTime = decodeSetAudioTime;
	decode->streamingResume = decodeResume;
	decode->streamingPause = decodePause;

	//decode->mode = IMIRROR_MODE_DOCKING;
	decode->mode = getImirrorMode();
	EZCASTLOG("Set mirror mode to %s\n", (decode->mode == IMIRROR_MODE_CAMERA)?"IMIRROR_MODE_CAMERA":"IMIRROR_MODE_DOCKING");

	return 0;
}

void uninitWireDecode(wireDecodeInfo_t *decode)
{
	if(decode == NULL)
	{
		EZCASTWARN("decode is NULL.\n");
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
}

int PlugPlayCallback(int cmd, void *retVal)
{
	switch(cmd)
	{
		case PLUGPLAY_CMD_IDEVICEREAD:
			setEzwireStatus(1, EZWIRE_DEV_IOS, EZWIRE_STATUS_CONNECTED);
			ezRemoteSendKey(0x1CE);
			sync();
			break;
		case PLUGPLAY_CMD_ISMIRRORREAD:
#ifdef PNP_USE_WIFIDISPLAY
			return 1;//getPlugPlayStatus();
#else
			return getPlugPlayStatus();
#endif
			break;
		case PLUGPLAY_CMD_MIRRORSTOP:
			{
				if(retVal == NULL)
				{
					EZCASTWARN("retVal is NULL.\n");
					break;
				}
				ezRemoteSendKey(0x1CF);
				wireDecodeInfo_t *decode = (wireDecodeInfo_t *)retVal;
				uninitWireDecode(decode);
				system("usbmuxd --exit");
				break;
			}
		case PLUGPLAY_CMD_INIT_DECODER:
			{
				if(retVal == NULL)
				{
					EZCASTWARN("retVal is NULL.\n");
					return -1;
				}
				wireDecodeInfo_t *decode = (wireDecodeInfo_t *)retVal;
				return initWireDecode(decode);
				break;
			}
		default:
			break;
	}
	
	return 0;
}
#endif
static int isiPhoneX()
{
	int i = 0;
	//system("ps");
	for(i=0; i<40; i++)
	{
		int ret = system("ideviceinfo -s > "IDEVICEINFO_SIMPLE_FILE_TYPE);
		EZCASTLOG("ret: %d\n", ret);
		if(ret == 0)
			break;
	}
	//system("cat "IDEVICEINFO_SIMPLE_FILE);
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

static int iphone_in(libusb_device *dev){
	EZCASTLOG("iphone in\n");
	setEzwireStatus(1, EZWIRE_DEV_IOS, EZWIRE_STATUS_CONNECTING);
	if(access("/tmp/eth_usb_stop", F_OK) == 0){
		unlink("/tmp/eth_usb_stop");
	}
	
#if (EZWIRE_CAPTURE_ENABLE != 0)
	if(isWirePlugMode() == 1)
	{
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8074)
		printf("start iphone_in display\n");
	#else
		if(getNetDisplayStatus() == 0)
		{
			//if(isiPhoneX() == 0)
			{
				setiOSInStatus(1);
				EZCASTLOG("wifi_display is not start!!!\n");
				return 0;
			}
		}
	
	#endif
		if(imirror_ValeriaCheck(dev) != 0)
		{/*
			if(isiPhoneX() == 1)
			{
				ezRemoteSendKey(0x170);
				system(AM_CASE_SC_DIR"/eth_usb.sh iphone in &");
				return 0;
			}
			*/
			imirror_ValeriaSwitch(dev, 1);
		}
		else
		{
			system("killall -9 usbmuxd");
			system("usbmuxd --enable-exit");
			imirror_ValeriaStart(PlugPlayCallback);
			ValeriaStartStatus = 1;
		}
	}
	else
#endif
	{
		system(AM_CASE_SC_DIR"/eth_usb.sh iphone in &");
	}

	return 0;
}

 int iphone_out(){
	EZCASTLOG("iphone out\n");
	setEzwireStatus(1, EZWIRE_DEV_IOS, EZWIRE_STATUS_DISCONNECT);
	system(AM_CASE_SC_DIR"/eth_usb.sh iphone out");
	EZCASTLOG("EZ_Wifidisplay socket reset\n");
	ezRemoteSendKey(0x171);
	ezFuiWifiStopPlayback();
#if (EZWIRE_CAPTURE_ENABLE != 0)
	imirror_ValeriaStop();
	ValeriaStartStatus = 0;
#endif
	ezRemoteSendKey(0x1C7);
	return 0;
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
	system(AM_CASE_SC_DIR"/usb_process.sh host_adb_start");
	adbStartFlag = 1;
	setDefaultVol();
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
#define AOA_MODEL_STRING			("Usb displayer")
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

	char *modelUrl = curl_escape(AOA_MODEL_STRING, 0);
	char *vendorUrl = curl_escape(vendor, 0);
	char *manufactureUrl = curl_escape(AOA_MANUFACTURE_STRING, 0);
	snprintf(url, size, "%s?model=%s&fw_version=%s&ota_vendor=%s&manufacture=%s&version=%s&mac_address=%s", \
					AOA_URI_STRING, modelUrl?modelUrl:"unknown", ota_get_local_version(), vendorUrl?vendorUrl:"unknown", manufactureUrl?manufactureUrl:"unknown", AOA_VERSION_STRING, id);

	if(modelUrl)
	{
		curl_free(modelUrl);
		modelUrl = NULL;
	}
	if(vendorUrl)
	{
		curl_free(vendorUrl);
		vendorUrl = NULL;
	}
	if(manufactureUrl)
	{
		curl_free(manufactureUrl);
		manufactureUrl = NULL;
	}

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


int adbNoAudioFlag = 0;
void switchToAoa(libusb_device *dev)
{
	if(adbNoAudioFlag == 1 || access(AOA_DISABLE, F_OK) == 0)
	{
		//adbNoAudioFlag = 0;
		EZCASTLOG("Android ADB\n");
		int fd = open(AUDIO_UNSUPPORT_FLAG, O_CREAT);
		if(fd >= 0)
			close(fd);
		
#if EZWIRE_TYPE == AM8252C_MIRAPLUG
		EZCASTLOG("LineIn ON...\n");
		ezFuiSetAudioLineIn(2);
#else
		if(isAM8251W() != 0)
		{
			EZCASTLOG("LineIn ON...\n");
			ezFuiSetAudioLineIn(2);
		}
#endif
		
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

void __usleep(int usec)
{
    int err;
    struct timespec req,rem;

    req.tv_sec=usec/1000000;
    req.tv_nsec=usec%1000000;

__re_sleep:
    err = nanosleep(&req,&rem);
    if(err){
         if(errno == EINTR){
              req.tv_sec=rem.tv_sec;
              req.tv_nsec=rem.tv_nsec;
              goto __re_sleep;
         }
         else{
              printf(">> %s error:%d\n",__FUNCTION__,err);
         }
    }

    return;
}

void iOSDisconnect()
{
	system(AM_CASE_SC_DIR"/usb_process.sh host_iph_out");
#if (EZWIRE_CAPTURE_ENABLE != 0)
	if(ValeriaStartStatus != 0)
	{
		EZCASTLOG("Stop iOS Direct Mirroring.\n");
		imirror_ValeriaStop();
		ValeriaStartStatus = 0;
	}
#endif
	ezRemoteSendKey(0x1C7);
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

	if(initFlag < 0)
	{
		initFlag = libusb_init(NULL);
		if (initFlag < 0)
			return initFlag;
	}

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
	EZCASTLOG("adbStartFlag: %d, adbNoAudioFlag: %d, val: %d\n", adbStartFlag, adbNoAudioFlag, val);
	
	if(adbNoAudioFlag != !!val)
	{
		adbNoAudioFlag = !!val;
		EZCASTLOG("adbNoAudioFlag: %d\n", adbNoAudioFlag);
		
		if(adbStartFlag == 0)
			return 0;
		
		if(adbNoAudioFlag == 0)
		{
			switchToAccessory();
		}
		else
		{
			set_gpio(94, 0);
			__usleep(1000000);
			set_gpio(94, 1);
		}
	}
	return 0;
}

int adbAudioInit()
{
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && ((MODULE_CONFIG_EZCASTPRO_MODE==8075)||(MODULE_CONFIG_EZCASTPRO_MODE==8074))
	adbNoAudioFlag = 0;
	if(access(AOA_DISABLE, F_OK) == 0)
		unlink(AOA_DISABLE);
#else
	if(access(AOA_DISABLE, F_OK) == 0)
		adbNoAudioFlag = 1;
	else
		adbNoAudioFlag = 0;
#endif

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
static int read_device_num()
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
int swith_usb(int num)
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
	char w_buf[256]="";
	EZCASTLOG("Android in\n");

	ret = libusb_get_active_config_descriptor(dev, &config);
	if(ret != 0){
		EZCASTWARN("Get config discriptor fail!\n");
		return ret;
	}

	EZCASTLOG("num_altsetting: %d, bNumInterfaces: %d\n", config->interface->num_altsetting, config->bNumInterfaces);
    if(config->bNumInterfaces == 1 && config->interface->num_altsetting == 1){
        if(config->interface->altsetting->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE)
        {
            dVender = -8;
            system(AM_CASE_SC_DIR"/usb_process.sh host_mass_in");
			goto __END__;
        }
        else if(config->interface->altsetting->bInterfaceClass == LIBUSB_CLASS_HID)
        {
            char app_path[50];
            dVender = -3;
            sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
            sprintf(app_path,"%s %s",app_path,"host_hid_in");
            system(app_path);
            printf("start hid functions\n");
            hid_start();
            _inform_mouse_in();
			goto __END__;
        }
    }
    /* JamesChiang, 20161007, Fix Hid device has not only one bNumInterfaces. */
    else if(config->interface->num_altsetting == 1 && config->interface->altsetting->bInterfaceClass == LIBUSB_CLASS_HID)
	{
		char app_path[50];
		dVender = -3;
		sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
		sprintf(app_path,"%s %s",app_path,"host_hid_in");
		system(app_path);
		printf("start hid functions\n");
		hid_start();
	    _inform_mouse_in();
        goto __END__;
	}
#if (EZWIRE_ENABLE	&& (EZWIRE_TYPE==AM8252B_MIRAWIRE || EZWIRE_TYPE==AM8252N_MIRAWIRE || EZWIRE_TYPE==AM8252N_MIRALINE || EZWIRE_TYPE==AM8256_MIRALINE))		// MiraWire for iOS only with 8252B do not support Android connect;
	goto __END__;
#endif
	//setEzwireStatus(1, EZWIRE_DEV_ANDROID, EZWIRE_STATUS_CONNECTING);
	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		currVender = 1;
	}else{
		currVender = desc.idVendor;
	}
	EZCASTLOG("dVender: 0x%04x\n", currVender);

	// Check ADB+Audio+accessory mode
	if((desc.idVendor == 0x18d1) && \
		((desc.idProduct == 0x2d01) || ((desc.idProduct >= 0x2d00) && (desc.idProduct <= 0x2d05))))
	{
		EZCASTLOG("Android ADB\n");
		if(desc.idProduct <= 0x2d01){
			int fd = open(AUDIO_UNSUPPORT_FLAG, O_CREAT);
			if(fd >= 0){
				close(fd);
			}
		}else{
			if(access(AUDIO_UNSUPPORT_FLAG, F_OK) == 0)
				unlink(AUDIO_UNSUPPORT_FLAG);
		}
		_inform_switch_aoa();
		system("ifconfig lo up");
		ret = system("adb devices > "ADB_DEVICEINFO_FILE);
		fd = open(ADB_DEVICEINFO_FILE, O_RDONLY);
		if(fd >= 0)
		{
			char buf[1024];
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
				EZCASTLOG("Android USB share\n");
				clean_device_data();
#if EZWIRE_TYPE == AM8252C_MIRAPLUG
				ezFuiSetAudioLineIn(1);
#else
				if(isAM8251W() != 0){
					ezFuiSetAudioLineIn(1);
				}
#endif

				system(AM_CASE_SC_DIR"/usb_process.sh host_android_in");
				dVender = currVender;
				
				setDefaultVol();
				
				EZCASTLOG("dVender: 0x%04x\n", dVender);
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
						_inform_switch_adb();
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
				EZCASTLOG("Android AOA\n");
				setAndroidInStatus(1);
				if(getNetDisplayStatus() != 0)
				{
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
				}
				else
				{
					EZCASTLOG("wifi_display is not start!!!\n");
				}								
			}
		}
	}
	#if 0
	EZCASTLOG("Android AOA\n");
	setAndroidInStatus(1);
	if(getNetDisplayStatus() != 0)
	{
		switchToAoa();
	}
	else
	{
		EZCASTLOG("wifi_display is not start!!!\n");
	}
	#endif

__END__:
	if(config != NULL)
		libusb_free_config_descriptor(config);
	return 0;
}

static int android_out(){
	EZCASTLOG("Android out\n");
	setEzwireStatus(1, EZWIRE_DEV_ANDROID, EZWIRE_STATUS_DISCONNECT);
	system(AM_CASE_SC_DIR"/eth_usb.sh android out");
	EZCASTLOG("EZ_Wifidisplay socket reset\n");
	ezFuiWifiStopPlayback();
	//ezRemoteSendKey(0x1C7);
	return 0;
}

static void hid_between_hub_in()
{
    char app_path[50];
    dVender = -3;
    sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
    sprintf(app_path,"%s %s",app_path,"host_hid_in");
    system(app_path);
    EZCASTLOG("start hid functions\n");
    hid_start();
    _inform_mouse_in();
}

int libusb_handle(){
	libusb_device **devs;
	libusb_device *dev;
	struct libusb_device_descriptor desc;
	int i = 0, r;
	ssize_t cnt;
    int change_hcd_multi_driver = 0;
	EZCASTLOG("USB handle\n");

	if(initFlag < 0)
	{
		initFlag = libusb_init(NULL);
		if (initFlag < 0)
			return initFlag;
	}

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

    struct stat s;
    EZCASTLOG("is_hub_device=%d, am7x_hcd_multi=%d \n", is_hub_device, stat("/sys/module/am7x_hcd_multi", &s));
    if( is_hub_device == 1 && stat("/sys/module/am7x_hcd_multi", &s) == 0 )
    {
        hid_between_hub_in();

    	if(devs != NULL)
		    libusb_free_device_list(devs, 1);
	    devs = NULL;
        return 0;
    }
	//print_devs(devs);
	while ((dev = devs[i++]) != NULL) {
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return r;
		}
		EZCASTLOG("%04x:%04x:%02x (bus %d, device %d)\n",
			desc.idVendor, desc.idProduct, desc.bDeviceClass,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));
		if(desc.bDeviceClass != HUB_DEV) {
			if(desc.idVendor == IPHONE_VENDOR){
				dVender = desc.idVendor;
				iphone_in(dev);
			}else
				android_in(dev);
		}

        /* JamesChiang, 20160615, touch has hub */
        if( desc.bDeviceClass == HUB_DEV && desc.idVendor != 0x1d6b && is_hub_device == 0 )
        {
            EZCASTLOG("Hub plug in !\n");
            change_hcd_multi_driver = 1;
            break;
        }
	}

	if(devs != NULL)
		libusb_free_device_list(devs, 1);
	devs = NULL;

    if( change_hcd_multi_driver == 1 )
    {
        EZCASTLOG("Hub device plug in ! \n");
        dVender = -99;
        system("rmmod am7x_hcd");
        system("insmod /lib/modules/2.6.27.29/am7x_hcd_multi.ko");
        is_hub_device = 1;
    }
    
	return 0;
}
#else

int adbAudioInit()
{
	return 0;
}

int setAdbNoAudio()
{
	return 0;
}

#endif
static int _deal_usb_msg(struct am_sys_msg *msg)
{

	char app_path[50];
	// process ralink dongle switch problem,beacause wo should block usb message to avoid load or unload wifi driver 
	Block_RalinkMessageForSwitchFunction(msg);
	printf("<%s %d>msg->subtype=====%x,msg->dataload=====%x reserved:%0x\n",__FUNCTION__,__LINE__,msg->subtype,msg->dataload,msg->reserved[0]);
	sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
	switch(msg->subtype){
		case DEVICE_USB_IN:
			printf("DEVICE_USB_IN           subtype===%d\n",msg->dataload);
			switch(msg->dataload){
				case DEVICE_RAW:	
					sprintf(app_path,"%s %s",app_path,"dev_raw_in");
					break;
				case DEVICE_MASS_STORAGE:
					if(find_iso_dmg_file()==-1){
						sprintf(app_path,"%s %s %d %d",app_path,"dev_mass_in",UDC_WORK,CDROM_CLOSE);
					}else{
						sprintf(app_path,"%s %s %d %d",app_path,"dev_mass_in",UDC_WORK,CDROM_USE);
					}
					break;
				case DEVICE_MASS_STORAGE_NEXT:
					if(find_iso_dmg_file()==-1){
						sprintf(app_path,"%s %s %d %d",app_path,"dev_mass_in",UDC_NEXT_WORK,CDROM_CLOSE);
					}else{
						sprintf(app_path,"%s %s %d %d",app_path,"dev_mass_in",UDC_NEXT_WORK,CDROM_USE);
					}
					break;
				
				case DEVICE_SUBDISPLAY:
					sprintf(app_path,"%s %s",app_path,"subdisp_in");
				case DEVICE_PICTBRIDGE:
					sprintf(app_path,"%s %s",app_path,"picbri_in");
					break;
			}
			break;
		case DEVICE_USB_OUT:
			printf("DEVICE_USB_OUT           subtype===%d\n",msg->dataload);
			switch(msg->dataload){
				case DEVICE_RAW:
					sprintf(app_path,"%s %s",app_path,"device_raw_out");
					break;
				case DEVICE_MASS_STORAGE:
					usleep(500);  //bq add to test mount failed
					sprintf(app_path,"%s %s",app_path,"dev_mass_out");
					break;
				case DEVICE_SUBDISPLAY:
					sprintf(app_path,"%s %s",app_path,"subdisp_out");
					break;
				case DEVICE_PICTBRIDGE:
					sprintf(app_path,"%s %s",app_path,"subdisp_out");
					break;
			}
			break;
		case HOST_USB_IN:
			
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
			copy_ezlauncher_json_to_html_folder();
#endif
			
#if FUN_WIRE_ENABLE

#if EZWIRE_ENABLE
#if defined(MODULE_CONFIG_USB1_DISABLE) && (MODULE_CONFIG_USB1_DISABLE==1)
			if(msg->dataload == HOST_RAW)
#else
			if(msg->dataload == HOST_RAW_NEXT)
#endif
#else
			if(msg->dataload == HOST_RAW)
#endif
			{
				libusb_handle();
				return 0;
			}
#endif

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
					usbplugin_afterup=boot_complete_status();
#endif
					break;
					
				case HOST_MASS_STOARGE_SCANOK:
					//identy_usb_storage();
					//sprintf(app_path,"%s %s",app_path,"host_mass_scanok");
					break;
			
				case HOST_USB_HID:
					sprintf(app_path,"%s %s",app_path,"host_hid_in");
					_inform_mouse_in();
					break;
				case HOST_USB_UVC:
					sprintf(app_path,"%s %s",app_path,"host_uvc_in");
					break;
				case HOST_WLAN:
					sprintf(app_path,"%s %s",app_path,"host_sta_in");
					//dongle_in(msg->reserved[0]);  //before svn8515 store vidpid in reserverd[0] 
					dongle_in(msg->reserved[1]);     //after svn8515 store vidpid in reserverd[1]
					break;
				case HOST_USB_BLUETOOTH:
					sprintf(app_path,"%s %s",app_path,"host_bt_in");
					break;
				default:
					break;
			}
			break;
		case HOST_USB_OUT:
#if FUN_WIRE_ENABLE

#if EZWIRE_ENABLE
#if defined(MODULE_CONFIG_USB1_DISABLE) && (MODULE_CONFIG_USB1_DISABLE==1)
			if(msg->dataload == HOST_RAW)
#else
			if(msg->dataload == HOST_RAW_NEXT)
#endif
#else
			if(msg->dataload == HOST_RAW)
#endif
			{
				printf("---dVender: 0x%04x\n", dVender);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
				if(dVender == 0x1DE1)
					_inform_ezlauncher_out();
#endif
				if(dVender == 0x5ac)
				{
					iphone_out();
					dVender = -1;
				}else if(dVender > 0){
					android_out();
					dVender = -1;
				}else if(dVender == -8){
					system("/am7x/case/scripts/usb_process.sh host_mass_out");
					dVender = -1;
				}else if(dVender ==-3){
					hid_stop();//bluse add to test hid functions
					sprintf(app_path,"%s %s",app_path,"host_hid_out");
					_inform_mouse_out();

                    struct stat s;
                    EZCASTLOG("is_hub_device=%d, am7x_hcd_multi=%d \n", is_hub_device, stat("/sys/module/am7x_hcd_multi", &s));
                    if( is_hub_device == 1 && stat("/sys/module/am7x_hcd_multi", &s) == 0 )
                    {
                        EZCASTLOG("Hub device plug out ! \n");
                        is_hub_device = 0;
                        system("rmmod am7x_hcd_multi");
                        system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
                    }
				}else{
					setEzwireStatus(1, EZWIRE_DEV_IGNORE, EZWIRE_STATUS_DISCONNECT);
				}
				EZCASTLOG("Android ADB stop(adbStartFlag: %d)\n", adbStartFlag);
				adbStartFlag = 0;
				_inform_adb_stop();
				system(AM_CASE_SC_DIR"/usb_process.sh host_adb_stop");
				setAndroidInStatus(0);
				setiOSInStatus(0);
				ezFuiSetAudioLineIn(0);
				if(lastVolume >= 0)
					sys_volume_ctrl(_VOLUME_CTRL_SET, &lastVolume);
				lastVolume = -1;

				return 0;
			}
#endif
				switch(msg->dataload){
				case HOST_RAW:
					if(id_hcd_ko== HCD_USE_WIFI){//modify: foe unload wifi driver
						sprintf(app_path,"%s %s",app_path,"host_raw_out_wifi");
						id_hcd_ko=HCD_WIFI_DEFAULT;
						_update_usb_status(HOST_WLAN,HOST_USB_OUT);
						special_deal_for_ez_stream();
						dongle_out();
					}else{
						sprintf(app_path,"%s %s",app_path,"host_raw_out");
						special_deal_for_ez_stream();
					}
					break;
				case HOST_RAW_NEXT:
					if(id_hcd_ko==HCD_NEXT_USE_WIFI){
						sprintf(app_path,"%s %s",app_path,"host_raw_out_next_wifi");
						
						special_deal_for_ez_stream();
						id_hcd_ko=HCD_WIFI_DEFAULT;
						_update_usb_status(HOST_WLAN,HOST_USB_OUT);
						dongle_out();
					}else{
						sprintf(app_path,"%s %s",app_path,"host_raw_out_next");
						
						special_deal_for_ez_stream();
					}
					break;
				case HOST_MASS_STORAGE:
					//printf("usb mass outup_usbcard_loopbackplay_status=%d \n",up_usbcard_loopbackplay_status);
#if  EZCAST_ENABLE

					if(up_usbcard_loopbackplay_status==1)
					{
						audioloopbackplay_stop();
						printf("usb plug out,usb music loop back stop!!!\n ");
					}
#endif
					process_usb_storage_out(msg->reserved[0]);
					if(access("/dev/sda",F_OK)!=0 && access("/dev/sdb",F_OK)!=0){
						sprintf(app_path,"%s %s",app_path,"host_mass_out");
					}
					break;
					
				case HOST_USB_HID:
					hid_stop();//bluse add to test hid functions
					sprintf(app_path,"%s %s",app_path,"host_hid_out");
					_inform_mouse_out();
					break;
				case HOST_USB_UVC:
					_UvcStop();//bluse add to uvc functions
					sprintf(app_path,"%s %s",app_path,"host_uvc_out");
					break;
				case HOST_WLAN:

					sprintf(app_path,"%s %s",app_path,"host_sta_out");
					_update_usb_status(HOST_WLAN,HOST_USB_OUT);
					dongle_out();
					break;
					
				case HOST_USB_BLUETOOTH:
					sprintf(app_path,"%s %s",app_path,"host_bt_out");
					break;
				default:
					break;
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
static int _usb_event_direction(int usb_event)
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
static int _card_event_direction(int card_event)
{
	int direct=-1;
	
	switch(card_event){
		case SD_IN:
		case MMC_IN:
		case MS_IN:
		case MS_Pro_IN:
		case XD_IN:
		case CF_IN:
			direct = IN;
		break;

		case SD_OUT:
		case MMC_OUT:
		case MS_Pro_OUT:
		case MS_OUT:
		case XD_OUT:
		case CF_OUT:
			direct = OUT;
		break;

		default:
			direct = -1;
		break;
	}

	return direct;
}

static int _do_process_card(struct am_sys_msg msg)
{
	static struct am_sys_msg cardcheckmsg;
	printf("card_stat=%d,type=0x%x,subtype=0x%x,data=0x%x,pc_con=%d\n",card_stat,msg.type,msg.subtype,msg.dataload,pc_con);
	switch(card_stat){

		case HOTPLUG_STAT_OUT:
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == IN)){
				card_stat = HOTPLUG_STAT_IN_CHECKING;
				_deal_card_msg(&msg);
				_update_card_status(&msg);
				//_backup_usb_msg(&msg);
				_inform_application();
				if(pc_con){
					card_stat = HOTPLUG_STAT_IN;
					card_block_num = get_cards_block_num(msg.subtype);
					device_info_confirm(1,msg.subtype,card_block_num);
				}
				printf("card in process\n");
			}
		break;

		case HOTPLUG_STAT_IN_CHECKING:
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == OUT)){
				//_deal_card_msg(&cardcheckmsg);
				card_stat = HOTPLUG_STAT_OUT;
				_deal_card_msg(&msg);
				_update_card_status(&msg);
				_inform_application();
				if(pc_con){
					device_info_confirm(0,msg.subtype,card_block_num);
				}
				printf("card out process: %d\n",HOTPLUG_STAT_IN_CHECKING);
			}

			if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_card_event_direction(msg.subtype) == IN)){
				//_deal_card_msg(&bak_msg);
				//_update_card_status(&bak_msg);
				//memset(&bak_msg,0,sizeof(struct am_sys_msg));
				card_stat = HOTPLUG_STAT_IN;
				printf("card in ok\n");
				
			}
		break;
		
		case HOTPLUG_STAT_IN:
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == OUT)){
				card_stat = HOTPLUG_STAT_OUT_CHECKING;
				//memcpy(&cardcheckmsg,&msg,sizeof(struct am_sys_msg));
				//first out process place, stop here to release audio_adc before sdcard out
			//	printf("sdcard storage out up_usbcard_loopbackplay_status=%d",up_usbcard_loopbackplay_status);
#if  EZCAST_ENABLE

				if(up_usbcard_loopbackplay_status==2)
				{
					audioloopbackplay_stop();
					printf("sdcard Plug out , sdcard music loopback stop!!!!\n ");
				}
#endif
				_update_card_status(&msg);
				_inform_application();
				if(pc_con){
					card_stat = HOTPLUG_STAT_OUT; ///这个地方需要改变状态，避免连接PC之后第二次插拔卡检测不到
					device_info_confirm(0,msg.subtype,card_block_num);
					card_block_num =0;
					_deal_card_msg(&msg);  //需要等到USB 释放操作之后才能去卸载卡驱动等
				}
				printf("card out process: %d\n",HOTPLUG_STAT_OUT_CHECKING);
			}
		break;

		case HOTPLUG_STAT_OUT_CHECKING:
			if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_card_event_direction(msg.subtype) == OUT)){
				card_stat = HOTPLUG_STAT_OUT;
				_deal_card_msg(&msg); //需要等到上层的动作做完之后才去卸载卡驱动等
				printf("card out ok\n");
			}
		break;

		default:
		break;
	}

	return 0;
}

static int _do_process_cf(struct am_sys_msg msg)
{
	static struct am_sys_msg cfcheckmsg;
	int cf_multisector=0;
	printf("cf_stat=%d,type=0x%x,subtype=0x%x,data=0x%x,pc_con=%d\n",cf_stat,msg.type,msg.subtype,msg.dataload,pc_con);
	switch(cf_stat){

		case HOTPLUG_STAT_OUT:
			printf("%s,%d^^^^^^^^^^^^^^^^^^^^^^^\n",__FILE__,__LINE__);
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == IN)){
				cf_stat = HOTPLUG_STAT_IN_CHECKING;
				_deal_card_msg(&msg);
				_update_card_status(&msg);
				_inform_application();
					printf("%s,%d^^^^^^^^^^^^^^^^^^^^^^^\n",__FILE__,__LINE__);
				if(pc_con){
					cf_stat = HOTPLUG_STAT_IN;
					cf_block_num = get_cards_block_num(msg.subtype);	
					device_info_confirm(1,msg.subtype,cf_block_num);
				}
				printf("cf in process\n");
			}
		break;

		case HOTPLUG_STAT_IN_CHECKING:
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == OUT)){
				_deal_card_msg(&cfcheckmsg);
				cf_stat = HOTPLUG_STAT_OUT;
				_update_card_status(&msg);
				_inform_application();
				if(pc_con)
					device_info_confirm(0,msg.subtype,cf_block_num);
				printf("cf out process: %d\n",HOTPLUG_STAT_IN_CHECKING);
			}

			if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_card_event_direction(msg.subtype) == IN)){
				cf_stat = HOTPLUG_STAT_IN;
				printf("cf plug in ok\n");
			}
		break;
		
		case HOTPLUG_STAT_IN:
			if((msg.type ==SYSMSG_CARD) && (_card_event_direction(msg.subtype) == OUT)){
				cf_stat = HOTPLUG_STAT_OUT_CHECKING;
				//memcpy(&cardcheckmsg,&msg,sizeof(struct am_sys_msg));
				_update_card_status(&msg);
				_inform_application();
				if(pc_con){
					cf_stat = HOTPLUG_STAT_OUT; ///这个地方需要改变状态，避免连接PC之后第二次插拔卡检测不到
					device_info_confirm(0,msg.subtype,cf_block_num);
					cf_block_num = 0;
					_deal_card_msg(&msg);  //需要等到USB 释放操作之后才能去卸载卡驱动等
				}
				printf("cf out process: %d\n",HOTPLUG_STAT_OUT_CHECKING);
			}
		break;

		case HOTPLUG_STAT_OUT_CHECKING:
			if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_card_event_direction(msg.subtype) == OUT)){
				cf_stat = HOTPLUG_STAT_OUT;
				_deal_card_msg(&msg);
				printf("cf out process: %d\n",HOTPLUG_STAT_OUT_CHECKING);
			}
		break;

		default:
		break;
	}
	return 0;
}

static int _do_process_usb(struct am_sys_msg msg)
{
	static struct am_sys_msg usbcheckmsg;
	printf("%s,%d:<00> type=0x%x,subtype=%x,dataload=%x,usb_stat=%d,reserved[0]=0x%x\n",__FUNCTION__,__LINE__,msg.type,msg.subtype,msg.dataload,usb_stat,
		msg.reserved[0]);
	char *device_name = NULL;
	if(msg.subtype == DEVICE_USB_VENDOR){
#if 1
		printf("%s,%d:<00> Haha SubType=DEVICE_USB_VENDOR,loaddata=%d\n",__FILE__,__LINE__,msg.dataload);
		//system("lsmod");
		if(msg.dataload ==DEVICE_MASS_2_SUBDISP){
			subdisplay_switchto_subdisplay();
		}
		else if(msg.dataload ==DEVICE_MASS_2_DEBUG){
			mass_switchto_debug();
		}
		else if(msg.dataload==DEVICE_SUBDISPLAY){
			subdisp_env_info_t env_info;
			env_info.width = IMAGE_WIDTH_E;
			env_info.height = IMAGE_HEIGHT_E;
			env_info.chipid = 0x7555;
			printf("%s,%d:<00> Subdisplay W=%d,H=%d\n",__FUNCTION__,__LINE__,env_info.width,env_info.height);
			subdisplay_work_start(&env_info);
		}
		else if(msg.dataload==DEVICE_SUBDISP_2_MASS ||msg.dataload==DEVICE_SUBDISP_QUIT){
			subdisplay_backfrom_subdispaly(msg.dataload);
		}
		else if(msg.dataload==DEVICE_DEBUG_2_MASS ||msg.dataload==DEVICE_DEBUG_QUIT){
			mass_backfrom_debug(msg.dataload);
		}
#endif
	}
	else if((msg.subtype==DEVICE_USB_IN || msg.subtype==DEVICE_USB_OUT) 
		&&(msg.dataload==DEVICE_CON_NODPDM || msg.dataload==DEVICE_POWER_ADAPTER)){
		return 0;
	}else{
		switch(usb_stat){
			case HOTPLUG_STAT_OUT:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == IN)){
					if(msg.subtype==DEVICE_USB_IN){
						usb_stat = HOTPLUG_STAT_IN_CHECKING;
						UsbPort=USB_GROUP0;
						_backup_usb_msg(&msg);
						_inform_application_device_in();
						printf("<00> usb device in\n");
					}
					else if(msg.subtype== HOST_USB_IN ){
						if(_deal_usb_msg(&msg)!=0){
							printf("<00> insmod hcd  fail\n");
							usb_stat = HOTPLUG_STAT_OUT;;
							break;
						}
						usb_stat = HOTPLUG_STAT_IN_CHECKING;
						printf("<00> usb host  in process\n");
					}
				}
				else{
					procee_normal_usbstorage(msg.reserved[0]);
					_deal_usb_msg(&msg);
					#if defined MODULE_CONFIG_EZCASTPRO_MODE && (MODULE_CONFIG_EZCASTPRO_MODE == 8075)
					Write_default_info_TOJson();
					printf("======================clear ezmusic.json file ==========================\n");
					#endif
					printf("<00> usb out  ok\n");
				}
			break;
			case HOTPLUG_STAT_IN_CHECKING:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){
					if(_deal_usb_msg(&msg)!=0)
						break;
					usb_stat = HOTPLUG_STAT_OUT;
					_inform_application();
					printf("<00> usb out process\n");
				}else if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == IN)){
	
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_MASS_STOARGE_SCANOK){
				
						device_name =get_bdev_name(msg.reserved[1]);	
						printf("%s %d device_name:%s\n",__FUNCTION__,__LINE__,device_name);
						strcpy(double_dev[0].devname,device_name);
						identy_usb_storage(1,device_name);
						double_dev[0].sector_num= get_bdev_partnums(device_name);
						printf("<%s %d> usb port0 sector_num:%d\n",__func__,__LINE__,double_dev[0].sector_num);
						strcpy(double_dev[0].hcd_name,"am7x_hcd-");
						_update_usb_storage_device_status(HOST_MASS_STORAGE,msg.subtype,1);
						identy_hcd_flag = 1;
						_inform_application();
			
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_HID){
						_deal_usb_msg(&msg);
						printf("start hid functions\n");
						hid_start();//bluse add to test hid function
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_UVC){
						_deal_usb_msg(&msg);
						sleep(1);
						 _UvcStart();
					}else {
						 if(_deal_usb_msg(&msg)){			
						printf("<00>insmod host client driver fail\n");
						break;
						}
					}
					/*simple client driver,fsm goes into STAT_IN*/
					if(msg.subtype==HOST_USB_IN&&(msg.dataload==HOST_USB_HID
							||msg.dataload==HOST_WLAN)){//for mouse or other not mass storage device
						if(msg.dataload==HOST_WLAN){///for wifi dongle
							printf("##########info ap dongle in##&&&&&&\n");
							id_hcd_ko = HCD_USE_WIFI;//for identify hcd use wifi ko or other hcd usb wifi ko 
							_update_usb_status(HOST_WLAN,msg.subtype);
							_inform_application();
						}
						usb_stat = HOTPLUG_STAT_IN;	
					}
					// For Bluetooth Dongle
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_BLUETOOTH){ 
						printf("<00> ##########info bluetooth dongle in##&&&&&&\n");
						_update_usb_status(HOST_USB_BLUETOOTH,msg.subtype);
						_inform_application(); //??? but we are NOT sure if the driver have installed OK
						usb_stat = HOTPLUG_STAT_IN;	
					}
					
				}
				else if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_usb_event_direction(msg.subtype) == IN)){
					usb_stat = HOTPLUG_STAT_IN;
					if(msg.subtype==DEVICE_USB_IN){
						if(_deal_usb_msg(&bak_msg)!=0){
							printf("<00> insmod udc fail\n");
							break;
						}				
						pc_con = 1;
						memset(&bak_msg,0,sizeof(struct am_sys_msg));
						//bq add for test cdrom auto run 1213 usb1 of 1211 usb*/
						//find_iso_dmg_file();
					}
					printf("<00> usb int OK\n");
				}
			break;		
			case HOTPLUG_STAT_IN:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){
					if(msg.subtype==DEVICE_USB_OUT){
						if(_deal_usb_msg(&msg)!=0){
							printf("<00> rm device driver fail\n");
							break;
						}
						usb_stat = HOTPLUG_STAT_OUT;
						_inform_application_device_out();
						printf("<00> usb device out \n");
					}
					else if(msg.subtype==HOST_USB_OUT){
						usb_stat = HOTPLUG_STAT_OUT_CHECKING;
						/*tell app to stop using  this device*/
						if(bad_disk){
							if(_deal_usb_msg(&msg)!=0){
								printf("<00> rmmod host client driver fail\n");
								break;
							}
							bad_disk = 0;
							break;
						}
						/*bq add to solution bluetooth ko rmmove problem*/
						if(msg.dataload==HOST_USB_BLUETOOTH){
							if(_deal_usb_msg(&msg)!=0){
								printf("rmmod bluetooth drivers failed\n");
								break;
							}
						}	

						_deal_usb_msg(&msg);
						if(msg.dataload==HOST_MASS_STORAGE)
							_update_usb_storage_device_status(msg.dataload,msg.subtype,1);//bq add for double mass storage device 
						else
							_update_usb_status(msg.dataload,msg.subtype);
							
						/*FIXME	you can use another inform key to tell fui*/
						_inform_application();
						_backup_usb_msg(&msg);
						usleep(600000);
						printf("<00> inform AP stop using  this device\n");
					}
			break;
			case HOTPLUG_STAT_OUT_CHECKING:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){				
					if(_deal_usb_msg(&msg)!=0){
						printf("<00> rmmod hcd fail\n");	
						break;
					}
					usb_stat = HOTPLUG_STAT_OUT;	
					multisector_num[0] = 0;	
					printf("<00> usb out process: rmmod hcd\n");
					if(mtp_device_in==1)
					{
						printf("realse MTP\n");
						mtp_device_in=0;
						do_usb_process("mtp_device_out");
						mtpfs_in_out_flag_change();						
						_update_usb_status(HOST_MASS_STORAGE,HOST_USB_OUT);
						_inform_application();
						
					}
				}
				else if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_usb_event_direction(msg.subtype) == OUT)){
					if(msg.subtype==HOST_USB_OUT){
						if(_deal_usb_msg(&bak_msg)!=0){
							printf("<00> rmmod host client driver fail\n");
							break;
						}
						memset(&bak_msg,0,sizeof(struct am_sys_msg));
						printf("<00> usb out process: rmmod host client driver\n");
					}
				}
			break;

			default:
			break;
			}
		}
	}
	return 0;
}

static int _do_process_usb_next(struct am_sys_msg msg)
{
	printf("%s,%d:<01> type=0x%x,subtype=%x,dataload=%x,usb_stat_next=%d,reserved[0]=0x%x\n",__FUNCTION__,__LINE__,msg.type,msg.subtype,msg.dataload,usb_stat_next,
		msg.reserved[0]);
	char *device_name = NULL;
	if(msg.subtype == DEVICE_USB_VENDOR){
#if 1
		printf("%s,%d:<01> Haha SubType=DEVICE_USB_VENDOR,loaddata=%d\n",__FUNCTION__,__LINE__,msg.dataload);
		if(msg.dataload ==DEVICE_MASS_2_SUBDISP){
			subdisplay_switchto_subdisplay();
		}
		else if(msg.dataload ==DEVICE_MASS_2_DEBUG){
			mass_switchto_debug();
		}
		else if(msg.dataload==DEVICE_SUBDISPLAY){
			subdisp_env_info_t env_info;
			env_info.width = IMAGE_WIDTH_E;
			env_info.height = IMAGE_HEIGHT_E;
			env_info.chipid = 0x7555;
			printf("%s,%d:<01> Subdisplay W=%d,H=%d\n",__FILE__,__LINE__,env_info.width,env_info.height);
			subdisplay_work_start(&env_info);
		}
		else if(msg.dataload==DEVICE_SUBDISP_2_MASS ||msg.dataload==DEVICE_SUBDISP_QUIT){
			subdisplay_backfrom_subdispaly(msg.dataload);
		}
		else if(msg.dataload==DEVICE_DEBUG_2_MASS ||msg.dataload==DEVICE_DEBUG_QUIT){
			mass_backfrom_debug(msg.dataload);
		}
#endif
	}
	else if((msg.subtype==DEVICE_USB_IN || msg.subtype==DEVICE_USB_OUT) 
		&&(msg.dataload==DEVICE_CON_NODPDM || msg.dataload==DEVICE_POWER_ADAPTER)){
		return 0;
	}else{
		switch(usb_stat_next){
			case HOTPLUG_STAT_OUT:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == IN)){
					if(msg.subtype==DEVICE_USB_IN){
						usb_stat_next = HOTPLUG_STAT_IN_CHECKING;
						UsbPort=USB_GROUP1;
						_backup_usb_msg(&msg);
						_inform_application_device_in();
						printf("<01> usb device in\n");
					}
					else if(msg.subtype== HOST_USB_IN ){
						if(_deal_usb_msg(&msg)!=0){
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
					_deal_usb_msg(&msg);
					printf("<01> usb out ok\n");
				}
			break;

			case HOTPLUG_STAT_IN_CHECKING:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){
					if(_deal_usb_msg(&msg)!=0)
						break;
					usb_stat_next = HOTPLUG_STAT_OUT;
					_inform_application();
					printf("<01> usb out process\n");
				}
				else if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == IN)){
					//printf("<01>#########################&&&&&&\n");
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_MASS_STOARGE_SCANOK){
				
						device_name =get_bdev_name(msg.reserved[1]);
						printf("%s %d <<<<<<<<<<<<<< device_name:%s\n",__FUNCTION__,__LINE__,device_name);
						identy_usb_storage(2,device_name);	
						strcpy(double_dev[1].devname,device_name);
						double_dev[1].sector_num= get_bdev_partnums(device_name);
						printf("<%s %d> usb port0 sector_num:%d\n",__func__,__LINE__,double_dev[0].sector_num);
						strcpy(double_dev[1].hcd_name,"am7x_hcd_next");
						_update_usb_storage_device_status(HOST_MASS_STORAGE,msg.subtype,2);
						identy_hcd_flag = 2;
						_inform_application();
						printf("<01> multisector_num===%d\n",multisector_num[1]);
			
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_HID){
						_deal_usb_msg(&msg);
						printf("start hid functions\n");
						hid_start();//bluse add to test hid function
					}else if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_UVC){
						_deal_usb_msg(&msg);
						sleep(1);
						 _UvcStart();
					}else {
						if(_deal_usb_msg(&msg)){
						printf("<01> insmod host client driver fail\n");
						break;
					}
					}
					/*simple client driver,fsm goes into STAT_IN*/
					if(msg.subtype==HOST_USB_IN&&(msg.dataload==HOST_USB_HID
							||msg.dataload==HOST_WLAN)){//for mouse or other not mass storage device
						if(msg.dataload==HOST_WLAN){///for wifi dongle
							printf("<01> ##########info ap dongle in##&&&&&&\n");
							id_hcd_ko = HCD_NEXT_USE_WIFI;//for 1213 usb1 hcd use wifi ko
							_update_usb_status(HOST_WLAN,msg.subtype);
							_inform_application();
						}
						usb_stat_next = HOTPLUG_STAT_IN;	
					}
					// For Bluetooth Dongle
					if(msg.subtype==HOST_USB_IN&&msg.dataload==HOST_USB_BLUETOOTH){ 
						printf("<01> ######info ap bluetooth dongle in##&&&&&&\n");
						_update_usb_status(HOST_USB_BLUETOOTH,msg.subtype);
						_inform_application(); //??? but we are NOT sure if the driver have installed OK
						usb_stat_next = HOTPLUG_STAT_IN;	
					}
					
				}
				else if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_usb_event_direction(msg.subtype) == IN)){
					usb_stat_next = HOTPLUG_STAT_IN;
					if(msg.subtype==DEVICE_USB_IN){
						if(_deal_usb_msg(&bak_msg)!=0){
							printf("<01@@>insmod udc fail\n");
							break;
						}
						pc_con = 1;
						memset(&bak_msg,0,sizeof(struct am_sys_msg));
						//bq add for cdrom auto run 1213 usb1 of 1211 usb*/
						//find_iso_dmg_file();  //if /mnt/cdrom have actions_dousb.iso file ,then start cdrom functions in scripte 
					}		
					printf("<01> usb int OK\n");
				}
			break;
			
			case HOTPLUG_STAT_IN:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){
					if(msg.subtype==DEVICE_USB_OUT){
						if(_deal_usb_msg(&msg)!=0){
							printf("<01> rm device driver fail\n");
							break;
						}
						usb_stat_next = HOTPLUG_STAT_OUT;
						_inform_application_device_out();
						printf("<01> usb device out \n");
					}
					else if(msg.subtype==HOST_USB_OUT){
						usb_stat_next = HOTPLUG_STAT_OUT_CHECKING;
						/*tell app to stop using  this device*/
						printf("%s %d bad_disk:%d dataload:%0x \n",__func__,__LINE__,bad_disk,msg.dataload);
						if(bad_disk){
							if(_deal_usb_msg(&msg)!=0){
								printf("<01> rmmod host client driver fail\n");
								break;
							}
							bad_disk = 0;
							break;
						}		
						//if(msg.dataload==HOST_MASS_STORAGE){
							_deal_usb_msg(&msg);
						//}
						if(msg.dataload==HOST_MASS_STORAGE)
							_update_usb_storage_device_status(msg.dataload,msg.subtype,2);//bq add for double mass storage device 
						else
							_update_usb_status(msg.dataload,msg.subtype);
						
						/*FIXME	you can use another inform key to tell fui*/
						_inform_application();
						_backup_usb_msg(&msg);
						//usleep(600000);
						printf("<01> inform AP stop using  this device\n");
					}				
				}
			break;

			case HOTPLUG_STAT_OUT_CHECKING:
				if((msg.type ==SYSMSG_USB) && (_usb_event_direction(msg.subtype) == OUT)){				
					if(_deal_usb_msg(&msg)!=0){
						printf("<01> rmmod hcd fail\n");	
						break;
					}
					usb_stat_next = HOTPLUG_STAT_OUT;	
					//multisector_num[1] = 0;	
					printf("<01> usb out process: rmmod hcd\n");
					if(mtp_device_in==1)
					{
						printf("release MTP \n");
						mtp_device_in=0;
						do_usb_process("mtp_device_out");
						mtpfs_in_out_flag_change();	
						_update_usb_status(HOST_MASS_STORAGE,HOST_USB_OUT);
						_inform_application();
					}
				}
				else if((msg.type ==SYSMSG_HOTPLUG_CHECK) && (_usb_event_direction(msg.subtype) == OUT)){
					if(msg.subtype==HOST_USB_OUT){
						if(_deal_usb_msg(&bak_msg)!=0){
							printf("<01> rmmod host client driver fail\n");
							break;
						}
						memset(&bak_msg,0,sizeof(struct am_sys_msg));
						printf("<01> usb out process: rmmod host client driver\n");
					}
				}
			break;

			default:
			break;
		}
	}
	return 0;
}
static void creat_mount_usb_dir()
{
	char call_buf[128];
	memset(call_buf,0x00,128);
	int i = 5;
	for(i=5;i<=6;i++)
		sprintf(call_buf,"%s mkdir -p  /mnt/usb%d\n",call_buf,i);
//	printf("debug %s %d buf:%s\n",__FUNCTION__,__LINE__,call_buf);
	system(call_buf);
}
static int _do_process_check(struct am_sys_msg msg)
{
	switch(msg.subtype){
		case SD_IN:
		case MMC_IN:
		case MS_IN:
		case MS_Pro_IN:
		case XD_IN:
		case SD_OUT:
		case MMC_OUT:
		case MS_Pro_OUT:
		case MS_OUT:
		case XD_OUT:
			_do_process_card(msg);
		break;
		
		case CF_IN:
		case CF_OUT:
			_do_process_cf(msg);
		break;

		case HOST_USB_IN:
		case DEVICE_USB_IN:
		case HOST_USB_OUT:
		case DEVICE_USB_OUT:
			//bq:add for 1213 usb message process
#if AM_CHIP_ID ==1213 ||AM_CHIP_ID==1220
			if(access("/mnt/usb5",F_OK)!=0){
				creat_mount_usb_dir();
			}
			
			if(msg.reserved[0]==0xa0)
				_do_process_usb(msg);
			if(msg.reserved[0]==0xa1)
				_do_process_usb_next(msg);
#else
			_do_process_usb(msg);
#endif
		break;
		
		default:		
		break;
	}
	return 0;
}
/**/
int id_message = 0;
int identify_message(struct am_sys_msg msg)
{
	
	struct am_sys_msg msg_lower = msg;
	printf("<%s %d> reserved=%02x\n",__FILE__,__LINE__,msg_lower.reserved[0]);
	usleep(50);
	if(msg_lower.reserved[0] == 0xa0)
		id_message= 0xa0;
	if(msg_lower.reserved[0] == 0xa1)
		id_message = 0xa1;

	return 0;
}
static int _do_process_UsbCompsiteDevice(struct am_sys_msg msg)
{
	int rtn=0;
	int compsite_info;
	compsite_info = msg.reserved[1];

	if((compsite_info&1<<2)==1){
		do{
			rtn=1;
		}while(rtn==0);
	}
	
	return rtn;
}
/*
static int do_process_uvc()
{
	char patch[60]={0x00};
	sprintf(patch,"%s/%s %s\n",AM_CASE_SC_DIR,"usb_process.sh","host_uvc_in");
	if(0 !=system(patch)){
		printf("<%s> process error:%d\n",__func__,errno);
		return -1;
	}
	sleep(1);
	printf("<%s %d>\n",__func__,__LINE__);
	return _UvcStart();
}
*/
/*process exit udisk symbol function*/
static int _process_udisk_symbol_func()
{
	char patch[60]={0x00};
	sprintf(patch,"%s/%s %s\n",AM_CASE_SC_DIR,"usb_process.sh","dev_remove_udisk");
	if(0 !=system(patch)){
		printf("<%s> process error:%d\n",__func__,errno);
		return -1;
	}
	printf("<%s %d>\n",__func__,__LINE__);
	return 0;
}

static void *hotplug_thread(void * arg)
{
	struct am_sys_msg msg;
	int rp=0;
	while(1){		
		_hotplug_sem_wait(&hotplug_work.lockget);
		msg = hotplug_work.msgfifo[hotplug_work.rp];
		if(msg.type==SYSMSG_USB||msg.subtype==HOST_USB_IN||msg.subtype==HOST_USB_OUT \
			||msg.subtype==DEVICE_USB_IN||msg.subtype==DEVICE_USB_OUT)
			identify_message(msg); //bq 
		rp = hotplug_work.rp;
		hotplug_work.rp ++;
		if(hotplug_work.rp>=MEDIA_MSG_FIFO_LEN){
			hotplug_work.rp = 0;
		}
		_hotplug_sem_post(&hotplug_work.lockput);
		printf("Get Msg rp=%d,Msgtype===0x%x,subtype=0x%x ,dataload=0x%x reserved=0x%0x\n",rp,msg.type,msg.subtype,msg.dataload,msg.reserved[0]);
		switch(msg.type){
			case SYSMSG_CARD:
				if((msg.subtype == CF_IN) || (msg.subtype == CF_OUT)){
					_do_process_cf(msg);
				}
				else{
					_do_process_card(msg);
				}
			break;

			case SYSMSG_USB:
			
				if(msg.dataload==0xc){
					_do_process_UsbCompsiteDevice(msg);
					goto flage;
				}
				else if(msg.dataload==DEVICE_R_UDISK){
					_process_udisk_symbol_func();
					goto flage;
				}/*else if(msg.dataload==0xc0){
					do_process_uvc();
					goto flage;
				}*/
			
				#if AM_CHIP_ID ==1213 ||AM_CHIP_ID==1220
				/*bq:1213 usb message process,this define two status;one is process usb0 ,
				*other is  process usb1;notice if the ic is not belong to 1213 ,_do_rocess_usb()will process it
				*/
				if(msg.reserved[0]==0xa0){
					mtp_in_usb=1;
					_do_process_usb(msg);
				}
				else if(msg.reserved[0]==0xa1){
					mtp_in_usb=2;
					_do_process_usb_next(msg);
				}
				#else
					_do_process_usb(msg);
				#endif	
			flage:
			break;

			case SYSMSG_HOTPLUG_CHECK:
				_do_process_check(msg);
			break;

			default:			
			break;
		}
	}

	pthread_exit(NULL);
}

int hotplug_open_work()
{
	int err;
//	reset_flags=0;
	hotplug_work.rp = hotplug_work.wp =0;	
	if(sem_init(&hotplug_work.lockget, 0, 0)==-1){
		printf("hotplug get lock sem init error\n");
		return -1;
	}	
	if(sem_init(&hotplug_work.lockput, 0, MEDIA_MSG_FIFO_LEN - 1)==-1){
		sem_destroy(&hotplug_work.lockget);
		printf("hotplug out fifo sem init error\n");
		return -1;
	}
	/** create hotplug thread */
	err = pthread_create(&hotplug_tid,NULL,hotplug_thread,NULL);
	if(err != 0){
		printf("hotplug thread create error\n");
		sem_destroy(&hotplug_work.lockget);
		sem_destroy(&hotplug_work.lockput);
		return -1;
	}
	/** init state machine for each media */
	card_stat = HOTPLUG_STAT_OUT;
	usb_stat = HOTPLUG_STAT_OUT;
	usb_stat_next = HOTPLUG_STAT_OUT; //bq add 1213
	cf_stat = HOTPLUG_STAT_OUT;

	card_block_num = 0;
	cf_block_num = 0;
	hotplug_work_inited = 1;
	return 0;
}

int hotplug_close_work()
{
	int err;
	if(hotplug_work_inited==0){
		printf("media hotplug work not inited\n");
		return -1;
	}	
	/** send request to cancel thread */
	err=pthread_cancel(hotplug_tid);
	
	/** wait thread until exit */
	err=pthread_join(hotplug_tid, NULL);
	
	sem_destroy(&hotplug_work.lockget);
	sem_destroy(&hotplug_work.lockput);
	hotplug_work.rp = hotplug_work.wp =0;

	card_stat = HOTPLUG_STAT_OUT;
	usb_stat = HOTPLUG_STAT_OUT;
	cf_stat = HOTPLUG_STAT_OUT;

	hotplug_work_inited=0;
	return 0;
}
int hotplug_put_msg(struct am_sys_msg msg)
{
	if(hotplug_work_inited==0){
		printf("media hotplug work not inited\n");
		return -1;
	}
	_hotplug_sem_wait(&hotplug_work.lockput);
	printf("Put Msg wp=%d,Msgtype=0x%x,subtype=0x%x,dataload=0x%x\n",hotplug_work.wp,msg.type,msg.subtype,msg.dataload);
	hotplug_work.msgfifo[hotplug_work.wp] = msg;
	hotplug_work.wp ++;
	if(hotplug_work.wp>=MEDIA_MSG_FIFO_LEN){
		hotplug_work.wp = 0;
	}
	_hotplug_sem_post(&hotplug_work.lockget);
	return 0;
}
int feedback_mtp_device_in_flag(){
	return mtp_device_in;
}


