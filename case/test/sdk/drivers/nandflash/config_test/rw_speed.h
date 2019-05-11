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
#include <sys/stat.h>

#if 1
#define  Trace_MSG(fmt,msg...)  printf(fmt,##msg)
#else
#define  Trace_MSG(fmt,msg...)  do {} while (0)
#endif


int static w_speed(unsigned int flag,char * buf);
unsigned int static m_check(unsigned char * buf, unsigned int longth);
unsigned int static rd(unsigned char * sdfilename,unsigned char * nffilename,unsigned char * buf,unsigned int w_sum);
int static r_speed(unsigned int flag,char * buf);
int RW_Speed(unsigned int sum_param,char * param_buf);