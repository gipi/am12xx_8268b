#ifndef SWF_SYSTEM_H
#define SWF_SYSTEM_H

#include "swf_types.h"
#include "act_plugin.h"
#include "fontdrv.h"

typedef struct
{
	int year;
	int month;
	int day;
	int wday;
} SWF_DATE;

typedef struct
{
	int hour;
	int minute;
	int second;
} SWF_TIME;

unsigned int swf_sys_getTickCount();
void   swf_sys_getTime(SWF_TIME * t);
void   swf_sys_getDate(SWF_DATE * d);
unsigned int swf_sys_getMilliseconds();
unsigned int swf_sys_createTimer(int interval, int id, void (*handler)(unsigned int));
void   swf_sys_deleteTimer(unsigned int id);

stream_input_t * create_local_input(char * filename);
stream_input_t * create_local_output(char * filename);
int swf_sys_remove_file(char *filename);

void * swf_sys_sem_create(int init);
void swf_sys_sem_pend(void * handle);
void swf_sys_sem_post(void * handle);
void swf_sys_sem_destroy(void * handle);
void swf_sys_sleep(int tick);

#endif
