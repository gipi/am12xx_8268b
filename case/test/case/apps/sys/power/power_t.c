#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys_pmu.h>
#include <sys_rtc.h>

#define TEST_SYS_STANDBY	1
#define TEST_SYS_POWER		2
#define TEST_SYS_DPOWER	3
#define TEST_POWER_MODE	TEST_SYS_STANDBY

#if TEST_POWER_MODE==TEST_SYS_STANDBY
#define TEST_WAKE_MODE	EXT_DC_MODE|IRE_DC_MODE|KEYE_DC_MODE

#define STANDBY_BIN_PATH	"/am7x/sdk/standby.bin"
#define SYS_PM_STAT_PATH	"/sys/power/state"
#elif TEST_POWER_MODE==TEST_SYS_POWER
#define TEST_POFF_MODE		PM_SPEC_MODE

void rtc_print(rtc_date_t *rtc_date,rtc_time_t *rtc_time)
{
	printf("\n\nget_rtc:%d:%d:%d:%d\n",rtc_date->year,rtc_date->month,rtc_date->day,rtc_date->wday);
	printf("%d:%d:%d\n",rtc_time->hour,rtc_time->min,rtc_time->sec);
}
#endif

#define TEST_DEFAULT_MIN	10
#define TEST_DEFAULT_SEC	30

#define PMU_DEV_PATH		"/dev/pmu"

int main(void)
{
	int fd,ret,i;
	char test_buf[128];
	char *func_buf;
	struct stat fbuf;
	struct am_pm_info pm_info;
	struct am_pm_arg pm_arg;
	
	printf("This test code is mainly for pm\n");

#if TEST_POWER_MODE==TEST_SYS_STANDBY
	/*****************************************
	config standby info
	******************************************/
	fd = open(PMU_DEV_PATH,O_RDWR);
	if(fd==-1){
		printf("can not open pmu device\n");
		exit(-19);
	}
//	ioctl(fd,AM_PMU_VALID,&ret);
//	if(!ret){
	{
	/****************************************
		read standby.bin
		******************************************/
#if 0
		int tfd;
		ret = stat(STANDBY_BIN_PATH,&fbuf);
		if(ret<0)
			exit(-19);
		func_buf = (char *)malloc(fbuf.st_size);
		if(!func_buf)
			exit (-12);
	
		tfd = open(STANDBY_BIN_PATH,O_RDONLY);
		if(tfd==-1){
			printf("open standby.bin fail\n");
			exit(-19);
		}
		ret = read(tfd,func_buf,fbuf.st_size);
		close(tfd);

		/********set pmu**********/
		memset(&pm_info,0,sizeof(struct am_pm_info));
#endif		
	//	pm_info.func_addr = func_buf;
	//	pm_info.func_size = fbuf.st_size;
		pm_info.arg.wakeup_mode = TEST_WAKE_MODE;

#if 0		
		switch(pm_info.arg.wakeup_mode){
		case RTC_DC_MODE:{
			rtc_date_t rtc_date;
			rtc_time_t rtc_time;

			rtc_date.year = 2010;
			rtc_date.month = 4;
			rtc_date.day = 23;
			rtc_date.wday = 5;

			rtc_time.hour = 10;
			rtc_time.min = TEST_DEFAULT_MIN;
			rtc_time.sec =TEST_DEFAULT_SEC;
			tm_set_rtc(&rtc_date,&rtc_time); //set rtc time
			sleep(1);
			tm_get_rtc(&rtc_date,&rtc_time);
			printf("time %d:%d:%d\n",rtc_time.hour,rtc_time.min,rtc_time.sec);

			rtc_time.sec += 10;
			pm_info.arg.rtc_param[0] = AM_PMU_MK_DATE(rtc_date.year, rtc_date.month, rtc_date.day);	//alarm date
			pm_info.arg.rtc_param[1] = AM_PMU_MK_TIME(rtc_time.hour,rtc_time.min,rtc_time.sec);	// alarm time
			break;
		}
		case EXT_DC_MODE:
			printf("Hey guys, this mode has not been implemented yet!"); 
			break;
		case KEYE_DC_MODE:
			pm_info.arg.key_param[0] = 3;	//count times
			pm_info.arg.key_param[1] = 6;	//key number
			break;
		case IRE_DC_MODE:
			pm_info.arg.ire_param[0] = 1;   //valid code number
			/*irkey code*/
			pm_info.arg.ire_param[1] = 0x12;//0xc0;//0x12; 
			pm_info.arg.ire_param[2] = 0;
			pm_info.arg.ire_param[3] = 0;
			pm_info.arg.ire_param[4] = 0;
			break;
		case KEY_NDC_MODE:
			pm_info.arg.sram_param[0] = 60;	//gpio num
			break;
		default:
			break;
		}
#endif	
		if((pm_info.arg.wakeup_mode&RTC_DC_MODE) == RTC_DC_MODE)
		{
			rtc_date_t rtc_date;
			rtc_time_t rtc_time;

			rtc_date.year = 2010;
			rtc_date.month = 4;
			rtc_date.day = 23;
			rtc_date.wday = 5;

			rtc_time.hour = 10;
			rtc_time.min = TEST_DEFAULT_MIN;
			rtc_time.sec =TEST_DEFAULT_SEC;
			tm_set_rtc(&rtc_date,&rtc_time); //set rtc time
			sleep(1);
			tm_get_rtc(&rtc_date,&rtc_time);
			printf("time %d:%d:%d\n",rtc_time.hour,rtc_time.min,rtc_time.sec);

			rtc_time.sec += 10;
			pm_info.arg.rtc_param[0] = AM_PMU_MK_DATE(rtc_date.year, rtc_date.month, rtc_date.day);	//alarm date
			pm_info.arg.rtc_param[1] = AM_PMU_MK_TIME(rtc_time.hour,rtc_time.min,rtc_time.sec);	// alarm time
		}

		if((pm_info.arg.wakeup_mode&EXT_DC_MODE) == EXT_DC_MODE)
		{
			pm_info.arg.ext_param[0] = 28;
			printf("Hey guys, this mode has not been implemented yet! mode :%x\n",pm_info.arg.wakeup_mode); 
		}

		if((pm_info.arg.wakeup_mode&KEYE_DC_MODE) == KEYE_DC_MODE)
		{
			pm_info.arg.key_param[0] = 3;	//count times
			pm_info.arg.key_param[1] = 6;	//key number
		}

		if((pm_info.arg.wakeup_mode&IRE_DC_MODE) == IRE_DC_MODE)
		{
			pm_info.arg.ire_param[0] = 1;   //valid code number
			/*irkey code*/
			pm_info.arg.ire_param[1] = 0x12;//0xc0;//0x12; 
			pm_info.arg.ire_param[2] = 0;
			pm_info.arg.ire_param[3] = 0;
			pm_info.arg.ire_param[4] = 0;
		}

		if((pm_info.arg.wakeup_mode&KEY_NDC_MODE) == KEY_NDC_MODE)
			pm_info.arg.sram_param[0] = 60;	//gpio num

		ioctl(fd,AM_PMU_SET,&pm_info);
	}
	
	close(fd);
	/*****************************************
	enter standby
	*******************************************/
	fd = open(SYS_PM_STAT_PATH, O_RDWR);
	if(fd==-1){
		printf("cant open power state\n");
		exit(-19);
	}

	/*read status*/
	ret = read(fd,test_buf,128);
	if(ret>0){
		test_buf[ret] = '\0';
		printf("power state:%s\n",test_buf);
		memcpy(test_buf,"mem",3);
		write(fd,test_buf,3);
	}else{
		printf("read failed :%d\n",ret);
	}

	close(fd);
#elif TEST_POWER_MODE==TEST_SYS_POWER
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	fd = open(PMU_DEV_PATH,O_RDWR);
	if(fd==-1){
		printf("can not open pmu device\n");
		exit(-19);
	}
	pm_arg.wakeup_mode = TEST_POFF_MODE;

	rtc_date.year = 2010;
	rtc_date.month = 4;
	rtc_date.day = 23;
	rtc_date.wday = 5;

	rtc_time.hour = 10;
	rtc_time.min = TEST_DEFAULT_MIN;
	rtc_time.sec =TEST_DEFAULT_SEC;
	tm_set_rtc(&rtc_date,&rtc_time); //set rtc time
	sleep(1);
	tm_get_rtc(&rtc_date,&rtc_time);
	printf("time %d:%d:%d\n",rtc_time.hour,rtc_time.min,rtc_time.sec);

	rtc_time.sec += 10;
	pm_arg.param[0] = AM_PMU_MK_DATE(rtc_date.year, rtc_date.month, rtc_date.day);	//on alarm date
	pm_arg.param[1] = AM_PMU_MK_TIME(rtc_time.hour,rtc_time.min,rtc_time.sec);	// on alarm time

	if(pm_arg.wakeup_mode == PM_GEN_MODE)
		pm_arg.param[2] = RTC_ALARM_GPO_LOW;
	else
		pm_arg.param[2] = RTC_ALARM_PW_MAX-4;
	rtc_time.sec += 10;
	pm_arg.param[3] = AM_PMU_MK_DATE(rtc_date.year, rtc_date.month, rtc_date.day);	//off alarm date
	pm_arg.param[4] = AM_PMU_MK_TIME(rtc_time.hour,rtc_time.min,rtc_time.sec);	// off alarm time
	
	ioctl(fd,AM_PMU_POFF,&pm_arg);

	close(fd);

	while(1){
		sleep(1);
		tm_get_rtc(&rtc_date,&rtc_time);
		rtc_print(&rtc_date,&rtc_time);
		if(rtc_time.min-TEST_DEFAULT_MIN>0)
			break;
	}
#elif TEST_POWER_MODE==TEST_SYS_DPOWER
	fd = open(PMU_DEV_PATH,O_RDWR);
	if(fd==-1){
		printf("can not open pmu device\n");
		exit(-19);
	}
	for(i=0;i<0xffff;i++){
		printf("enter low\n");
		ioctl(fd,AM_PMU_PLOW,NULL);
		sleep(1);
		printf("enter high\n");
		ioctl(fd,AM_PMU_PHIGH,NULL);
		sleep(1);
	}
	close(fd);
#endif


	exit(0);
}
