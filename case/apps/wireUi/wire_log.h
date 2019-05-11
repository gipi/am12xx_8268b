#ifndef __WIRE_LOG_H
#define __WIRE_LOG_H

#include <stdio.h>
#include "ezcast_public.h"

#if defined(MODULE_CONFIG_DISABLE_SWF_ENGINE) && MODULE_CONFIG_DISABLE_SWF_ENGINE!=0
	#define EZWIRE_SNOR_8M	1
	#define EZWIRE_SWF_MALLOC	0
#else
	#define EZWIRE_SNOR_8M	1
	#define EZWIRE_SWF_MALLOC	1
#endif

#define Debug 1
#if Debug
#define WLOGV(fmt, args...)   printf("V:<%s,%d> "fmt"\n",__FUNCTION__,__LINE__, ##args)
#define WLOGD(fmt, args...)   printf("D:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define WLOGI(fmt, args...)   printf("I:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define WLOGW(fmt, args...)   printf("W:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define WLOGE(fmt, args...)   printf("E:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define WLOGT(fmt, args...) do { struct timespec __t; clock_gettime(CLOCK_MONOTONIC, &__t); printf("<%6lu.%03lu> [%s:%d] "fmt,__t.tv_sec,__t.tv_nsec/1000000,__FUNCTION__,__LINE__,##args); } while (0)
#else
#define WLOGV(fmt, args...)   
#define WLOGD(fmt, args...)  
#define WLOGI(fmt, args...)   
#define WLOGW(fmt, args...)  
#define WLOGE(fmt, args...)
#define WLOGT(fmt, args...)
#endif

#define EZCASTLOG(fmt, args...) do { struct timespec __t; clock_gettime(CLOCK_MONOTONIC, &__t); printf("<%6lu.%03lu> [%s:%d] "fmt,__t.tv_sec,__t.tv_nsec/1000000,__FUNCTION__,__LINE__,##args); } while (0)
#define EZCASTWARN(fmt, args...) do { struct timespec __t; clock_gettime(CLOCK_MONOTONIC, &__t); printf("<%6lu.%03lu> WARNNING!!! [%s:%d] "fmt,__t.tv_sec,__t.tv_nsec/1000000,__FUNCTION__,__LINE__,##args); } while (0)

#endif
