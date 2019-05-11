
/* load photo lib */

#ifndef __LOAD_PHOTO_H__
#define __LOAD_PHOTO_H__

#ifndef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sys_buf.h"
#else
#define O_RDWR 3
#endif

#ifdef WIN32
#define DLL_EXPORT
#else
#define DLL_EXPORT	__attribute__ ((visibility("default")))
#endif

#include "jpegdecapi.h"
#include "ppapi.h"
#include "dwl.h"
#include "osapi.h"
#include "jpg_prog_dec.h"
#include "load_photo_api.h"
//#include "pngdec.h"
#include "jpeg_dec.h"
//#include "bmp_dec.h"
//#include "gif_lib.h"

#define LP_STREAM_BUF_SIZE (128 * 1024)

//#define LP_DEBUG 1

typedef struct tagLPSysBufInst
{
	int fd;
	struct mem_dev *arg;
}LP_SYS_BUF_INST;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif  //__LOAD_PHOTO_H__
