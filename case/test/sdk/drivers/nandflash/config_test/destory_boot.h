#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys_vram.h>
//#include <ctime.h>


#if 1
#define  Trace_MSG(fmt,msg...)  printf(fmt,##msg)
#else
#define  Trace_MSG(fmt,msg...)  do {} while (0)
#endif

int static m_eraseblks(unsigned int sum_blks,unsigned int flag,int *m_param);
int static m_phywriteblks(unsigned int sum_blks,unsigned int flag,int *m_param);
int static assign_destory(unsigned int sum_blks,unsigned int flag,int *m_param);
int static range_destory(unsigned int sum_blks,unsigned int flag,int *m_param);
int Destory_xx_block(unsigned int sum_param,char * param_buf);



