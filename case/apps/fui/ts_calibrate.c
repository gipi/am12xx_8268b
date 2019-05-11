#ifdef MODULE_CONFIG_TS_CALIBRATE
#include "swf_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "tslib.h"
#include "sys_cfg.h"
#include <dlfcn.h>
#include "ipc_key.h"
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fui_common.h>
#include "math.h"


typedef void *(*calibrate_open)(const char *, int);
typedef int (*calibrate_config)(struct tsdev *);
typedef int (*calibrate_getsample)(struct tsdev *ts, calibration *cal,int index, int x, int y, char *name);
typedef void *(*calibrate_perform)(calibration *cal);
typedef int (*calibrate_read)(struct tsdev *, struct ts_sample *, int);
typedef int (*calibrate_close)(struct tsdev *);

void* tsdllhandle = NULL;

static calibrate_open		cal_open	=	NULL;
static calibrate_config		cal_config	=	NULL;
static calibrate_getsample	cal_getsample	=	NULL;
static calibrate_perform		cal_perform_calibration	=	NULL;
static calibrate_read		cal_read	=	NULL;
static calibrate_close		cal_close	=	NULL;


typedef struct _ts_content{
	int touchflag;
	int index;
	struct tsdev *ts;
	calibration cal;
}ts_content;
//**********************************//
#define DEBUG  0
#if DEBUG 
#define DBG_MSG(format,args...)	printf(format,##args)
#else
#define DBG_MSG(format,args...)	do{}while(0)
#endif

#define x_err_distance  100
#define y_err_distance  100
#define set_x   50
#define set_y   50
//*******************************//
static int _tscal_dll_open(void)
{
	char tslibpath[128];
	//open ts dll
	sprintf(tslibpath,"%s%s",AM_DYNAMIC_LIB_DIR,"libts.so");
	printf("tslibpath == %s\n",tslibpath);
	if(tsdllhandle){
		dlclose(tsdllhandle);
	}
	tsdllhandle = dlopen(tslibpath,RTLD_LAZY|RTLD_GLOBAL);
	printf("handle == 0x%x\n",tsdllhandle);
	if(tsdllhandle==NULL){
		printf("dl open error\n");
		goto dll_open_error;
	}

	//restore ts dll api
	cal_open = dlsym(tsdllhandle,"ts_open");
	if(cal_open == NULL){
		printf("a_open error:%s\n",dlerror());
		goto init_api_error;
	}

	cal_config = dlsym(tsdllhandle,"ts_config");
	if(cal_config == NULL){
		printf("a_cmd error\n");
		goto init_api_error;
	}

	cal_getsample = dlsym(tsdllhandle,"get_sample");
	if(cal_getsample == NULL){
		printf("a_close error\n");
		goto init_api_error;
	}


	cal_perform_calibration = dlsym(tsdllhandle,"perform_calibration");
	if(cal_perform_calibration == NULL){
		printf("a_close error\n");
		goto init_api_error;
	}
	
	cal_read = dlsym(tsdllhandle,"ts_read");
	if(cal_read == NULL){
		printf("a_close error\n");
		goto init_api_error;
	}

	cal_close = dlsym(tsdllhandle,"ts_close");
	if(cal_close == NULL){
		printf("ts_close error\n");
		goto init_api_error;
	}
	return 0;


init_api_error:
	
	if(tsdllhandle){
		dlclose(tsdllhandle);
		tsdllhandle = NULL;
	}
	
dll_open_error:
	
	return -1;
}

int _tscal_dll_close()
{

	cal_open = NULL;
	cal_config = NULL;
	cal_getsample = NULL;
	cal_perform_calibration = NULL;
	cal_read = NULL;

	if(tsdllhandle){
		dlclose(tsdllhandle);
		tsdllhandle = NULL;
	}
	
	return 0;
}
ts_content* tscon;

/********************************
*@检查所按的点是否在相应点所要求的范围内
*@ index :设置点的序号(0~~4)
*@cal:结构体指针，可以查询到所按下点的实际坐标
*@ return 0 在相应的范围内
		 -1不在范围内
*********************************/
static char check_data(int index,calibration *cal)
{
	char result=-1;
	DBG_MSG("DBG_MSG:index=%d,x=%d,y=%d\n",index,cal->x[index],cal->y[index]);
	switch(index)
	{
		case 0://Top left
		{
			if((abs(cal->x[index]-set_x)>x_err_distance)|(abs(cal->y[index]-set_y)>y_err_distance))
			{
				DBG_MSG("dot outTop left range\n");
				result=-1;
			}
			else
			{
				DBG_MSG("dot in Top left range \n");
				result=0;
			}
			break;
		}
		case 1://Top right
		{
			if((abs(cal->x[index]-(IMAGE_WIDTH_E-set_x))>x_err_distance)|(abs(cal->y[index]-set_y)>y_err_distance))
			{
				DBG_MSG("dot out Top right range\n");
				result=-1;
			}
			else
			{
				DBG_MSG("dot in Top right range\n");
				result=0;
			}
			break;
		}
		case 2://Bot right
		{
			DBG_MSG("with=%d,hight=%d\n",IMAGE_WIDTH_E,IMAGE_HEIGHT_E);
			DBG_MSG("err_x=%d,err_y=%d\n",abs(cal->x[index]-IMAGE_WIDTH_E),abs(cal->y[index]-IMAGE_HEIGHT_E));
			if((abs(cal->x[index]-(IMAGE_WIDTH_E-set_x))>x_err_distance)|(abs(cal->y[index]-(IMAGE_HEIGHT_E-set_y))>y_err_distance))
			{
				DBG_MSG("dot out Bot right range\n");
				result=-1;
			}
			else
			{
				DBG_MSG("dot in Bot right range\n");
				result=0;
			}
			break;
		}		
		case 3://Bot left
		{
			if((abs(cal->x[index]-set_x)>x_err_distance)|(abs(cal->y[index]-(IMAGE_HEIGHT_E-set_y))>y_err_distance))
			{
				DBG_MSG("dot out Bot left range\n");
				result=-1;
			}
			else
			{
				DBG_MSG("dot in Bot left range\n");
				result=0;
			}
			break;
		}			
		case 4://center
		{
			if((abs(cal->x[index]-(IMAGE_WIDTH_E/2))>x_err_distance)|(abs(cal->y[index]-(IMAGE_HEIGHT_E/2))>y_err_distance))
			{
				DBG_MSG("dot out center range\n");
				result=-1;
			}
			else
			{
				DBG_MSG("dot in center range\n");
				result=0;
			}
			break;
		}
		default:
			DBG_MSG("index err\n");
			result=-1;
			break;
	}
	return result;
}

static int cl_open(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("cl_open \n");
	char *tsdevice = NULL;
	int ret=0;
	int pid;
	unsigned int i;
	int xres=IMAGE_WIDTH_E, yres=IMAGE_HEIGHT_E;
	if(_tscal_dll_open()==-1)
		return -1;
	printf("open /dev/event1\n");
	tscon=(ts_content*)sys_shmget(sizeof(ts_content) ,TSCAL_SHM_KEY);
	pid = fork();
	if(pid<0)
	{
		printf("fork ts listener erro\n");
		exit(0);
	}
	if(pid==0)
	{

		if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL ) {
			tscon->ts = cal_open(tsdevice,0);
		} else {
			tscon->ts = cal_open("/dev/event1", 0);
		}

		if (!tscon->ts) {
			printf("ts_open");
			ret = -1;
		}
		if (cal_config(tscon->ts)) {
			printf("ts_config");
			ret = -1;
		}
		//set top left dot
		do
		{
			DBG_MSG("set Top left\n");	
			cal_getsample (tscon->ts, &(tscon->cal), 0,50,50,	"Top left");
			DBG_MSG("\n DEBUG Top left : X = %4d Y = %4d\n", tscon->cal.x [0], tscon->cal.y[0]);
		}
		while(check_data(0, &(tscon->cal))==-1);
		tscon->touchflag = 1;
		tscon->index++;
		
		//set top right dot
		do
		{
			DBG_MSG("set top right\n");	
			cal_getsample (tscon->ts, &(tscon->cal), 1,xres-50,50,	"Top right");
			DBG_MSG("\n DEBUG Top right : X = %4d Y = %4d\n", tscon->cal.x [1], tscon->cal.y[1]);
		}
		while(check_data(1, &(tscon->cal))==-1);
		tscon->touchflag = 1;
		tscon->index++;

		//set Bot right dot
		do
		{
			DBG_MSG("set  Bot right\n");	
			cal_getsample (tscon->ts, &(tscon->cal), 2,xres-50,yres-50,"Bot right");
			DBG_MSG("\n DEBUG Bot right : X = %4d Y = %4d\n", tscon->cal.x [2], tscon->cal.y[2]);
		}
		while(check_data(2, &(tscon->cal))==-1);
		tscon->touchflag = 1;
		tscon->index++;

		//set Bot left dot
		do
		{
			DBG_MSG("set  Bot left\n");	
			cal_getsample (tscon->ts, &(tscon->cal), 3,50,yres-50,	"Bot left");
			DBG_MSG("\n DEBUG Bot left : X = %4d Y = %4d\n", tscon->cal.x [3], tscon->cal.y[3]);
		}
		while(check_data(3, &(tscon->cal))==-1);
		tscon->touchflag = 1;
		tscon->index++;
		
		//set center dot
		do
		{
			DBG_MSG("set center\n");	
			cal_getsample (tscon->ts, &(tscon->cal), 4,xres/2,yres/2,"Center");
			DBG_MSG("\n DEBUG center : X = %4d Y = %4d\n", tscon->cal.x [4], tscon->cal.y[4]);
		}
		while(check_data(4, &(tscon->cal))==-1);
		tscon->touchflag = 1;
		tscon->index++;

		cal_close(tscon->ts);
		exit(0);

	}

	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


static int cl_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int status;
	
	
	sys_shmrm(TSCAL_SHM_KEY);
	wait(&status);
	_tscal_dll_close;
	SWFEXT_FUNC_END();	
}


static int cl_get_samplepoint(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int p_index=0;
	int ret = -1;
	int x_pos=0,y_pos=0;
	p_index = Swfext_GetParamNum();
	if(tscon->touchflag)
	{
		tscon->touchflag = 0;
		ret = 0;
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


static int cl_set_calibratedata(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int i;
	int cal_fd;
	char cal_buffer[256];	
	char *calfile = NULL;
	
	if (cal_perform_calibration (&(tscon->cal))) {
		printf ("Calibration constants: ");
		for (i = 0; i < 7; i++) printf("%d ", tscon->cal.a [i]);
		printf("\n");
		if ((calfile = getenv("TSLIB_CALIBFILE")) != NULL) {
			cal_fd = open (calfile, O_CREAT | O_RDWR);
		} else {
			cal_fd = open ("/mnt/vram/pointercal", O_CREAT | O_RDWR | O_TRUNC);
		}
		
		sprintf (cal_buffer,"%d %d %d %d %d %d %d",
				tscon->cal.a[1], tscon->cal.a[2], tscon->cal.a[0],
				tscon->cal.a[4], tscon->cal.a[5], tscon->cal.a[3], tscon->cal.a[6]);
		write (cal_fd, cal_buffer, strlen (cal_buffer) + 1);
		fsync(cal_fd);
		close (cal_fd);
		i = 0;
		get_touch_calibrate_params();
	} else {
		printf("Calibration failed.\n");
		i = -1;
	}	
	Swfext_PutNumber(i);
	SWFEXT_FUNC_END();	
}



int swfext_calibrate_register(void)
{
	SWFEXT_REGISTER("cl_Open", cl_open);
	SWFEXT_REGISTER("cl_Close", cl_close);
	SWFEXT_REGISTER("cl_GetSamplePoint", cl_get_samplepoint);
	SWFEXT_REGISTER("cl_SetCalibrateData", cl_set_calibratedata);

	return 0;
	
}
#else
int swfext_calibrate_register(void)
{
	return 0;
}
#endif	/** MODULE_CONFIG_TS_CALIBRATE */
