
#ifndef  _EZ_GSTREAM_H_
#define  _EZ_GSTREAM_H_

#include "ez_stream.h"
#include "av_api.h" //for 'int64'


#define EZGSTREAM_DEF_BUFSIZE (65536)

typedef void (*ezGstreamMsgCb_t)(void *arg);
typedef int (*ezGstreamSeekCb_t)(void *stream, long long contentOffset);
typedef int (*ezGstreamReadCb_t)(void *stream, unsigned char *buf, long size);
typedef int (*ezGstreamWriteCb_t)(void *stream, long long contentOffset);
typedef int (*ezGstreamAbortCb_t)(void *stream);
typedef int (*ezGstreamPauseCb_t)(void);
typedef int (*ezGstreamResumeCb_t)(void);

int ezGstreamSetAvMsgCb(ezGstreamMsgCb_t streamAvStopMsgCb,
	ezGstreamMsgCb_t streamAvLengthMsgCb, ezGstreamMsgCb_t streamAvTimeMsgCb,
	ezGstreamMsgCb_t streamAvSeekMsgCb);

int ezGstreamStart(void *stream,
	ezGstreamSeekCb_t streamSeekCb, ezGstreamReadCb_t streamReadCb,
	ezGstreamPauseCb_t playerPauseCb, ezGstreamResumeCb_t playerResumeCb);

int ezGstreamStart2(void *stream,
	ezGstreamWriteCb_t streamWriteCb, ezGstreamAbortCb_t streamAbortCb,
	ezGstreamPauseCb_t playerPauseCb, ezGstreamResumeCb_t playerResumeCb);

int ezGstreamPlay(ezStreamOpType_t contentType, int seekable, long long contentLength);

int ezGstreamRead(long long contentOffset);

int ezGstreamPause(void);

int ezGstreamStop(void);

int ezGstreamSimplePlay(ezStreamOpType_t contentType, int seekable, long long contentLength);

int ezGstreamSimpleStop(void);

void ezGstreamInit(ezStreamOpMode_t mode);

int ezGstreamReinit(ezStreamOpMode_t mode);

#endif //_EZ_GSTREAM_H_

