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
#include <stream/stream_api.h>
#include <sys_nand.h>
#include <stdbool.h>
#include <zlib.h>
#include <zconf.h>
#include <ota_fw.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <wire_heap.h>

#include "wire_ota.h"
#include "wire_log.h"
#include "wire_ui.h"
#include "wire_osd.h"
#include "wire_config.h"
static pthread_t ota_down_thread_id,ota_status_thread_id,ota_up_thread_id;
sem_t ota_sem;
unsigned int ota_thread_status = 0,urltype=0;
struct wrap_stream_t *s_ota = NULL;
long total_len=0,download_len=0;
PartUpdate_Info  PartUpdate_info[LinuxMax];
int fw_flag = 0;
void *pHeap = NULL;
char *ota_uncompress=NULL,*ota_compress=NULL,*buffer=NULL;
char *ota_buf=NULL;
int src_width,src_height;

static Fwu_status_t ota_status =
{
	.prg = INI_PRG,
	.state = S_INIT
};

#define OTA_Info(fmt,arg...) printf("[OTA]: line:%d "fmt"\n",__LINE__,##arg)
#define INFO(fmt,arg...) printf("[INFO] :"fmt,##arg)
#define FW_ID_NAME    "ActionsFirmware"
#define FW_ID_SIZE    16
#define TRY_TIME           5    //retry times 
#define OTA_TIMEOUT    (5*60)  //ota download time out ,calculated in seconds
#define OTA_UPGRADE_OSD_W (320)
#define OTA_UPGRADE_OSD_H	(180)
#define OTA_UPGRADE_OSD_PERENT_W		(75)
#define OTA_UPGRADE_OSD_PERENT_H		(10)
#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"
#define P_HEAP_OTA_SIZE 10*1024*1024

static void *mallocOtaHeap(int size)
{
	void *heap = NULL;

	heap = (void *)wire_MemoryInit(size);
	OSprintf("after sysbuf_heap_ota_init\n");
	if(!heap){
		WLOGE("get heap_ota failed\n");
		return NULL;
	}
	return heap;
}

static void freeOtaHeap(void *heap)
{
	if(heap){
		wire_MemoryRelease(heap);
		heap = NULL;
	}
}

void ota_feed_status(int prg, Fwu_state_t state)
{
	ota_status.prg = prg;
	ota_status.state = state;
	///UP_MSG("###feed status prg: %d ,%d\n",prg,state);
}

void _ota_thread_sleep(unsigned int millsec)
{
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
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

static void _img_memset4(unsigned int * buffer,unsigned int color, int count){
 	int i; 
	if(((int)buffer&0x3)) 
		printf("memset4 error %x %x\n",buffer); 
	for(i=0;i<count;i++) 
		*(buffer+i)=color; 
}

void *ota_status_thread(void *arg)
{
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

void *ota_down_thread(void *arg)
{
	int fd =(int)arg,ret = 0;
	long file_len,download_old;
	int times,res=0,len=0,i;
	ota_data_t	ota_temp;
	fw_info_t   firminfo;
	struct timeval tm_start,tm_end; 
	char * fm_buf=NULL;
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

	if(ioctl(fd,OTA_SETFWLEN,file_len))
	{
		printf("File size is too big,no enough memory to restore it!");
		res = -EAGAIN;
		ota_feed_status(INI_PRG,S_ERROR);
		goto OTA_DOWN_OUT2;	
	}	

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
					//am_stream_seek_set(s_ota,download_len);
					;
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

void  *ota_up_thread(void *arg)
{
	int fd=(int)arg,ret=0,rtn=-1,nandfd;
	char prg[10];
	int osd_width = (src_width*OTA_UPGRADE_OSD_PERENT_W)/100;
	int osd_hight = (src_height*OTA_UPGRADE_OSD_PERENT_H)/100;
	int fill_x = (src_width-osd_width)/2;
	int fill_y = (src_height-osd_hight)/2;
	int percent_x = src_width/2;

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
		//char buf[20] = "Upgrade:";
		//osdengine_show_string((src_width-str_width)/2 , str_y, str_width, str_size, buf,2,0);
		//osdengine_show_string(src_width/2+percent_w, str_y, str_size*2, str_size, "%",2,0);
		osdengine_update_osdrect(0,0,src_width, src_height);
	}	
	while((ota_status.state <S_FINISHED)){
		_ota_sem_wait(&ota_sem);
		ret =ioctl(fd,OTA_GETSTATUS,(Fwu_status_t *)&ota_status);
		if(1 == fw_flag){
			sprintf(prg, "%d", ota_status.prg);
			//osdengine_clear_osdrect(src_width/2 - 160, (src_height-160)/2, 40, 30);
			osdengine_fill_osdrect(fill_x, fill_y, osd_width*ota_status.prg/100, osd_hight, 15);			
			osdengine_update_osdrect(0,0,src_width, src_height);
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
		if(ota_compress){
			OSHfree(pHeap, ota_compress);
			ota_compress = NULL;
		}
		if(ota_uncompress){
			OSHfree(pHeap, ota_uncompress);
			ota_uncompress = NULL;
		}
		if(buffer){
			OSHfree(pHeap, buffer);
			buffer = NULL;
		}
		if(pHeap){
			freeOtaHeap(pHeap);
			pHeap = NULL;
		}
	}
	ota_up_thread_id =0;
	sem_destroy(&ota_sem);
	ota_thread_status=2;
	close(fd);
	pthread_exit((void*)&rtn);
	return NULL;
}

int	ota_upgrade_routine(int fd)
{
	int ret=0,rtn=-1,result,cfgfd,retry,srclen,destlen,fw_offset,fw_fd;
	unsigned long  phyaddr;
	PartUpdate_Info * cur_info =PartUpdate_info;
	ota_data_t	ota_temp;
	ota_up_thread_id=0;
	unsigned long buffer_addr,ota_compress_addr,ota_uncompress_addr;

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
	
	cfgfd = open("/dev/otadrv",O_RDWR);
	if(cfgfd != -1){
		ret=ioctl(cfgfd,OTA_GETFWSTATUS,&fw_flag);
		if(0==ret){
			if(fw_flag==1){
				ret = ioctl(cfgfd,OTA_GETORIFILELENTH,&result);
				if(0 == ret){
					destlen = result;
					DE_config ds_conf;
					void *deinst = getDeHandle();
					osd_de_get_config(deinst,&ds_conf,DE_CFG_ALL);
					src_width = ds_conf.input.width;
					src_height = ds_conf.input.height;
					ds_conf.input.enable=0;
					osd_de_set_Config(deinst,&ds_conf,DE_CFG_IN);

					//pHeap = getUIpHeap();
					if(!pHeap){
						pHeap = mallocOtaHeap(P_HEAP_OTA_SIZE);
						if(!pHeap){
							WLOGE("get heap_ota failed\n");
							rtn = -5;
							goto MALLOC_OTAHEAP_ERR;
						}
					}
				//	SWF_MemCheck(1);
					ota_uncompress =(char *)OSHmalloc(pHeap, destlen, &ota_uncompress_addr);					
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
					ota_compress = (char *)OSHmalloc(pHeap, result, &ota_compress_addr);
					if(!ota_compress){
						OTA_Info("Malloc memory for uncompress file error!\n");						
						rtn = -8;
						goto MALLOC_ZIP_ERR;						
					}
					char buf[25] = "prepare for upgrade ...";
					buffer = (char *)OSHmalloc(pHeap, OTA_UPGRADE_OSD_W*OTA_UPGRADE_OSD_H*2, &buffer_addr);
					if(!buffer){
						printf("[%s]%d, OSHmalloc fail\n", __func__, __LINE__);
						rtn = -9;
						goto MALLOC_OSD_ERR;
					}
					int osd_width = (src_width*OTA_UPGRADE_OSD_PERENT_W)/100;
					int osd_hight = (src_height*OTA_UPGRADE_OSD_PERENT_H)/100;
					ds_conf.input.enable = 1;
					ds_conf.input.width = OTA_UPGRADE_OSD_W;
					ds_conf.input.height = OTA_UPGRADE_OSD_H;
					_img_memset4((unsigned int*)buffer,0x80108010,ds_conf.input.width*ds_conf.input.height/2);
					ds_conf.input.img = (unsigned long*)buffer;
					ds_conf.input.bus_addr = buffer_addr;
					osd_de_set_Config(deinst,&ds_conf,DE_CFG_IN);					
					osdengine_init_osd(0,0,src_width,src_height,DE_PIX_FMT_OSDBIT4MODE,"/am7x/case/data/palette.plt");
					osdengine_enable_osd();
					osdengine_fill_osdrect((src_width-osd_width)/2, (src_height-osd_hight)/2, osd_width, osd_hight, 11);
					osdengine_update_osdrect(0,0,src_width, src_height);
					phyaddr = ota_uncompress_addr;
					ioctl(cfgfd,OTA_SETDESTADDR,phyaddr);
					phyaddr = ota_compress_addr;
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
		OSHfree(pHeap, ota_compress);
		ota_compress = NULL;
	}
MALLOC_ZIP_ERR:
	if(ota_uncompress){
		OSHfree(pHeap, ota_uncompress);
		ota_uncompress = NULL;
	}
	if(pHeap){
		freeOtaHeap(pHeap);
		pHeap = NULL;
	}
MALLOC_UNZIP_ERR:
MALLOC_OTAHEAP_ERR:
OPEN_OTADRV_ERR:
	close(cfgfd);	
	OTA_Info("Ota error,rtn=%d!\n",rtn);
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

int otaEntry(char *url)
{
	int ret=-1,fd=-1;
	char callbuf[128];
	
	if(url == NULL){
		WLOGE("url is NULL!!!");
		return -1;
	}
	
	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
	fd=open("/dev/otadrv",O_RDWR);
	if(fd<0){
		EZCASTLOG("Open /dev/otadrv failed!\n");
		WLOGE("error");
		return -EAGAIN;
	}

	if(access("/etc/version.conf", F_OK) == 0)
	{
		snprintf(callbuf, sizeof(callbuf), "cp -dprf %s %s", VERSION_CFG_PATH, OTA_CFG_PATH);
		system(callbuf);
	}
	
	ret=ota_download_routine(fd,url);
	if(ret < 0){
		WLOGE("Create ota download thread fail!!!\n");
		return -1;
	}

	
	do{
		sleep(1);
		WLOGI("Download: %d\%\n", ota_status.prg);
	}while(ota_status.state!=S_FINISHED&&ota_status.state!=S_ERROR);
	
	if(ota_status.state==S_FINISHED){
		
		if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
			sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
			system(callbuf);
		}
		fd=open("/dev/otadrv",O_RDWR);
		if(fd<0){
			printf("open /dev/otadrv failed!\n");
			return -EAGAIN;
		}
		#if (EZWIRE_TYPE==MIRAPLUG)
		EZDisplayMemRelease();
		#endif
		ret=ota_upgrade_routine(fd);
	}else{
		WLOGE("Download firmware fail!!!\n");
		ret = -1;
	}
	
	return ret;
}

