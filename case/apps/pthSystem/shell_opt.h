#ifndef __SHELL_OPT_H__
#define __SHELL_OPT_H__

#include <stdint.h>
#include <sys/time.h>

#define CMD_MAX_LEN		256


#define	LOG_DEBUG	0
#define	LOG_NORMAL	1
#define	LOG_ERROR	2
#define	LOG_QUEIT	3

#define LOG_LEVEL	LOG_NORMAL
// The start time 01/01/2014 00:00:00 is 1388534400
#if (LOG_LEVEL == LOG_QUEIT)
	#define EZCASTDEBUG(fmt, args...) {}
	#define EZCASTLOG(fmt, args...) {}
	#define EZCASTERR(fmt, args...)	{}
#elif (LOG_LEVEL == LOG_ERROR)
	#define EZCASTDEBUG(fmt, args...) {}
	#define EZCASTLOG(fmt, args...) {}
	#define EZCASTERR(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL]<ERR> [%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
#elif (LOG_LEVEL == LOG_NORMAL)
	#define EZCASTDEBUG(fmt, args...) {}
	#define EZCASTLOG(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL][%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
	#define EZCASTERR(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL]<ERR> [%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
#else
	#define EZCASTDEBUG(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL]<DEBUG> [%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
	#define EZCASTLOG(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL][%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
	#define EZCASTERR(fmt, args...) do { struct timeval _tv; gettimeofday(&_tv,NULL); printf("[SHELL]<ERR> [%6lu.%06lu] [%s:%d] "fmt,_tv.tv_sec-1388534398,_tv.tv_usec,__FUNCTION__,__LINE__,##args); } while (0)
#endif


#endif
