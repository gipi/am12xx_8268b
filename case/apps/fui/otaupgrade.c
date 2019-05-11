#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <ota_updata.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <swf_types.h>
#include <stream/stream_api.h>
#include <sys_nand.h>
#include <stdbool.h>
#include <zlib.h>
#include <zconf.h>
#include <ota_fw.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "fui_common.h"
#include "display.h"
#include "apps_vram.h"
#include "ezcast_public.h"
#include "websetting.h"
extern INT32S mem_ungzip(INT8S *dest, INT32S *destLen, const INT8S *source, INT32S sourceLen);

#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"

#define ENOMEM  12
#define EAGAIN  11
#define DOWNLOAD_MODE      1    //only download firmware to reserved space,do not upgrade firmware when finish downloading firmware
#define UPGRADE_MODE       2    //only  upgrade firmware .
#define DOWN_THEN_UP_MODE  3    //download firmware first ,then automaticly upgrade firware.
#define CHECK_VERSION      4    //check the firmware version 
#define TRY_TIME           5    //retry times 
#define FW_ID_NAME    "ActionsFirmware"
#define FW_ID_SIZE    16
#define	PATH_SIZE     128
#define OTA_TIMEOUT    (5*60)  //ota download time out ,calculated in seconds
#define FIRST_ATTR "VERSION_SOFTWARE"
#define VERSION_ATTR "FIRMWARE"
#define LAST_ATTR "DDR_CAPACITY"
#define EXCEPT_ATTR "VERSION_SVN"
#define VERSION_CFG_PATH "/etc/version.conf"
#define OTA_UPGRADE_OSD_W (320)
#define OTA_UPGRADE_OSD_H	(180)
#define OTA_UPGRADE_OSD_PERENT_W		(75)
#define OTA_UPGRADE_OSD_PERENT_H		(10)
#define OTA_UPGRADE_OSD_PERENT_STR_W	(25)
#define OTA_UPGRADE_OSD_STR_SIZE		(48)	// Based on 1920*1080
#define URL_SRC         "www.iezvu.com/upgrade"
#define URL_IP          "192.155.87.52/am_ota"
#define HOSTAPCOF_PATH			"/etc/rtl_hostapd_01.conf"
#define HOSTAPCOF_VRAM_PATH		"/mnt/vram/rtl_hostapd_01.conf"
#define GETOTAURL_OTACONF		GETOTAURL_OTACONFFILE
#define GETOTAURL_URLFILE		"/tmp/ota_url"
#define GETOTAURL_URL			"https://www.iezvu.com/upgrade/ota_rx.php"
#define MEM_ZIPLEN          (5*1024*1024)       //memory space size allocated for compress file,calculated in bytes
#define MEM_UNZIPLEN        (10*1024*1024)       //memory space size allocated for uncompress file,calculated in bytes.

char* ota_get_server_version(void);

#if 0

#else

#if 1
#define OTA_Info(fmt,arg...) printf("[OTA]: line:%d "fmt"\n",__LINE__,##arg)
#define INFO(fmt,arg...) printf("[INFO] :"fmt,##arg)
#else
#define OTA_Info(fmt,arg...) do {} while (0)
#define INFO(fmt,arg...) do {} while (0)
#endif
char local_version[20] = "error";
char server_version[20] = "error";
int ota_check_flag = 0; 
static pthread_t ota_down_thread_id,ota_status_thread_id,ota_up_thread_id, ota_check_thread_id;
sem_t ota_sem;
long total_len=0,download_len=0;
struct wrap_stream_t *s_ota=NULL;
unsigned int ota_thread_status = 0,urltype=0;
char *ota_uncompress=NULL,*ota_compress=NULL,*buffer=NULL;
int scr_width = 0, src_height = 0;
int fw_flag = 0;
int proBoxUpgradeState=0;
static ota_num_t ota_num = {		// this value is the number that how many people download the firmware
	.iSet = 0,
	.num = -1,
};

static Fwu_status_t ota_status =
{
	.prg = INI_PRG,
	.state = S_INIT
};
PartUpdate_Info  PartUpdate_info[LinuxMax];
char * ota_buf=NULL;
int ota_check_version_exit=1;
int OTA_AnalysisUpdataFlag(char *flagbuf,PartUpdate_Info *cur_info)
{
	char *strP,*strCurP;
	int j;

	strP=flagbuf;
	strCurP=strchr(strP,'=');
	if( strCurP == NULL )
		return 0;

	j=strCurP-strP;
	if( j == 0 )
		return 0;
	if(j< sizeof(cur_info->partupdate_attrstr)){
		strncpy(cur_info->partupdate_attrstr,strP,j-1);//-1 becase have one ' '
		cur_info->partupdate_attrstr[j-1]=0;
	}
	else{
		strncpy(cur_info->partupdate_attrstr,strP,sizeof(cur_info->partupdate_attrstr) - 1);//-1 becase have one ' '
		cur_info->partupdate_attrstr[sizeof(cur_info->partupdate_attrstr) - 1]=0;
	}
	strP=strCurP+1;
	if((strchr(strP,'F'))||(strchr(strP,'f')))
		cur_info->partupdate_flag=0;
	else
		cur_info->partupdate_flag=1;

	return 1;
}

void OTA_AnalysisUpdataFile(void)
{
	char flagbuf[256];
	int j=0;
	char *strP,*strCurP;
	PartUpdate_Info *cur_info;

	strP=ota_buf;
	cur_info=PartUpdate_info;

	while(*strP!=0)
	{
		strCurP=strchr(strP,'\n');
		if( strCurP == NULL )
			break;
		j=strCurP-strP;
		if(j<sizeof(flagbuf)){
			strncpy(flagbuf,strP,j);
			flagbuf[j]=0;
		}
		else{
			strncpy(flagbuf,strP,sizeof(flagbuf) - 1);
			flagbuf[sizeof(flagbuf) - 1]=0;			
		}
		if(OTA_AnalysisUpdataFlag(flagbuf,cur_info))
			cur_info++;

		strP=strCurP+1;
	}
	if(*strP!=0)// no '\n'
		OTA_AnalysisUpdataFlag(strP,cur_info);

}

void ota_feed_status(int prg, Fwu_state_t state)
{
	ota_status.prg = prg;
	ota_status.state = state;
	///UP_MSG("###feed status prg: %d ,%d\n",prg,state);
}

//make the thread sleep, the unit is millseconds
void _ota_thread_sleep(unsigned int millsec)
{
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
}

void _ota_sendmsg()
{
	OTA_Info("OTA UPGRADE Send MSG");
	SWF_Message(NULL,SWF_MSG_KEY_CARDUPGRADE,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
}

static void _ota_sem_wait(sem_t * sem)
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
static void _ota_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

void  *ota_up_thread(void *arg)
{
	int fd=(int)arg,ret=0,rtn=-1,nandfd;
	char prg[10];
	int osd_width = (scr_width*OTA_UPGRADE_OSD_PERENT_W)/100;
	int osd_hight = (src_height*OTA_UPGRADE_OSD_PERENT_H)/100;
	int str_width = (scr_width*OTA_UPGRADE_OSD_PERENT_STR_W)/100;
	int str_size = (src_height*OTA_UPGRADE_OSD_STR_SIZE)/1080;
	int fill_x = (scr_width-osd_width)/2;
	int fill_y = (src_height-osd_hight)/2;
	int str_y = (src_height-osd_hight)/2 - (str_size*2);
	int percent_x = scr_width/2;
	int percent_w = str_size*3/2;
	EZCASTLOG("osd_width: %d, osd_hight: %d, str_width: %d, str_size: %d, fill_x: %d, fill_y: %d, str_y: %d, percent_x: %d\n", osd_width, osd_hight, str_width, str_size, fill_x, fill_y, str_y, percent_x);
	ret=ioctl(fd,OTA_SETPARTUPGRADE,PartUpdate_info);
	if(ret != 0){
		printf("copy upgrade configration file failed!\n");
	}
	nandfd=open("/dev/nand_block",O_RDWR);
	if(nandfd != -1){
		ioctl(nandfd,IOCTL_WRITE_DISABLE,NULL);
		close(nandfd);
	}
	ota_feed_status(INI_PRG,S_INIT);
	ioctl(fd,OTA_UPGRADE,ret);	
	if(1 == fw_flag){
		char buf[20] = "Upgrade:";
		osdengine_clear_osdrect((scr_width-str_width)/2, str_y, str_width, (str_size*2)-1);
		osdengine_show_string((scr_width-str_width)/2 , str_y, str_width, str_size, buf,2,0);
		osdengine_show_string(scr_width/2+percent_w, str_y, str_size*2, str_size, "%",2,0);
		osdengine_update_osdrect(0,0,scr_width, src_height);
	}	
	while((ota_status.state <S_FINISHED)){
		_ota_sem_wait(&ota_sem);
		ret =ioctl(fd,OTA_GETSTATUS,(Fwu_status_t *)&ota_status);
		if(1 == fw_flag){
			sprintf(prg, "%d", ota_status.prg);
			//osdengine_clear_osdrect(scr_width/2 - 160, (src_height-160)/2, 40, 30);
			osdengine_fill_osdrect(fill_x, fill_y, osd_width*ota_status.prg/100, osd_hight, 15);			
			osdengine_clear_osdrect(percent_x, str_y, percent_w, str_size);
			osdengine_show_string(percent_x, str_y, percent_w, str_size, prg,2,0);
			osdengine_update_osdrect(0,0,scr_width, src_height);
		}
		_ota_sem_post(&ota_sem);
		_ota_thread_sleep(100);
//		rtn = 0;
	}
	if(ota_status.state == S_FINISHED){		
		rtn = 0;
	}
	else{
		OTA_Info("OTA UPGRADE ERROR,please try again!\n");
	}
	ioctl(fd,OTA_EXITUP,rtn);	
	nandfd=open("/dev/nand_block",O_RDWR);
	if(nandfd != -1){
		ioctl(nandfd,IOCTL_WRITE_ENABLE,NULL);
		close(nandfd);		
	}
	if(1 == fw_flag){
		if(ota_uncompress)
			SWF_Free(ota_uncompress);
		if(ota_compress)
			SWF_Free(ota_compress);
		if(buffer)
			SWF_Free(buffer);
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);	
		ota_uncompress =NULL;
		ota_compress = NULL;
		buffer= NULL;
	}
	ota_up_thread_id =0;
	sem_destroy(&ota_sem);
	ota_thread_status=2;
	close(fd);
	pthread_exit((void*)&rtn);
	return NULL;
}

extern void *deinst;
int	ota_upgrade_routine(int fd){
	int ret=0,rtn=-1,result,cfgfd,retry,srclen,destlen,fw_offset,fw_fd;
	unsigned long  phyaddr;
	PartUpdate_Info * cur_info =PartUpdate_info;
	ota_data_t	ota_temp;
	ota_up_thread_id=0;

#if ( IS_SNOR_FLASH)
	//ota_thread_status=0;
	char callbuf[128];
	EZCASTLOG("insmod am7x_ota_upgrade.ko\n");
	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
	fd = open("/dev/otadrv",O_RDWR);
	if(fd<0){
		printf("open /dev/otadrv failed!\n");
		return -EAGAIN;
	}	
#endif

	cfgfd = open("/dev/otadrv",O_RDWR);
	if(cfgfd != -1){
		ret=ioctl(cfgfd,OTA_GETFWSTATUS,&fw_flag);
		if(0==ret){
			if(fw_flag==1){
				ret = ioctl(cfgfd,OTA_GETORIFILELENTH,&result);
				if(0 == ret){
					destlen = result;
					DE_config ds_conf;
					de_get_config(deinst,&ds_conf,DE_CFG_ALL);
					scr_width = ds_conf.input.width;
					src_height = ds_conf.input.height;
					ds_conf.input.enable=0;
					de_set_Config(deinst,&ds_conf,DE_CFG_IN);
					if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
					{
						/** first set swf to sleep, and buffer will be released */
						SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
					}	
				//	SWF_MemCheck(1);
					ota_uncompress =(char *)SWF_Malloc(destlen);					
					if(!ota_uncompress){
						OTA_Info("Malloc memory for compress file error!\n");						
						rtn = -6;
						goto MALLOC_UNZIP_ERR;
					}
					ret = ioctl(cfgfd,OTA_GETCOMPRFILELENTH,&result);
					if(0 != ret){
						rtn = -7;
						goto MALLOC_ZIP_ERR;
					}
					ota_compress = (char *)SWF_Malloc(result);
					if(!ota_compress){
						OTA_Info("Malloc memory for uncompress file error!\n");						
						rtn = -8;
						goto MALLOC_ZIP_ERR;						
					}
					char buf[25] = "prepare for upgrade ...";
					buffer = (char *)SWF_Malloc(OTA_UPGRADE_OSD_W*OTA_UPGRADE_OSD_H*2);
					if(!buffer){
						printf("[%s]%d, swf_malloc fail\n", __func__, __LINE__);
						rtn = -9;
						goto MALLOC_OSD_ERR;
					}
					int osd_width = (scr_width*OTA_UPGRADE_OSD_PERENT_W)/100;
					int osd_hight = (src_height*OTA_UPGRADE_OSD_PERENT_H)/100;
					int str_width = (scr_width*OTA_UPGRADE_OSD_PERENT_STR_W)/100;
					int str_size = (src_height*OTA_UPGRADE_OSD_STR_SIZE)/1080;
					ds_conf.input.enable = 1;
					ds_conf.input.width = OTA_UPGRADE_OSD_W;
					ds_conf.input.height = OTA_UPGRADE_OSD_H;
					_img_memset4((unsigned int*)buffer,0x80108010,ds_conf.input.width*ds_conf.input.height/2);
					ds_conf.input.img = (unsigned long*)buffer;
					ds_conf.input.bus_addr = fui_get_bus_address((unsigned long)buffer);
					de_set_Config(deinst,&ds_conf,DE_CFG_IN);					
					osdengine_init_osd(0,0,scr_width,src_height,DE_PIX_FMT_OSDBIT4MODE,"/am7x/case/data/palette.plt");
					osdengine_enable_osd();
					osdengine_fill_osdrect((scr_width-osd_width)/2, (src_height-osd_hight)/2, osd_width, osd_hight, 11);
					osdengine_show_string((scr_width-str_width)/2, (src_height-osd_hight)/2 - (str_size*2), str_width, str_size, buf,15,0);
					osdengine_update_osdrect(0,0,scr_width, src_height);
					phyaddr = fui_get_bus_address((unsigned long)ota_uncompress);
					ioctl(cfgfd,OTA_SETDESTADDR,phyaddr);
					phyaddr = fui_get_bus_address((unsigned long)ota_compress);
					ioctl(cfgfd,OTA_SETSRCADDR,phyaddr);					
					goto OTA_CONTINUE;
				}
				else{
					rtn = -4;
					goto OPEN_OTADRV_ERR;
				}
			}
			else if(fw_flag==0){
				goto OTA_CONTINUE;
			}
			else{
				OTA_Info("Ota firmware is invalid,quit upgrade process!\n");
				rtn = -3;
				goto OPEN_OTADRV_ERR;
			}
		}
		else{
			rtn = -2;
			goto OPEN_OTADRV_ERR;
		}
	}
	else{
		return -1;
	}
MALLOC_OSD_ERR:
	if(ota_compress){
		SWF_Free(ota_compress);
		ota_compress =NULL;
	}	
MALLOC_ZIP_ERR:
	if(ota_uncompress){
		SWF_Free(ota_uncompress);
		ota_uncompress =NULL;
	}
MALLOC_UNZIP_ERR:
	SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
OPEN_OTADRV_ERR:
	close(cfgfd);	
	OTA_Info("Ota error,rtn=%d!\n",rtn);
	#if EZMUSIC_ENABLE
	SysCGI_priv.otaupgrade_failed_flag=1;
	#endif
	return rtn;
OTA_CONTINUE:
	close(cfgfd);
	ret =0;
	if(access("/sys/module/am7x_battery",F_OK)==0){
		do{
			system("rmmod am7x_battery.ko");
			ret++;
		}while((access("/sys/module/am7x_battery",F_OK)==0)&&(ret<TRY_TIME));
	}
	if(ret>=TRY_TIME){
		printf("can not remove am7x-battery.ko!\n");
		goto OTA_UP_END;
	}	
//	system("lsmod -l");
	//system("killall -9 thttpd");
	//system("killall -9 udhcpd");
	//system("killall -9 wpa_supplicant");	
	//system("killall -9 hostapd");
//	system("ps");
	cfgfd=open(OTA_CFG_PATH,O_RDONLY,0644);
	if(cfgfd != -1){
		ota_buf =(char *)malloc(1024);
		if(ota_buf){
			OTA_Info("config %s exist\n",OTA_CFG_PATH);
			memset((char *)ota_buf,0,1024);
			memset((char *)PartUpdate_info,0,sizeof(PartUpdate_Info)*LinuxMax); 
			read(cfgfd,ota_buf,1024);
			OTA_AnalysisUpdataFile();		
			free(ota_buf);
		}			
		close(cfgfd);
	}
	if(sem_init(&ota_sem,0,0)==-1){
		OTA_Info("Sem init error!\n");
		goto OTA_UP_END;
	}
	ret = pthread_create(&ota_up_thread_id,NULL,ota_up_thread,(void*)fd);
	if(ret==-1){
		OTA_Info("Creat Thread Error!\n");
		sem_destroy(&ota_sem);
		close(fd);
		rtn=-10;
	}
	else{	
		ota_feed_status(INI_PRG,S_INIT);		
		OTA_Info("Create Thread OK id=%d\n",ota_up_thread_id);
		_ota_sem_post(&ota_sem);		
		ota_thread_status = 1;
		rtn= 0;
	}
OTA_UP_END:
	return rtn;
}


void *ota_status_thread(void *arg){
	int res = -1;
	ota_feed_status(INI_PRG,S_INIT);
	if(total_len>0){
		while(ota_status.state< S_FINISHED){
			if(ota_down_thread_id ==0){
				ota_status.state = S_ERROR;	
				goto OTA_UP_ERR;
			}
			ota_status.prg = (download_len/512 * 100L)/(total_len/512);
			ota_status.prg = ((ota_status.prg > 100 ) ? 100 : ota_status.prg);
			if(download_len >= total_len)
				ota_status.state = S_FINISHED;
			OTA_Info("prg=%d\n",ota_status.prg);
			_ota_thread_sleep(1000);
		}
		if(ota_status.state == S_FINISHED){			
			res=0;
		}
	}
OTA_UP_ERR:
	download_len = 0;
	pthread_exit(&res);
	return NULL;
}

void *ota_down_thread(void *arg){
	int fd =(int)arg,ret = 0;
	long file_len,download_old;
	int times,res=0,len=0,i;
	ota_data_t	ota_temp;
	fw_info_t   firminfo;
	struct timeval tm_start,tm_end; 
	char * fm_buf=NULL;

#if  (IS_SNOR_FLASH)
	//ota_thread_status=0;
	char callbuf[128];
	EZCASTLOG("insmod am7x_ota_upgrade.ko\n");
	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
	sleep(1);
	fd = open("/dev/otadrv",O_RDWR);
	if(fd<0){
		printf("open /dev/otadrv failed!\n");
		return -EAGAIN;
	}	
#endif

	fm_buf = (char *)malloc(OTA_RWSIZE*sizeof(char));
	if(!fm_buf){
		printf("Malloc buf error!\n");
		res =-1;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT;
	}
	ota_temp.buf=(char *)fm_buf;
	if(ioctl(fd,OTA_ENTER,&len)!=0){
		printf("Get reserve space size error!\n");
		res =-1;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;		
	}
	file_len = am_ota_get_filesize(s_ota);
	if((file_len<=0)){  //||(file_len%512 != 0)
		printf("Get filesize error,filesize:%ld!\n",file_len);
		res = -EAGAIN;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;	
	}
#if IS_SNOR_FLASH	//8252n snor,fw in ddr
	if(ioctl(fd,OTA_SETFWLEN,file_len))
	{
		printf("File size is too big,no enough memory to restore it!");
		res = -EAGAIN;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;	
	}	
#else
	else if(file_len/512 > len)
	{
		printf("File size is too big,no enough space to restore it! [file_len/512: %d] [len: %d]\n", file_len/512, len);
		res = -EAGAIN;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;	
	}
#endif	
	total_len=file_len;	
	ota_temp.offset = 0;
	ret = pthread_create(&ota_status_thread_id,NULL,ota_status_thread,(void *)arg);
	if(ret==0){
		OTA_Info("Create Status Thread OK id=%d\n",ota_status_thread_id);
		sleep(1);
	}
	else{
		OTA_Info("Creat Thread Error,Can't get download status!\n");
		res =-1;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;
	}	
	OTA_Info("File lenth is %ld,Downloading firmware ......\n",total_len);

	ota_feed_status(INI_PRG,S_RUNNING);	
	download_old = download_len;
	gettimeofday(&tm_start,NULL);
	while(total_len > download_len){
		len=am_ota_read(s_ota,fm_buf,OTA_RWSIZE);
		if(len>=0){	
			ota_temp.sector_num = (len/512);
			if(ota_temp.offset == 0){
				if(0!= memcmp(FW_ID_NAME,fm_buf,FW_ID_SIZE)){
					res =-1;
					ota_feed_status(INI_PRG,S_ERROR);
					goto OTA_DOWN_OUT2;					
				}		
			}
			if((0 == download_len)&&(len >=1024)){
				if(ioctl(fd,OTA_UPDATEMBR,&ota_temp)!=0){
					printf("update MBR error!\n");
					res =-1;
					ota_feed_status(INI_PRG,S_ERROR);
					goto OTA_DOWN_OUT2;		
				}							
			}
			if((download_len + len)== total_len){
				ota_temp.sector_num = (len+511)/512;
			}								
			times = 0;	
			do{
				if(ioctl(fd,OTA_DOWNLOAD_FW,&ota_temp)==0)
					break;
				else
				 	times++;
			}while(times<TRY_TIME);			
			if(times < TRY_TIME){
				ota_temp.offset += (len/512);
				download_len += 512*(ota_temp.sector_num);
				if(len%512 != 0){					
					//am_stream_seek_internal(s_ota,download_len);
					am_stream_seek_set(s_ota,download_len);
				}
			}	
			else{
				printf("ota download error_%d.len=%d\n",__LINE__,len);
				res = -1;
				ota_feed_status(ota_status.prg,S_ERROR);
				break;				
			}	
		}
		else{
			printf("ota download error_%d\n",__LINE__);
			res = -1;
			ota_feed_status(ota_status.prg,S_ERROR);
			break;
		}
		if(download_old != download_len ){
			download_old = download_len;
			gettimeofday(&tm_start,NULL);
		}else{
			gettimeofday(&tm_end,NULL);
			if(tm_end.tv_sec >(tm_start.tv_sec + OTA_TIMEOUT)){
				OTA_Info("download firmware time out!\n");	
				ota_feed_status(INI_PRG,S_ERROR);
				goto OTA_DOWN_OUT2;
			}
		}
	}
	if(total_len<=download_len){
		printf("finish download!\n");
		ota_feed_status(FINISH_PRG,S_FINISHED);
	}
OTA_DOWN_OUT2:	
	if(fm_buf);
		free(fm_buf);
OTA_DOWN_OUT:
	am_ota_close(s_ota);
	s_ota = NULL;
	ota_down_thread_id =0;
	sem_destroy(&ota_sem);
	ota_thread_status=2;
	close(fd);
	pthread_exit(&res);	
	return NULL;
}

char *trim(char * src)
{
	int len = strlen(src); 
	char *begin = src;
	char *end = NULL;
	if(len == 0 ) 
		return NULL;
	while((*begin == ' ') || (*begin == '\t') || (*begin == '\n'))
		begin++;
	if(begin - src >= len)
	{
		printf("[%s():%d]no char\n", __func__, __LINE__);
		return NULL;
	}
	end = strchr(begin, '\r');
	if(end != NULL)
		*end = '\0';
	end = strchr(begin, '\n');
	if(end != NULL)
		*end = '\0';
	end = src + len;
	//end--;
	while(*end == ' '){
		end--;
	}
	if(end <= begin)
		return NULL;
	end[1] = '\0';

	return begin;
}

int seek_pos(FILE *fp, char *attr)
{
	int ret = -1;
	char buf[256];	
	char *pos  = NULL;
	bool resert = false;
	int len = -1;
	while(1)
	{		
		memset(buf, '\0', sizeof(buf));
		if(fgets(buf, sizeof(buf), fp) != NULL) {	
			len = strlen(buf);
			if((pos = trim(buf)) == NULL)
				continue;
			if(strncmp(pos, attr, strlen(attr)) == 0) {
				fseek(fp, -len, SEEK_CUR);
				ret = 0;
				break;
			} 
		} else {					
			if(resert) // already resert, not find
				break;
			else      //seek to begin of the file
			{
				resert = true;
				fseek(fp, 0, SEEK_SET);
			}
		}
	}
	
	return ret;
}

char *take_out(char *src, char spilt, bool dirct)
{
	char *ret = NULL;
	char *pos = NULL;
	int len = 0;
	if((pos = strchr(src, spilt)) == NULL)	
		goto end;
    
	if(dirct) { //right
		 ret = trim(pos + 1);
	} else {//left
		 *pos = '\0'; 
		 ret = trim(src);

	}
end:
	return ret;
}

//compare right and left int '=',drag the ' '
int check_tag(char *local_attr, char *local_value, FILE *fp, char *server_buf)
{
	int ret = -1;
	char *server_value = NULL, *temp_value = NULL;
	//seek the server file
	if(seek_pos(fp, local_attr) != 0)//not find
		goto end;

	fgets(server_buf, 256, fp);
	if((server_value = take_out(server_buf, '=', true)) == NULL)
		goto end;
	temp_value = trim(server_value);

	if(strcmp(local_value, temp_value) != 0) {
		goto end;
	} else {
		ret = 0;
		goto end;
	}											
end:	
	free(server_value);
	return ret;
}

#if EZCAST_ENABLE
static int url_change_to_ip(char *dest, char *src){
    char *src_tmp;
    char *dest_tmp = dest;

    //memset(dest, 0, sizeof(dest));
    src_tmp = strstr(src, URL_SRC);
	if(src_tmp == NULL){
		return -1;
	}
    memcpy(dest_tmp, src, (src_tmp-src));
    dest_tmp += (src_tmp-src);
    src_tmp += sizeof(URL_SRC)-1;
    sprintf(dest_tmp, "%s", URL_IP);
    dest_tmp += sizeof(URL_IP)-1;
    memcpy(dest_tmp, src_tmp, strlen(src_tmp));
    printf("\tsrc: %s\n\tdest: %s\n", src, dest);

    return 0;
}
#endif

static char *attr[] = {"VERSION_SOFTWARE", "VERSION_HARDWARE",
                   "VERSION_PROCESSOR", "VERSION_VENDOR", "VERSION_PRODUCT",
                    "FIRMWARE","DDR_TYPE", "DDR_CAPACITY"};

static int conf_download(char *cfgfilepath){
	struct wrap_stream_t *s_cfg=NULL;
	char *pch=NULL;
	int ret, cfgfd,rtn=0;
	
	s_cfg=am_ota_open(cfgfilepath,NULL,&rtn);
	if(s_cfg){
		ret = am_ota_get_filesize(s_cfg);	
		if((ret>0)&&(ret <= OTA_RWSIZE)){
			pch=(char *)malloc(OTA_RWSIZE * sizeof(char));
			if(pch){
				ret=am_ota_read(s_cfg,pch,OTA_RWSIZE);
				if(ret>0){	
					cfgfd=open(OTA_CFG_PATH,O_CREAT|O_RDWR|O_TRUNC,10644);
					if(-1 != cfgfd){
						write(cfgfd,pch,ret);
						close(cfgfd);
					}
					else{
						printf("download config file failed!\n");
						goto end;
					}
				} else {
					goto end;
				}
				free(pch);
			} else {
				goto end;
			}
		} else {
			goto end;
		}
		am_ota_close(s_cfg);
		printf("finish downloading config file!\n");
		return 0;
	}else{
		return 1;
	}

end:
	if(pch)	free(pch);
	if(s_cfg)	am_ota_close(s_cfg);
	return -1;
}

//-1 error 0 the local version is the newest >0 the server version
void *ota_check_version(void *surl)
{	
	char *url = (char *)surl;
	int ret;
	char  cfgfilepath[PATH_SIZE],*pch=NULL;
	char local_buf[256];
	char server_buf[256];
	FILE *local_fp = NULL, *server_fp = NULL;
//download the .conf
#if EZCAST_ENABLE
	ota_check_flag = 0;
	unlink(OTA_CFG_PATH);
	sprintf(server_version, "error");
	char ota_url[1024];
	memset(ota_url, 0, sizeof(ota_url));
	ret = ota_get_url(GETOTAURL_OTACONF, ota_url, NULL, sizeof(ota_url)-1);
	if(ret < 0){
		EZCASTLOG("Get OTA config file url error!!!\n");
		goto end;
	}
	EZCASTLOG("Config url:%s\n", ota_url);
	url = ota_url;
#endif
	memset(cfgfilepath,0,PATH_SIZE);
	pch = strrchr(url, '.');
	if(pch){
		strncpy(cfgfilepath,url,pch-url);
		strcat(cfgfilepath,".conf");
		printf("config file path is %s\n",cfgfilepath);	
		if(conf_download(cfgfilepath) == 1){
#if EZCAST_ENABLE
			EZCASTLOG("Download config file error!!!\n");
			goto end;
#endif
		}
		EZCASTLOG("server config: \n");
		system("cat /tmp/ota_partupgrade.conf | grep vram");
		EZCASTLOG("local config: \n");
		system("cat /etc/version.conf | grep vram");
	} else {
		goto end;
	}

    local_fp = fopen(VERSION_CFG_PATH, "r");    
    if(local_fp == NULL) {      
        printf("%s not exist\n",VERSION_CFG_PATH);              
          goto end;   
    }       
    server_fp = fopen(OTA_CFG_PATH, "r");   
    if(server_fp == NULL) {     
        printf("%s not exist\n",OTA_CFG_PATH);              
        goto end;   
    }
    int i = 0;
    char *local_value = NULL, *server_value = NULL;

    for(i=0; i<sizeof(attr)/sizeof(attr[0]); i++)
    {
       
        memset(local_buf, 0, sizeof(local_buf));
        memset(server_buf, 0, sizeof(server_buf));
        if((seek_pos(local_fp, attr[i]) != 0) || (seek_pos(server_fp, attr[i]) != 0)) 
            goto end;  
   
        if(fgets(local_buf, sizeof(local_buf), local_fp) == NULL || fgets(server_buf, sizeof(server_buf), server_fp) == NULL)        
            goto end;
            
        if((local_value = take_out(local_buf, '=', true)) == NULL || 
           (server_value = take_out(server_buf, '=', true)) == NULL)
            goto end;   


        if(strcmp(attr[i], "FIRMWARE") == 0) {// version attr
            char *temp1 = NULL, *temp1_save = NULL, *temp2 = NULL, *temp2_save = NULL;
            int local_values[4];
            int server_values[4];
            temp1 = strchr(local_value, '.');
            temp2 = strchr(server_value, '.');
            if(temp1 != NULL && temp2 != NULL) {// XX.XX.XXX.XX
                int j = 0;
                temp1 = strtok_r(local_value, ".", &temp1_save);
                temp2 = strtok_r(server_value, ".", &temp2_save);
                local_values[0] = atoi(temp1);
                server_values[0] = atoi(temp2);
                if(local_values[0] < 0 || server_values[0] < 0)
                    goto end;
                while((temp1 = strtok_r(NULL, ".", &temp1_save)) != NULL \
                          && (temp2 = strtok_r(NULL, ".", &temp2_save)) != NULL)
                {

                      if(++j >= sizeof(local_values))
                          goto end;
                      local_values[j] = atoi(temp1);
                      server_values[j] = atoi(temp2);                
                
                }
                if(j != sizeof(local_values)/sizeof(local_values[0]) - 1)
                   goto end;
              
                for(j=0; j<sizeof(local_values)/sizeof(local_values[0]); j++)
                {
                   if(local_values[j] <= server_values[j]) {
                       if(local_values[j] < server_values[j]) {
                          memset(server_version, '\0', sizeof(server_version)); 
                          sprintf(server_version, "%d.%d.%d.%d", server_values[0], server_values[1], server_values[2], server_values[3]);
                          break;
                       } else {
                           if(j == sizeof(local_values)/sizeof(local_values[0])-1) {
                               memset(server_version, '\0', sizeof(server_version));
                               memcpy(server_version, "newest", strlen("newest"));    
                               
                           }
                               
                        }
                    } else
                        goto end;

                  }
          
              } else if(temp1 == NULL && temp2 == NULL){ //XXXX a int
                  int server_t = atoi(server_value);
                  int local_t = atoi(local_value);
                  if(local_t <= server_t) {
                      memset(server_version, '\0', sizeof(server_version));
                      if(local_t < server_t) 
                          memcpy(server_version, server_value, strlen(server_value));
                     else if(server_t == local_t) 
                          memcpy(server_version, "newest", strlen("newest"));      
                   }
              } else //version format is error
                  goto end;
    
      } else { // other attr
            printf("local_value=%s\tserver_value=%s\n", local_value, server_value);
            if(strcmp(local_value, server_value) != 0) {
				printf("[%s][%d] -- strlen(local_value): %d[%d], strlen(server_value): %d[%d]\n", __func__, __LINE__, strlen(local_value), (int)local_value[strlen(local_value)-1], strlen(server_value), (int)server_value[strlen(server_value)-1]);
                goto end;
            }
            printf("=========strcmp end\n");
        }
        
    }

end:
	ota_check_flag = time(NULL);
	ota_check_version_exit=1;
	return (void *)0;	
}

static int sendOtaDeviceInfo()
{
	char url[512];
	
	getOtaDeviceInfoUrl(url, sizeof(url), ota_get_server_version());

	return upload_and_return(NULL, url, "/tmp/OTADeviceInfoReturn");
}

int ota_download_routine(int fd,char *url)
{
	int ret,rtn=0,i=0;
	char *pch,filetype[8],url2[256];
	ota_down_thread_id=0;
	if(sem_init(&ota_sem,0,0)==-1){
		OTA_Info("Creat Thread Error!");
		rtn =-1;
		goto OTA_WORK_END;
	}
	printf("url is:%s\n",url);
	pch = strrchr(url, '.');
	if(!pch){
		rtn =-1;
		sem_destroy(&ota_sem);
		goto OTA_WORK_END;	
	}
	strncpy(url2,url,pch-url);
	pch++;
	while(*pch != '\0'){
		filetype[i] = *pch;
		pch++;
		i++;
	}

	filetype[i]='\0';
	printf("filetype is :%s\n",filetype);
	if(i==0){
		rtn =-1;
		sem_destroy(&ota_sem);
		goto OTA_WORK_END;	
	}
	else{
		if(0x00 ==memcmp(filetype,"bin",3)){
			urltype = 0;
			strcat(url2,".gz");
		}
		else if(0x00 ==memcmp(filetype,"gz",3)){
			urltype = 1;
			strcat(url2,".bin");
		}
		else{
			rtn =-1;
			sem_destroy(&ota_sem);
			goto OTA_WORK_END;				
		}
	}
	s_ota=am_ota_open(url,NULL,&rtn);//(int *)&len
	if(NULL==s_ota){
		if(1 == urltype){
			s_ota=am_ota_open(url2,NULL,&rtn);
			if(NULL != s_ota){
				goto OTA_DOWNLOAD;
			}
		}		
		OTA_Info("Can't get sream!\n");
		sem_destroy(&ota_sem);
		rtn=-1;
		goto OTA_WORK_END;
	}
OTA_DOWNLOAD:	
	ret = pthread_create(&ota_down_thread_id,NULL,ota_down_thread,(void*)fd);
	if(ret==0){
		ota_feed_status(INI_PRG,S_INIT);
		OTA_Info("Create Thread OK id=%d",ota_down_thread_id);
		_ota_sem_post(&ota_sem);		
		ota_thread_status=1;
		download_len = 0;
//		pthread_mutex_destroy(&ota_mutex);
		sendOtaDeviceInfo();

		ret = 0;
	}
	else{
		OTA_Info("Creat Thread Error!");
		sem_destroy(&ota_sem);
		close(fd);
		rtn= -1;
	}
OTA_WORK_END:
	return rtn;
}

/**
@brief get the status of upgrading 
@param[in]	: none
@return the status which the lower 8 bits is the processing and the next 8 bits is the state
**/
int ota_get_status()
{
	int rtn=0;
	if(ota_thread_status>0){
		_ota_sem_wait(&ota_sem);
		rtn = rtn |ota_status.state;
		rtn = rtn <<8;
		rtn = rtn |ota_status.prg;
		//OTA_Info("getstatus:%x ", rtn);
		_ota_sem_post(&ota_sem);
		#if EZMUSIC_ENABLE
		//printf("ota_status.prg===============%d\n",ota_status.prg);
		if((0<ota_status.prg)&&(ota_status.prg<100))
		{
			if(1==SysCGI_priv.play_otaupgrade_waiting_end)
			{
				SysCGI_priv.otaupgrade_waiting_flag=1;
				SysCGI_priv.play_otaupgrade_waiting_end=0;
			}			
		}
		else if(90>ota_status.prg)
			SysCGI_priv.otaupgrade_download_finished++;
		//printf("SysCGI_priv.otaupgrade_download_finished===========%d\n",SysCGI_priv.otaupgrade_download_finished);
		if(2==SysCGI_priv.otaupgrade_download_finished)
			 SysCGI_priv.otaupgrade_successful_flag=1;
		#endif
	}
	else{		
		rtn = 0;
		//OTA_Info("getstatus:%x ", rtn);	
	}
	return rtn;
}
int ota_get_check_status()
{
	int ret = 0;
	int curr_t = time(NULL);
	if(ota_check_flag > 10) {//check finish
		//ota_check_flag = 0;
		ret = 1;
	}

	return ret;
}

char * ota_get_local_version()
{
	FILE *fp = NULL;
	char buf[256];
	char *temp_value = NULL;
    
	fp = fopen(VERSION_CFG_PATH, "r");	
	if(fp == NULL) {		
		printf("%s not exist\n",VERSION_CFG_PATH);				
		goto end;	
	}
	if((seek_pos(fp, "FIRMWARE") != 0)) {
		goto end;
	}
	if(fgets(buf, sizeof(buf), fp) != NULL) {
		if((temp_value = take_out(buf, '=', true)) == NULL)
			goto end;	
        memset(local_version, '\0', sizeof(local_version));
        memcpy(local_version, temp_value, strlen(temp_value));
	} else {
		goto end;
	}
	
end:	

	if(fp != NULL)
		fclose(fp);
	return local_version;
}

char* ota_get_server_version(void)
{
	return server_version;
}

#if EZCAST_ENABLE	// For get OTA url from server
size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)   

{   

       FILE *f = stream;   

       size_t n;   


       if (ferror(f))   

              return CURL_READFUNC_ABORT;   


       n = fread(ptr, size, nmemb, f) * size;   


       return n;   

}

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

int upload_and_return(char *path, char *surl, char *return_path){
	FILE *fp = NULL, *fp_tmp = NULL;
	CURL *curl;
	struct stat jstat;
	int ret = -1;

	curl_global_init(CURL_GLOBAL_ALL);

	if((curl = curl_easy_init()) != NULL){
		if(path)
		{
			if(access(path, 0)){
				printf("\t%s is not exist!!\n", path);
				ota_conf_init();
				if(access(path, F_OK))
				{
					EZCASTLOG("Can not find file: %s\n", path);
					ret = -1;
					goto __END__;
				}
			}
			
			fp = fopen(path, "rb");
			if(fp == NULL){
				perror("Open json");
				ret = -1;
				goto __END__;
			}
			
			if(stat(path, &jstat) < 0){
				perror("get file size");
				ret = -1;
				goto __END__;
			}
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE, jstat.st_size);
			curl_easy_setopt(curl, CURLOPT_PUT, 1L);
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_READDATA, fp);
		}else{
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
			curl_easy_setopt(curl, CURLOPT_PUT, 0L);
		}
		
		fp_tmp = fopen(return_path, "w+");
		if(fp_tmp == NULL){
			perror("Open GETOTAURL_URLFILE");
			ret = -1;
			goto __END__;
		}

		curl_easy_setopt(curl, CURLOPT_URL, surl);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp_tmp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		if(curl_easy_perform(curl) != CURLE_OK){
			perror("Upload OTACONF");
			ret = -1;
			goto __END__;
		}

		ret = 0;
	} 

__END__:
	if(curl)
		curl_easy_cleanup(curl);
		
	curl_global_cleanup();

	if(fp)
		fclose(fp);

	if(fp_tmp)
		fclose(fp_tmp);
	
	return ret;
}

static int ota_enforce = 0;
int ota_get_url(char *path, char *url_cfg, char *url_fw, int len){
	char *otaServer = GETOTAURL_URL;

#if OTA_SERVER_SET_ENABLE
	char otaServerTmp[128];
	int ret = ezCastGetStrConfig(CONFNAME_OTASERVER, otaServerTmp, sizeof(otaServerTmp));
	if(ret == 0 && strcmp(otaServerTmp, STR_NIL) != 0 && strlen(otaServerTmp) != 0){
		otaServer = otaServerTmp;
	}
#endif

	printf("[%s][%d] -- otaServer: %s\n", __func__, __LINE__, otaServer);
	if(upload_and_return(path, otaServer, GETOTAURL_URLFILE) < 0){
		printf("get ota url failed!!!\n");
		return -1;
	}
	ota_read_url(GETOTAURL_URLFILE, url_cfg, url_fw, &ota_enforce, len);
	if(url_cfg != NULL){
		printf("[%s][%d] -- url_cfg: %s\n", __func__, __LINE__, url_cfg);
	}
	
	if(url_fw != NULL){
		printf("[%s][%d] -- url_fw: %s\n", __func__, __LINE__, url_fw);
	}
	return 0;
}

int get_ota_enforce(){
	return ota_enforce;
}

int get_ota_download_number(){
	int num = -1;
	if(ota_num.iSet)
		num = ota_num.num;
	else
		num = -1;
	ota_num.iSet = 0;
	return num;
}

int ota_get_download_num(){
	int num = -1;
	if(create_get_ota_num_conf(server_version) < 0){
		printf("[%s][%d] -- Get OTA download number error!!!\n", __func__, __LINE__);
		return -1;
	}
	if(upload_and_return(GET_OTA_DOWNLOAD_NUM_CONF, GET_OTA_DOWNLOAD_NUM_URL, GET_OTA_DOWNLOAD_NUM_RETURN) < 0){
		printf("get ota url failed!!!\n");
		return -1;
	}
	if((num = json_get_ota_num()) > 0){
		ota_num.num = num;
		ota_num.iSet = 1;
		printf("[%s][%d] -- ota_num.num: %d\n", __func__, __LINE__, ota_num.num);
	}

	return 0;
}
int proBoxUpgradeEnv()//for ensure  probox 5g_config
{
	//printf("[%s][%d]proBoxUpgradeState=%d\n",__FILE__,__LINE__,proBoxUpgradeState);
	return proBoxUpgradeState;
}
static void ezcastUpgradePrepare(){
	struct sysconf_param sys_info;	
#if !EZWIRE_ENABLE
	if(get_last_ui() == 2){		// The default mode is EZMirror First
		set_last_ui(0, 0);		// Set to default value
	}
#endif

#if !EZWIRE_ENABLE
	// Write ssid and password to vram
	_get_env_data(&sys_info);

#if (defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075))
	proBoxUpgradeState=1;//for ensure  probox 5g_config is rtl_hostapd_01
#endif
	memset(sys_info.softap_info.softap_ssid, 0, sizeof(sys_info.softap_info.softap_ssid));
	memset(sys_info.softap_info.softap_psk, 0, sizeof(sys_info.softap_info.softap_psk));
	get_softap_ssid(sys_info.softap_info.softap_ssid, sizeof(sys_info.softap_info.softap_ssid));
	get_softap_psk(sys_info.softap_info.softap_psk, sizeof(sys_info.softap_info.softap_psk));
#if (defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075))
	proBoxUpgradeState=0;
#endif
	sys_info.softap_info.softap_psk_setted_flag=1;
	printf("[%s] [%d] -- firmware sys_info.softap_info.softap_psk:%s\n", __func__, __LINE__, sys_info.softap_info.softap_psk);
	_save_env_data(&sys_info);
	_store_env_data();
#endif
	//ezCastSetAutoplayVal(SET_AUTOPLAY_ENABLE, 1);
	//ezCastSetAutoplayVal(SET_AUTOPLAY_WAITIME, 30);
	#if !(defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075))
	ezCastCleanAutoplaySetting();
	#endif
	sync();
	
	disable_set_json_value();
}

int manual_ota(char *url){
	int ret=-1,fd=-1;
	char callbuf[128];
	
	if(url == NULL){
		EZCASTWARN("url is NULL!!!");
		return -1;
	}
	
	ezcastUpgradePrepare();
	sleep(1);

	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
#if (IS_SNOR_FLASH)
	EZCASTLOG("It's snor flash, and open ota device file in download function.\n");
#else
	fd=open("/dev/otadrv",O_RDWR);
	if(fd<0){
		EZCASTLOG("Open /dev/otadrv failed!\n");
		perror("error");
		return -EAGAIN;
	}
#endif

	if(access("/etc/version.conf", F_OK) == 0)
	{
		snprintf(callbuf, sizeof(callbuf), "cp -dprf %s %s", VERSION_CFG_PATH, OTA_CFG_PATH);
		system(callbuf);
	}
	
	ret=ota_download_routine(fd,url);
	if(ret < 0){
		EZCASTWARN("Create ota download thread fail!!!\n");
		return -1;
	}

#if 0	
	do{
		sleep(1);
		EZCASTLOG("Download: %d\%\n", ota_status.prg);
	}while(ota_status.state!=S_FINISHED&&ota_status.state!=S_ERROR);
	if(ota_status.state==S_FINISHED){
		wifi_subdisplay_end();
		wifi_stop_process();		//stop the wifi related processes
		wifi_close_fun();
		process_wifi_function_switch(3);
		sleep(1);
		
#if 1
		if(get_last_ui() == 2){ 	// The default mode is EZMirror First
			set_last_ui(0, 0);		// Set to default value
		}
#endif
		ezCastCleanAutoplaySetting();
		sync();
		disable_set_json_value();
		//ezcastUpgradePrepare();
		if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
			sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
			system(callbuf);
		}
		fd=open("/dev/otadrv",O_RDWR);
		if(fd<0){
			printf("open /dev/otadrv failed!\n");
			return -EAGAIN;
		}	
		ret=ota_upgrade_routine(fd);
	}else{
		EZCASTLOG("Download firmware fail!!!\n");
		ret = -1;
	}
#endif
	return ret;
}
	
#endif

int _ota_entry_main(int mode,char *url)
{
	int ret=0,fd = -1;
	char callbuf[128];
#if EZCAST_ENABLE
	char *pos = NULL;
	FILE *fp = NULL;
	char buf[128];
	char backupbuf[128];
	char cmd[128];
	char ota_url[1024];
#endif

#if (IS_SNOR_FLASH)
	EZCASTLOG("If it's snor flash, insmod am7x_ota_upgrade.ko while upgrade.\n");
#else
	//ota_thread_status=0;
	EZCASTLOG("insmod am7x_ota_upgrade.ko\n");
	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
	fd=open("/dev/otadrv",O_RDWR);
	if(fd<0){
		printf("open /dev/otadrv failed!\n");
		return -EAGAIN;
	}	
#endif
EZCASTLOG("luoyuping test mode=%d.\n",mode);
#if EZCAST_ENABLE
	switch(mode){
		case CHECK_VERSION :  			
			memset(ota_url, 0, sizeof(ota_url));
			if(ota_check_version_exit)
			{
				ota_check_version_exit=0;
				ret = pthread_create(&ota_check_thread_id,NULL,ota_check_version,(void*)ota_url);
				if(ret==-1){
					OTA_Info("Creat Thread Error!");
					ret = -1;
				}				
			}
			break;
		case DOWNLOAD_MODE: 
			if(access(OTA_CFG_PATH, F_OK) != 0){
				EZCASTLOG("OTA config file in server is not download!!!\n");
				snprintf(cmd, sizeof(cmd), "cp /etc/version.conf %s", OTA_CFG_PATH);
				system(cmd);
			}
			
			memset(ota_url, 0, sizeof(ota_url));
			ret = ota_get_url(GETOTAURL_OTACONF, NULL, ota_url, sizeof(ota_url)-1);
			if(ret < 0){
				EZCASTLOG("Get OTA download url error!!!\n");
				break;
			}
			ota_get_download_num();

			ret=ota_download_routine(fd,ota_url);
			if(ret==-1){
				memset(backupbuf, 0, sizeof(backupbuf));
				ret = url_change_to_ip(backupbuf, ota_url);
				if(ret < 0){
					printf("\tsurl error: %s\n", ota_url);
					break;
				}
				ret=ota_download_routine(fd,backupbuf);
			}
			break;
		case UPGRADE_MODE: 
#if !(IS_SNOR_FLASH)
			ezcastUpgradePrepare();
			sleep(1);
#endif
			ret=ota_upgrade_routine(fd);
			break;
		case DOWN_THEN_UP_MODE:
			memset(ota_url, 0, sizeof(ota_url));
			ret = ota_get_url(GETOTAURL_OTACONF, NULL, ota_url, sizeof(ota_url)-1);
			if(ret < 0){
				EZCASTLOG("Get OTA download url error!!!\n");
				break;
			}
			printf("[%s] [%d] -- firmware url:%s\n", __func__, __LINE__, ota_url);
			if(0==ota_download_routine(fd,ota_url))
				ota_upgrade_routine(fd);
			else
				printf("download firmware error ,failed to upgrade!\n");
			break;
		default:
			break;
	}
#else
	switch(mode){
		case CHECK_VERSION :  
			if(ota_check_version_exit)
			{
				ota_check_version_exit=0;
				ret = pthread_create(&ota_check_thread_id,NULL,ota_check_version,(void*)url);
								  if(ret==-1){
										OTA_Info("Creat Thread Error!");
										ret = -1;
									}				
			}
			break;
		case DOWNLOAD_MODE: ret=ota_download_routine(fd,url);
			break;
		case UPGRADE_MODE: ret=ota_upgrade_routine(fd);
			break;
		case DOWN_THEN_UP_MODE:if(0==ota_download_routine(fd,url))
									ota_upgrade_routine(fd);
								else
									printf("download firmware error ,failed to upgrade!\n");
			break;
		default:
			break;
	}
#endif

	return ret;
}
#endif
