#include "swf_ext.h"
#include "multialarm.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "errno.h"

#include <sys/msg.h>
#include <sys_msg.h>

#define al_info(fmt,arg...)		printf("INFO[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)
#define al_err(fmt,arg...)	printf("ERROR[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)

#define ALARM_EN	0
#define ALARM_INTERVAL_SEC 2
#define ALARM_INTERVAL_MILSEC 0

static int p_msg_id=-1;
static int cur_alarm_on_id=-1;

char send_msg_info(void *pararcv,void *parasend)
{
	int msgq_id = (int)pararcv;
	int alarm_ID = (int)parasend;
	al_info("!!!msgq_id==%d",msgq_id);
	if(msgq_id!=-1){
		int rtn;
		struct am_sys_msg msg;
		msg.type = SYSMSG_ALARM;
		msg.dataload = (unsigned int)alarm_ID;
		rtn = msgsnd(msgq_id,&msg,sizeof(struct am_sys_msg),IPC_NOWAIT);
		if(rtn==-1){
			al_err("Send msg Faild! error==%d\n",errno);
			return -1;
		}
		else{
			//printf("######Send MSG OK######rtn=%d\n",rtn);
			return 0;
		}
	}
	else
		return -1;
}

void get_msgq_info(int msgq_id)
{
	int rtn;
	struct msqid_ds msg_ds;
	rtn = msgctl(msgq_id,IPC_STAT,&msg_ds);
	printf("rtn==%d,lspid=%d,lrpid=%d,num=%d",rtn,msg_ds.msg_lspid,msg_ds.msg_lrpid,msg_ds.msg_qnum);
}

static void alarm_check_func(int signo)
{
	if(signo==SIGALRM){
		al_check();
		power_off_check();
		screen_off_check();
		
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		medialib_check_udisk_written();
	}
}

char alarm_init(void* para)
{
	int res;
	struct itimerval tick;   
	memset(&tick, 0, sizeof(tick));   
	if(ALARM_EN==1){
		alarm_msg_send_t msg_send;
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		signal(SIGALRM, alarm_check_func); 
		tick.it_interval.tv_sec = ALARM_INTERVAL_SEC;
		tick.it_interval.tv_usec = ALARM_INTERVAL_MILSEC;
		tick.it_value.tv_sec = ALARM_INTERVAL_SEC;
		tick.it_value.tv_usec = ALARM_INTERVAL_MILSEC;
		//alarm(ALARM_INTERVAL_SEC);
		res = setitimer(ITIMER_REAL, &tick, NULL);   
		if (res) {   
		   al_err("Set timer err: No=%d",errno);
		   return -1;
		}  
		else
			al_info("Alarm Timer Set OK!");
		msg_send.pararcv = para;
		msg_send.msg_send_func = send_msg_info;
		//printf("Msg_Info==0x%x\n",send_msg_info);
		al_init_alarms(&msg_send);
	}
	return 0;
	
}


////º¯Êý×¢²á
typedef enum{
	ALARM_CMD_ENABLE    =0,//enable alarm
	ALARM_CMD_DAY       =1,//the days to be noticed
	ALARM_CMD_SNOOZE    =2,//the interval time 
	ALARM_CMD_TIME      =3,//the time to be set
	ALARM_CMD_USERID    =4,//user can used the value to do someting
	ALARM_CMD_NAME      =5,//the name of a alarm
	ALARM_CMD_TOTNUM    =7,
	ALARM_CMD_IDXTOID   =8,	
}al_cmd_e;

#ifdef MODULE_CONFIG_ALARM
static int alm_add_alarm(void * handle)
{
	alarm_info_t palarminfo;
	int alarm_ID=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	memset(&palarminfo,0,sizeof(alarm_info_t));
	alarm_ID = (int)al_add_alarm(&palarminfo);
	
	Swfext_PutNumber(alarm_ID);
	SWFEXT_FUNC_END();
}


static int alm_del_alarm(void * handle)
{
	char alarm_id=0;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	alarm_id = (char)Swfext_GetNumber();
	rtn = (int)al_del_alarm(alarm_id);
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


static int alm_set_alarm(void * handle)
{
	alarm_info_t alarm_info;
	INT8S rtn,cmd,ID;
	INT32S value;
	INT8S *name=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	cmd = Swfext_GetNumber();
	ID = Swfext_GetNumber();
	value = Swfext_GetNumber();
	name = Swfext_GetString();
	rtn = al_get_alarm_info(ALARM_GET_INFO_PHYID,ID,&alarm_info);
	if(rtn!=-1){
		switch(cmd){
			case ALARM_CMD_ENABLE:
				alarm_info.enable = value;
				break;
			case ALARM_CMD_DAY:
				alarm_info.days = value;
				break;
			case ALARM_CMD_SNOOZE:
				alarm_info.snooze = value;
				break;
			case ALARM_CMD_TIME:
				alarm_info.hour = value/60;
				alarm_info.min = value%60;
				break;
			case ALARM_CMD_USERID:
				alarm_info.user_id = value;
				break;
			case ALARM_CMD_NAME:
				memcpy(alarm_info.name,name,ALARM_NAME_LEN-1);
				alarm_info.name[ALARM_NAME_LEN-1]='\0';
				break;
			default:
				printf("CMD in Call setAlarm is error!\n");
		}
		rtn = al_set_alarm(ID,&alarm_info);
	}
	
	Swfext_PutNumber((int)rtn);
	SWFEXT_FUNC_END();
}


static int alm_get_alarm_info(void * handle)
{
	INT8S rtn,cmd,idx;
	INT32S value;
	alarm_info_t alarm_info;
	INT8S str[ALARM_NAME_LEN];
	SWFEXT_FUNC_BEGIN(handle);
	
	memset(str,0,ALARM_NAME_LEN);
	cmd = Swfext_GetNumber();
	idx = Swfext_GetNumber();
	if(cmd==ALARM_CMD_TOTNUM){
		rtn = al_get_alarm_info(ALARM_GET_INFO_PHYID,idx,&alarm_info);
		sprintf(str,"%d",rtn);
	}
	else	if(cmd ==ALARM_CMD_IDXTOID){
		rtn = al_get_alarm_info(ALARM_GET_INFO_PHYID,idx,&alarm_info);
		if(rtn!=-1)
			sprintf(str,"%d",rtn);
	}
	else{
		rtn = al_get_alarm_info(ALARM_GET_INFO_PHYID,idx,&alarm_info);
		switch(cmd){
			case ALARM_CMD_ENABLE:
				value=alarm_info.enable;
				break;
			case ALARM_CMD_DAY:
				 value=alarm_info.days;
				break;
			case ALARM_CMD_SNOOZE:
				value=alarm_info.snooze;
				break;
			case ALARM_CMD_TIME:
				value = alarm_info.hour*60+alarm_info.min;
				break;
			case ALARM_CMD_USERID:
				value=alarm_info.user_id;
				break;
			case ALARM_CMD_NAME:
				memcpy(str,alarm_info.name,ALARM_NAME_LEN);
				break;
			default:
				printf("CMD in Call setAlarm is error!\n");
				goto EXIT;
				break;
		}
		if(cmd!=ALARM_CMD_NAME)
			sprintf(str,"%d",value);
	}

EXIT:
	Swfext_PutString(str);
	SWFEXT_FUNC_END();
}


static int alm_close_alarm(void * handle)
{
	char ID;
	SWFEXT_FUNC_BEGIN(handle);
	
	ID = Swfext_GetNumber();
	al_close_alarm(ID);
	
	SWFEXT_FUNC_END();
}

static int alm_open_alarm(void * handle)
{
	char ID;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	ID = Swfext_GetNumber();
	al_open_alarm(ID);
	
	SWFEXT_FUNC_END();
}

static int alm_load_info(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	al_load_alarm_info();

	SWFEXT_FUNC_END();
}


static int alm_store_info(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	al_store_alarm_info();
	
	SWFEXT_FUNC_END();
}

int set_alarm_on_id(int AlarmID)
{
	cur_alarm_on_id = AlarmID;
	return 0;
}

int get_alarm_on_id()
{
	int ID = cur_alarm_on_id;
	cur_alarm_on_id = -1;
	return ID;
}

static int alm_get_alarm_on_id(void * handle)
{
	int ID=-1;
	
	SWFEXT_FUNC_BEGIN(handle);

	ID = get_alarm_on_id();
	if(ID==-1){
		al_err("Sorry Alarm_on Id is Error!");
	}
	
	Swfext_PutNumber(ID);
	SWFEXT_FUNC_END();
}

#else
static int alm_add_alarm(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_del_alarm(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_set_alarm(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_get_alarm_info(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_close_alarm(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int alm_open_alarm(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int alm_load_info(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_store_info(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int alm_get_alarm_on_id(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	al_err("Open Alarm Config First !!!");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

int set_alarm_on_id(int AlarmID)
{
	return 0;
}

#endif

int swfext_alarm_register(void)
{
	SWFEXT_REGISTER("al_addAlarm", alm_add_alarm);
	SWFEXT_REGISTER("al_delAlarm", alm_del_alarm);
	SWFEXT_REGISTER("al_setAlarm", alm_set_alarm);
	SWFEXT_REGISTER("al_getAlarmInfo", alm_get_alarm_info);
	SWFEXT_REGISTER("al_closeAlarm", alm_close_alarm);
	SWFEXT_REGISTER("al_openAlarm", alm_open_alarm);
	SWFEXT_REGISTER("al_loadInfo", alm_load_info);
	SWFEXT_REGISTER("al_storeInfo",alm_store_info);
	SWFEXT_REGISTER("al_getAlarmOnID",alm_get_alarm_on_id);

	return 0;
}

