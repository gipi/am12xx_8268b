#ifdef MODULE_CONFIG_ROTATE_SENSOR

#include <pthread.h>
#include "unistd.h"
#include "stdio.h"
#define ROTATESENSOR_DEBUG
#ifdef ROTATESENSOR_DEBUG
#define rt_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define rt_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define rt_info(fmt,arg...) do{}while(0);
#define rt_err(fmt,arg...) do{}while(0);
#endif

#define ROTATESENSOR_GPIO_NUM 27
#define ROTATESENSOR_DITHER_COUNT 10 
static pthread_t rotatesensor_thread_id;
static void *rotatesensor_thread_ret;
static char current_status=0,prev_status=0;

enum{
		ROTATESENSOR_STATUS_V=0,   ////<under vertical position
		ROTATESENSOR_STATUS_H=1,	///< under horizontal position
};

//make the thread sleep, the unit is millseconds
void __rotatesensor_thread_sleep(unsigned int millsec)
{
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
}

void __rotatesensor_init_status()
{
	current_status = ROTATESENSOR_STATUS_H;
	prev_status = ROTATESENSOR_STATUS_H;
	rotatesensor_thread_id = 0;
	rotatesensor_thread_ret = NULL;
}

int __rotate_send_msg(int status)
{
	rt_info("HAHA Rotate Status Change Current Positon=%d (0=V,1=H)",status);
	if(status == ROTATESENSOR_STATUS_H){
		/**send your key msg**/
		//SWF_Message(NULL,SWF_MSG_KEY_F1,NULL);
		//SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	}
	else{
		
	}
	return 1;
}

void * rotatesensor_detect_thread(void *arg)
{
	unsigned int gpio_level=0;
	int last_count=0;
	while(1){
		get_gpio(ROTATESENSOR_GPIO_NUM,&current_status);
		if(current_status!=prev_status){
			//if(last_count>=ROTATESENSOR_DITHER_COUNT){
				__rotate_send_msg(current_status);
			//}
			last_count=0;
			prev_status=current_status;
		}
		else{
			last_count++;
		}
		//rt_info("Current Sensor Status===%d",current_status);
		pthread_testcancel();
		__rotatesensor_thread_sleep(500);
	}
	pthread_exit((void*)0);
	return NULL;
}


int rotatesensor_detect_thread_create()
{
	int ret=0;
	__rotatesensor_init_status();
	ret = pthread_create(&rotatesensor_thread_id,NULL,rotatesensor_detect_thread,NULL);
	if(ret==-1){
		rt_err("Create RotateSensor Detect Thread Error!");
		goto RT_DETECT_THREAD_END;
	}
	rt_info("Haha Create RotateSensor@@@@@@@@@@@@");
RT_DETECT_THREAD_END:
	return ret;
}

int rotatesensor_detect_thread_exit()
{
	pthread_cancel(rotatesensor_thread_id);
	pthread_join(rotatesensor_thread_id,&rotatesensor_thread_ret);
	rotatesensor_thread_id = 0;
	rotatesensor_thread_ret = NULL;
	return 0;
}
#endif/*MODULE_CONFIG_ROTATE_SENSOR*/

