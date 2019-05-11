#ifdef MODULE_CONFIG_AUDIO_RECORD
#include <dlfcn.h>
#include "swf_ext.h"
#include <string.h>
#include <stdio.h>
#include "audio_record.h"
#include "sys_cfg.h"
#include "sys_rtc.h"
#include "Audio_record_engine.h"
/** record status */
static RECORDER_STATE recordstat=REC_IDLE;

/**
* @brief prototype definitions for the record APIs
*/
typedef void *(*recordfunc_open)(void *param);
typedef int (*recordfunc_cmd)(void *handle,unsigned int cmd,unsigned int param);
typedef int (*recordfunc_close)(void *handle,void *param);

void* rec_dllhandle = NULL;
static recordfunc_open	rec_open	=	NULL;
static recordfunc_cmd	rec_cmd	=	NULL;
static recordfunc_close	rec_close	=	NULL;


void* record_handle = NULL;
file_iocontext_s f_io;
record_status_t status;
FILE *fp_write = NULL;
char record_filename[MAX_NAME_SIZE];

/******IO Interface*******/
extern long fui_os_fwrite( void *fp,unsigned char *ptr, unsigned long nbytes);
extern long fui_os_fseek_set(void *fp, long offset);
extern long fui_os_fseek_cur(void *fp, long offset);
extern long fui_os_fseek_end(void *fp, long offset);
extern long fui_os_ftell(void *fp);
/*************************/

#if 1
	#define recdbg_info(fmt, arg...) printf("RECINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	#define recdbg_err(fmt,arg...) printf("RECERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define recdbg_info(fmt, arg...)
	#define recdbg_err(fmt,arg...)
#endif

#define OPTIONAL

int record_open()
{
	char audiolibpath[128];
	//open audio dll
	sprintf(audiolibpath,"%s%s",AM_DYNAMIC_LIB_DIR,"librecorder.so");
	recdbg_info("audiolibpath == %s\n",audiolibpath);
	if(rec_dllhandle){
		dlclose(rec_dllhandle);
	}
	rec_dllhandle = dlopen(audiolibpath,RTLD_LAZY|RTLD_GLOBAL);
	recdbg_info("handle == 0x%x\n",rec_dllhandle);
	if(rec_dllhandle==NULL){
		recdbg_err("dl open error\n");
		goto dll_open_error;
	}

	rec_open = dlsym(rec_dllhandle,"audioEncOpen");
	if(rec_open == NULL){
		recdbg_err("rec_open error:%s\n",dlerror());
		goto init_api_error;
	}

	rec_cmd = dlsym(rec_dllhandle,"audioEncCmd");
	if(rec_cmd == NULL){
		recdbg_err("rec_cmd error\n");
		rec_open = NULL;
		goto init_api_error;
	}

	rec_close = dlsym(rec_dllhandle,"audioEncClose");
	if(rec_close == NULL){
		rec_open = NULL;
		rec_cmd = NULL;
		recdbg_err("rec_close error\n");
		goto init_api_error;
	}

	record_handle = rec_open(NULL);
	if(record_handle == NULL){
		rec_open = NULL;
		rec_cmd = NULL;
		rec_close = NULL;
		recdbg_err("record_handle error\n");
		goto init_api_error;
	}

	return 0;

init_api_error:
	if(rec_dllhandle){
		dlclose(rec_dllhandle);
		rec_dllhandle = NULL;
	}
	
dll_open_error:
	return -1;
}


static int ar_open(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(record_handle==NULL){
		recdbg_info("open record engine error\n");
		ret = record_open();
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_set_rate(void * handle)
{
	int sample_rate;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	sample_rate = Swfext_GetNumber();
	recdbg_info("sample rate is %d",sample_rate);
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		return -1;
	}
	ret = rec_cmd(record_handle,ENC_SET_AUDIO_PARAM,sample_rate);//set sample rate,8000,11025,12000,16000,22050,24000,32000,44100,48000,96000
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

#ifdef OPTIONAL
static int ar_set_gain(void * handle)
{
	int audio_gain;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	audio_gain = Swfext_GetNumber();
	recdbg_info("audio_gain is %d",audio_gain);
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_set_gain_out;
	}
	ret = rec_cmd(record_handle,ENC_SET_AUDIO_GAIN,audio_gain);//0~7
	
ar_set_gain_out:	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_set_record_time(void * handle)
{
	int record_time;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	record_time = Swfext_GetNumber();
	recdbg_info("record_time is %d",record_time);
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_set_record_time_out;
	}
	ret = rec_cmd(record_handle,ENC_SET_RECORD_TIME,record_time);//total record time second
ar_set_record_time_out:	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}
#endif

static int ar_set_record_filename(void * handle)
{
	int ret= 0;
	INT8S * filename = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	filename = Swfext_GetString();
	memset(record_filename,0,sizeof(record_filename));
	if(strlen(filename)<MAX_NAME_SIZE){
		strcpy(record_filename,filename);
	}
	else{
		ret = -1;
		recdbg_err("the length of file name is more than 256\n");
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_start_record(void * handle)
{
	rtc_date_t date;
	rtc_time_t time;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_start_record_out;
	}
	
	if(record_filename[0]=='\0'){
		tm_get_rtc(&date,&time);
		sprintf(record_filename,"/mnt/udisk/%4d%02d%02d-%02d%02d.wav",date.year,date.month,date.day,time.hour,time.min);
	}
	recdbg_info("record_filename is %s",record_filename);
	fp_write = fopen(record_filename,"wb+");
	if(fp_write==NULL){
		recdbg_err("can not open %s\n",record_filename);
		ret = -1;
		goto ar_start_record_out;
	}
	f_io.handle = (void *)fp_write;
	f_io.seek_cur = fui_os_fseek_cur;
	f_io.seek_end = fui_os_fseek_end;
	f_io.seek_set = fui_os_fseek_set;
	f_io.tell = fui_os_ftell;
	f_io.write = fui_os_fwrite;
	ret = rec_cmd(record_handle,ENC_START_RECORDING,(int)(&f_io));
	recordstat = REC__RECORDING;
	
ar_start_record_out:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

#ifdef OPTIONAL
static int get_record_status()
{
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		return -1;
	}
	rec_cmd(record_handle,ENC_GET_STATUS,(int)(&status));
	return 0;
}

static int ar_get_record_status(void * handle)
{
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = get_record_status();
	if(rtn == -1){
		recdbg_err("get record status error!\n");
		goto ar_get_record_status_out;
	}
ar_get_record_status_out:
	Swfext_PutNumber(recordstat);
	SWFEXT_FUNC_END();	
}

static int ar_get_recording_time(void * handle)
{
	int rtn = 0;
	int record_time = 0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = get_record_status();
	if(rtn == -1){
		recdbg_err("get record status error!\n");
		goto ar_get_recording_time_out;
	}
	record_time = status.time;
ar_get_recording_time_out:
	//recdbg_err("the recording time is %d\n",record_time);
	Swfext_PutNumber(record_time);
	SWFEXT_FUNC_END();	
}

#endif

static int ar_pause_record(void * handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_pause_record_out;
	}
	ret = rec_cmd(record_handle,ENC_PAUSE_RECORDING,0);
	recordstat = REC_PAUSE;
ar_pause_record_out:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_resume_record(void * handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_resume_record_out;
	}
	ret = rec_cmd(record_handle,ENC_CONTINUE_RECORDING,0);
	recordstat = REC__RECORDING;
ar_resume_record_out:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_stop_record(void * handle)
{
	int ret;
	int fd_write;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(record_handle==NULL){
		recdbg_err("open record engine error\n");
		ret = -1;
		goto ar_stop_record_out;
	}
	if(recordstat==REC_PAUSE)
		rec_cmd(record_handle,ENC_CONTINUE_RECORDING,0);
	ret = rec_cmd(record_handle,ENC_STOP_RECORDING,0);
	if(fp_write){
		fflush(fp_write);
		fd_write = fileno(fp_write);
		fsync(fd_write);
		fclose(fp_write);
		recdbg_err("fp write is closed!");
		fp_write = NULL;
	}
	recordstat = REC_STOPPED;
ar_stop_record_out:	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ar_close(void * handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	recdbg_info("record_handle is 0x%x",record_handle);
	if(record_handle){
		recdbg_info("close record handle\n");
		ret = rec_close(record_handle,0);
		record_handle = NULL;
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

int swfext_audio_record_register(void)
{
	SWFEXT_REGISTER("ar_Open", ar_open);
	SWFEXT_REGISTER("ar_Set_Rate", ar_set_rate);
	SWFEXT_REGISTER("ar_Set_Gain", ar_set_gain);
	SWFEXT_REGISTER("ar_Set_Record_Time", ar_set_record_time);
	SWFEXT_REGISTER("ar_Start_Record", ar_start_record);
	SWFEXT_REGISTER("ar_Get_Record_Status", ar_get_record_status);
	SWFEXT_REGISTER("ar_Pause_Record", ar_pause_record);
	SWFEXT_REGISTER("ar_Resume_Record", ar_resume_record);
	SWFEXT_REGISTER("ar_Stop_Record", ar_stop_record);
	SWFEXT_REGISTER("ar_Close", ar_close);
	SWFEXT_REGISTER("ar_Get_Recording_Time", ar_get_recording_time);
	SWFEXT_REGISTER("ar_Set_Record_Filename", ar_set_record_filename);

	return 0;
}
#else
int swfext_audio_record_register(void)
{
	return 0;
}
#endif	/** MODULE_CONFIG_AUDIO_RECORD */