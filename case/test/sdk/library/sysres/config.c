#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <fcntl.h>

#define TEST_SYS_TIMER	0
#define TEST_SYS_RTC	1
#define TEST_SYS_GPIO	0

#if TEST_SYS_TIMER
static void timer_isr(int *pdata)
{
	printf("timer ok : %d\n",(*pdata)++);
}
#endif

#if TEST_SYS_RTC
#define TEST_DEFAULT_MIN	34
#define TEST_DEFAULT_SEC	30
void rtc_print(rtc_date_t *rtc_date,rtc_time_t *rtc_time)
{
	printf("\n\nget_rtc:%d:%d:%d:%d\n",rtc_date->year,rtc_date->month,rtc_date->day,rtc_date->wday);
	printf("%d:%d:%d\n",rtc_time->hour,rtc_time->min,rtc_time->sec);
}
#endif

int main(void)
{
	int fd,i=0;
	char value=-1;
	#if TEST_SYS_RTC
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	#endif
	#if TEST_SYS_TIMER
	volatile int count = 0;
	#endif
	
	printf("This test code is mainly for system lib'\n");
	
	#if TEST_SYS_TIMER
	fd = am_timer_create(5000, (void(*)(void*))timer_isr, &count);
	if(fd>0){
		while(count<10);
		am_timer_del(fd);
	}
	#endif
	
	#if TEST_SYS_RTC
	tm_get_rtc(&rtc_date,&rtc_time);
	rtc_print(&rtc_date,&rtc_time);

	rtc_date.year = 2010;
	rtc_date.month = 4;
	rtc_date.day = 23;
	rtc_date.wday = 5;

	rtc_time.hour = 10;
	rtc_time.min = TEST_DEFAULT_MIN;
	rtc_time.sec =TEST_DEFAULT_SEC;
	rtc_print(&rtc_date,&rtc_time);
	
	tm_set_rtc(&rtc_date,&rtc_time);
	memset(&rtc_date,0,sizeof(rtc_date));
	memset(&rtc_time,0,sizeof(rtc_time));
	tm_get_rtc(&rtc_date,&rtc_time);
	rtc_print(&rtc_date,&rtc_time);

	rtc_time.sec += 10;
	tm_open_alarm(&rtc_date, &rtc_time);
	printf("==set alarm:\n");
	tm_get_alarm(&rtc_date, &rtc_time);
	rtc_print(&rtc_date,&rtc_time);
#if 0 //test alarm irq
	while(1){
		sleep(1);
		tm_get_rtc(&rtc_date,&rtc_time);
		rtc_print(&rtc_date,&rtc_time);
		if(rtc_time.min-TEST_DEFAULT_MIN>0)
			break;
	}
#else //test rtc read based on alarm irq
	tm_wait_alarm(&i);
	printf("rtc read data = 0x%x\n",i);
#endif
	tm_close_alarm();
	#endif

	#if TEST_SYS_GPIO
	get_gpio(2,&value);
	printf("get gpio2=%d\n",value);
	value = value?0:1;
	set_gpio(2,value);
	printf("set gpio2=%d\n",value);
	get_gpio(2,&value);
	printf("get gpio2=%d\n",value);
	#endif
	
	#if 0
	struct am_sys_msg msg;
	//system("/etc/sysinit");
	fd = open("/dev/sysmsg", O_RDWR);
	if(fd==-1){
		printf("cant open sysmsg device\n");
		exit(-19);
	}
	/*
	for(i=0;i<5;i++){
		read(fd, &msg, sizeof(struct am_sys_msg));
		printf("msg.type=%d\n",msg.type);
	}
	*/
	msg.type = SYSMSG_CARD;
	i = write(fd,&msg,sizeof(struct am_sys_msg));
	printf("write ret is %d\n",i);
	close(fd);
	#endif
	exit(0);
}
