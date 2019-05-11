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
#include <upgrade_fw.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <semaphore.h>
#include "cardupgrade.h"
#include <swf_types.h>
#include <sys_nand.h>





#ifndef	MODULE_CONFIG_CARD_UPGRADE
int cardup_entry_main(int temp)
{
	temp =0;
	return 0;
}
int cardup_get_status(){return 0;}

#else

#if 1
#define CARDUP_Info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define  CardUP_Info(fmt,msg...)  do {} while (0)
#endif

#define CARDUP_FILELEN 64
#define PATH_SIZE 128

static char cardup_filename[CARDUP_FILELEN];
static pthread_t cardup_thread_id;
sem_t cardup_sem;
int   pthread_status;
PartUpdate_Info  PartUpdate_info[LinuxMax];
char * temp_buf=cardup_filename;
static Fwu_status_t cardup_status =
{
	.prg = INI_PRG,
	.state = S_INIT
};


int AnalysisUpdataFlag(char *flagbuf,PartUpdate_Info *cur_info)
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

	strncpy(cur_info->partupdate_attrstr,strP,j-1);//-1 becase have one ' '
	cur_info->partupdate_attrstr[j-1]=0;
	strP=strCurP+1;
	if((strchr(strP,'F'))||(strchr(strP,'f')))
		cur_info->partupdate_flag=0;
	else
		cur_info->partupdate_flag=1;

	return 1;
}

void AnalysisUpdataFile(void)
{
	char flagbuf[64];
	int j=0;
	char *strP,*strCurP;
	PartUpdate_Info *cur_info;

	strP=temp_buf;
	cur_info=PartUpdate_info;

	while(*strP!=0)
	{
		strCurP=strchr(strP,'\n');
		if( strCurP == NULL )
			break;
		j=strCurP-strP;

		strncpy(flagbuf,strP,j);
		flagbuf[j]=0;

		if(AnalysisUpdataFlag(flagbuf,cur_info))
			cur_info++;

		strP=strCurP+1;
	}
	if(*strP!=0)
		AnalysisUpdataFlag(strP,cur_info);

}


//make the thread sleep, the unit is millseconds
void _cardup_thread_sleep(unsigned int millsec)
{
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
}

void _cardup_sendmsg()
{
	CARDUP_Info("CARD UPGRADE Send MSG");
	SWF_Message(NULL,SWF_MSG_KEY_CARDUPGRADE,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
}


static void _cardup_sem_wait(sem_t * sem)
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
static void _cardup_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

void  *cardup_thread(void *arg)
{
	int fd=(int)arg;
	int ret=0,rtn=0,nandfd;
	CARDUP_Info("FD=%d",fd);
	if(cardup_filename[0]==0){
		CARDUP_Info("Carzy: fd may be error!");
		rtn = 0;
		cardup_status.state = S_ERROR;
		goto CARDUP_THREAD_OUT;
	}
	else{
			nandfd=open("/dev/nand_block",O_RDWR);
			if(nandfd != -1){
			ioctl(nandfd,IOCTL_WRITE_DISABLE,NULL);
			close(nandfd);
		}
		ret=ioctl(fd,FWU_SETPARTUPGRADE,PartUpdate_info);
		if(ret != 0){
			printf("copy upgrade configration file failed!\n");
		}
		ret=ioctl(fd,FWU_SETPATH,cardup_filename);
		if(ret !=0x0){
			CARDUP_Info("Set Path Error ret=%d!",ret);
			cardup_status.state = S_ERROR;
			rtn = 0;
		}
		else{
			while(cardup_status.state<S_FINISHED){
				_cardup_sem_wait(&cardup_sem);
				ret =ioctl(fd,FWU_GETSTATUS,&cardup_status);		
			//	if(cardup_status.prg)
			//		CARDUP_Info("prg = %d percent!state:%x", cardup_status.prg,cardup_status.state);
				_cardup_sem_post(&cardup_sem);
				_cardup_thread_sleep(1000);
			}
			//rtn = 1;
		}
		
	}
CARDUP_THREAD_OUT:
	if(cardup_status.state==S_FINISHED){
		rtn = 1;			
	}else{
		rtn = 0;
	}
//	printf("rtn =%d",rtn);
	ioctl(fd,FWU_EXITUP,&rtn);
	memset(cardup_filename,0,CARDUP_FILELEN);
	close(fd);
	nandfd=open("/dev/nand_block",O_RDWR);
	if(nandfd != -1){
		ioctl(nandfd,IOCTL_WRITE_ENABLE,NULL);
		close(nandfd);		
	}
	pthread_status = 2;
	cardup_thread_id =(~0);
	sem_destroy(&cardup_sem);
	pthread_exit((void*)rtn);
	return NULL;
}


/**
@brief check whether the file for upgrading is exist
**/
int cardup_check_file(char * filename)
{
	int fd=0,retry=0,result;
	int rtn=-1;
	int cfgfd=0;
	char  cfgfilepath[PATH_SIZE];
	char * pch=cfgfilepath;
	PartUpdate_Info * cur_info =PartUpdate_info;

	CARDUP_Info("filename=%s\n",filename);
	memset(cardup_filename,0,CARDUP_FILELEN);

	if(access("/sys/module/am7x_battery",F_OK)==0){
		do{
			system("rmmod am7x_battery.ko");
			retry++;
		}while((access("/sys/module/am7x_battery",F_OK)==0)&&(retry<10));
	}
	if(retry>=10){
		printf("can not remove am7x-battery.ko!\n");
		goto CARDUP_CHECK_OUT;
	}	
/*
	retry = 0;
	if(access("/sys/module/am7x_net",F_OK)==0){
		do{
			system("rmmod am7x_net.ko");
			retry++;
		}while((access("/sys/module/am7x_net",F_OK)==0)&&(retry<10));
	}
	if(retry>=10){
		printf("can not remove am7x-net.ko!\n");
		goto CARDUP_CHECK_OUT;
	}	
	*/
	system("lsmod -l");
	system("killall -9 thttpd");
	system("killall -9 udhcpd");
	system("killall -9 wpa_supplicant");	
	system("killall -9 hostapd");
	system("ps");
//	system("killall -9 ps");
//	system("killall -9 sh");
	memset((char *)cfgfilepath,0,PATH_SIZE);
	pch = strrchr(filename, '.');
	if(pch){
		strncpy(cfgfilepath,filename,pch-filename);
		strcat(cfgfilepath,".conf");
		cfgfd=open(cfgfilepath,O_RDONLY,0644);
		if(cfgfd != -1){
			temp_buf =(char *)malloc(1024);
			if(temp_buf){
				printf("config %s exist\n",cfgfilepath);
				memset((char *)temp_buf,0,1024);
				memset((char *)PartUpdate_info,0,sizeof(PartUpdate_Info)*LinuxMax);	
				read(cfgfd,temp_buf,1024);
				AnalysisUpdataFile();
#if 0	
				printf("PartUpdate Info:\n");
				cur_info=PartUpdate_info;
				for(retry=0;retry<LinuxMax;retry++)
				{
					if(0==cur_info->partupdate_attrstr[0])
						break;
					printf("%s=%d\n",cur_info->partupdate_attrstr,cur_info->partupdate_flag);
					cur_info++;
				}
#endif			
			//	printf("\n");
				free(temp_buf);
			}			
		close(cfgfd);
		}
	}
	if(access(filename,F_OK)!=-1){//the file is exist
		system("insmod /lib/modules/2.6.27.29/am7x_upgrade.ko");
		if ((fd = open("/dev/upgrade", O_RDWR)) < 0){
			CARDUP_Info("open file wrong!->return value:%d\n",fd);
			goto CARDUP_CHECK_OUT;
		}
		else{
			strcpy(cardup_filename,filename);
			rtn = fd;
			/*
			cfgfd=open("/dev/nand_block",O_RDWR);
			if(cfgfd != -1){
				result=ioctl(cfgfd,IOCTL_WRITE_DISABLE,NULL);
				close(cfgfd);
				if(0!=result)
					rtn= -1;			
			}
			else{
				rtn= -1;
			}
			*/
		}
	}
	else{
		CARDUP_Info("not exist filename=%s",filename);
	}

CARDUP_CHECK_OUT:
	return rtn;
}


int cardup_start_work(int fd)
{
	int ret;
	int rtn=0;
	cardup_thread_id=(~0);
	if(sem_init(&cardup_sem,0,0)==-1){
		CARDUP_Info("Creat Thread Error!");
		goto CARDUP_WORK_END;
	}
	ret = pthread_create(&cardup_thread_id,NULL,cardup_thread,(void*)fd);
	if(ret==-1){
		CARDUP_Info("Creat Thread Error!");
		sem_destroy(&cardup_sem);
		rtn=-1;
	}
	else{
		_cardup_sem_post(&cardup_sem);
		cardup_status.prg = INI_PRG;
		cardup_status.state = S_INIT;
		pthread_status=1;
		CARDUP_Info("Create Thread OK id=%d");
		rtn= 0;
	}
CARDUP_WORK_END:
	return rtn;
}


/**
@brief get the status of upgrading 
@param[in]	: none
@return the status which the lower 8 bits is the processing and the next 8 bits is the state
**/
//cardup_thread_id
int cardup_get_status()
{
	int rtn=0;
	if(pthread_status!=0){  
		_cardup_sem_wait(&cardup_sem);
		rtn = rtn |cardup_status.state;
		rtn = rtn <<8;
		rtn = rtn |cardup_status.prg;
		CARDUP_Info("getstatus:%x ", rtn);
		_cardup_sem_post(&cardup_sem);
	}
	else
		rtn = 0;
	return rtn;
}

int cardup_entry_main(int temp)
{
	int fd=-1;//richardyu 0509
	int ret=0;
	pthread_status=0;
//richardyu 0509
	switch(temp){
		case 0:
			printf("richard: searching  USB for ACTUPGRADE.BIN\n");
			//fd = cardup_check_file("/mnt/usb1/ACTUPGRADE.BIN");
			fd = cardup_check_file("/mnt/user1/thttpd/html/airusb/usb/ACTUPGRADE.BIN");
			break;
		case 1:
			printf("richard: searching Card for ACTUPGRADE.BIN\n");
			//fd = cardup_check_file("/mnt/card/ACTUPGRADE.BIN");
			fd = cardup_check_file("/mnt/user1/thttpd/html/airusb/sdcard/ACTUPGRADE.BIN");
			break;
		case 2:
			printf("richard: searching LAN for ACTUPGRADE.BIN\n");
			break;
	}
//richardyu 0509
	if(fd!=-1){
		ret = cardup_start_work(fd);
		if(ret==0)
			_cardup_sendmsg();
	}
	return 0;
}
#endif
