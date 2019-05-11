#ifndef __EZ_AV_UTIL_H__
#define __EZ_AV_UTIL_H__

#ifndef WIN32
#include <stdint.h>
#else
typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef long int32_t;
typedef unsigned long uint32_t;

typedef __int64	int64_t;
typedef unsigned __int64 uint64_t;
#endif

#ifndef bswap_16
static uint16_t bswap_16(uint16_t x)
{
    return (x>>8) | (x<<8);
}

#endif

#ifndef bswap_32
static uint32_t bswap_32(uint32_t x)
{
	x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
	return (x>>16) | (x<<16);
}
#endif


#define AV_RN16(a) (*((const av_u16*)(a)))
#define AV_RN32(a) (*((const av_u32*)(a)))
#define AV_RN64(a) (*((const av_u64*)(a)))

#define AV_RB16(x)  ((((const av_u8*)(x))[0] << 8) | ((const av_u8*)(x))[1])
#define AV_RB24(x)  ((((const av_u8*)(x))[0] << 16) | (((const av_u8*)(x))[1]<<8) | ((const av_u8*)(x))[2])

//#define AV_RB16(x)    bswap_16(AV_RN16(x))
#define AV_RL16(x)    AV_RN16(x)

#define AV_RB32(x)    bswap_32(AV_RN32(x))
#define AV_RL32(x)    AV_RN32(x)

#define AV_RB64(x)    bswap_64(AV_RN64(x))
#define AV_RL64(x)    AV_RN64(x)



typedef char av_i8;
typedef unsigned char av_u8;

typedef short av_i16;
typedef unsigned short av_u16;

typedef long av_i32;
typedef unsigned long av_u32;

#ifdef WIN32
typedef __int64	av_i64;
typedef unsigned __int64 av_u64;
#else
typedef long long av_i64;
typedef unsigned long long av_u64;
#endif



#ifdef WIN32
#define AV_NOPTS_VALUE	(av_i64)(0x8000000000000000)
#else
#define AV_NOPTS_VALUE	(av_i64)(0x8000000000000000LL)
#endif
#define AV_TIME_BASE	1000000




#define AVERROR(x)  (-(x))

enum{
	EINVAL = 1,
	EIO,
	EDOM,
	ENOMEM,
	EILSEQ,
	ENOSYS,
	ENOENT,
	EPIPE,
	ENOFILE,
	EOPT,
	EAGAIN,
	EINTR,
};

#define AVERROR_UNKNOWN     AVERROR(EINVAL)  /**< unknown error */
#define AVERROR_IO          AVERROR(EIO)     /**< I/O error */
#define AVERROR_NUMEXPECTED AVERROR(EDOM)    /**< Number syntax expected in filename. */
#define AVERROR_INVALIDDATA AVERROR(EINVAL)  /**< invalid data found */
#define AVERROR_NOMEM       AVERROR(ENOMEM)  /**< not enough memory */
#define AVERROR_NOFMT       AVERROR(EILSEQ)  /**< unknown format */
#define AVERROR_NOTSUPP     AVERROR(ENOSYS)  /**< Operation not supported. */
#define AVERROR_NOENT       AVERROR(ENOENT)  /**< No such file or directory. */
#define AVERROR_EOF         AVERROR(EPIPE)   /**< End of file. */

//#define EZ_WIFI_DISPLAY
#ifdef EZ_WIFI_DISPLAY
#define av_malloc(size) (void *)ezMalloc(size)
#define av_mallocz(size) (void *)ezMallocz(size)
#define av_free(ptr)  ezFree(ptr)
#define av_freep(arg) do{\
        void **ptr= (void**)arg;\
        av_free(*ptr);\
        *ptr = NULL;\
    }while(0);
#define AV_LOG_DEBUG 0
#define av_dlog(x, fmt, arg...) EZLOG(fmt, ##arg)
#define av_log(x, y, fmt, arg...) EZLOG(fmt, ##arg)

#define ez_avlog(fmt, arg...) EZLOG(fmt, ##arg)

#else
#include <stdlib.h>
#define av_malloc(size) (void *)malloc(size)
#define av_mallocz(size) (void *)calloc(size, 1)
#define av_free(ptr)  do{\
    if(ptr)\
        free(ptr);\
    }while(0)
#define av_freep(arg) do{\
        void **ptr= (void**)arg;\
        av_free(*ptr);\
        *ptr = NULL;\
    }while(0);

#define AV_LOG_DEBUG 0
//#define av_dlog(x, fmt, arg...) printf(fmt, ##arg)
#define av_dlog(x, fmt, arg...) do{}while(0)

#define av_log(x, y, fmt, arg...) printf(fmt, ##arg)

#define ez_avlog(fmt, arg...) printf(fmt, ##arg)

#endif






#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))


#include "ez_mpegts.h"
void * packet_malloc(int size, unsigned int *pbus);
void packet_free(void * ptr);
void packet_freep(void *arg);
int av_new_packet(AVPacket *pkt, int size);
void av_init_packet(AVPacket *pkt);
void av_destruct_packet(AVPacket *pkt);
void av_free_packet(AVPacket *pkt);


#endif
