
#ifndef  _EZ_STREAM_H_
#define  _EZ_STREAM_H_

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "av_api.h" //for 'int64'

//#include "ez_customer.h" //Exported to 'LSDK/sdk/include'.


typedef enum ezStreamMsg_e ezStreamMsg_t;
enum ezStreamMsg_e
{ 
	ezStreamMsgStop, 
	ezStreamMsgPlay,
	ezStreamMsgRead,
	ezStreamMsgPause,
	ezStreamMsgAvLength,
	ezStreamMsgAvTime,
	ezStreamMsgAvSeek,
	ezStreamMsgAvPause,
	ezStreamMsgAvResume,
};

typedef enum ezStreamOpMode_e ezStreamOpMode_t;
enum ezStreamOpMode_e
{ 
	ezStreamModeNone,
	ezStreamModeVideo,
	ezStreamModeHttp,
	ezStreamModeDlna,
	ezStreamModeMiracast,
	ezStreamModeAudio,
};

typedef enum ezStreamOpType_e ezStreamOpType_t;
enum ezStreamOpType_e
{ 
	ezStreamTypeNone,
	ezStreamTypeVideo,
	ezStreamTypeAudio,
};

typedef struct ezStream_s ezStream_t;
struct ezStream_s
{
	ezStreamOpMode_t opMode;
	ezStreamOpType_t opType;

	struct {
		void *handle; //File handler
		unsigned long long offset; //File position
		unsigned long long length; //File length
		long seekable; //File seekable indicator (1: yes, 0: no)

		//File I/O callback functions, 'LSDK/sdk/include/av_api.h:file_iocontext_s'
		void *(*open)(char *path, char *mode);
		int (*close)(void *stream);
		int (*read)(void *stream, unsigned char *buf, int buf_size);
		long (*seekSet)(void *stream, int64 offset);
		long (*seekCur)(void *stream, int64 offset);
		long (*seekEnd)(void *stream, int64 offset);
		int64 (*getFilePos)(void *stream);
		int64 (*getFileLen)(void *stream);
		int (*getSeekable)(void *stream);
		long (*read32)(void *stream, unsigned char *buf, unsigned long buf_size);
		long (*seekSet32)(void *stream, long offset);
		long (*seekCur32)(void *stream, long offset);
		long (*seekEnd32)(void *stream, long offset);
		long (*getFilePos32)(void *stream);
		long (*getFileLen32)(void *stream);
	} file;
	struct {
		long width;
		long height;

		unsigned long logBuf;
		unsigned long phyBuf;
		long bufSize;
	} player;
	struct {
		long fileLengthOk:1,
			fileSeekableOk:1,
			httpUrlOk:1,
			httpUserAgentOk:1,
			avMemErrIndicate:1,
			avFreezeIndicate:1,
			avFlowCtrlIndicate:1,
			xx:25;
	} status;
	struct {
		unsigned long nCacheMiss;
		unsigned long nCachePreload;
		unsigned long nFSeekSet;
		unsigned long nFSeekCur;
		unsigned long nFSeekEnd;
		unsigned long nFRead;
		unsigned long nFReadBlk;
	} stats;

	void *session; //EZ Stream session (owner).
	void (*msgCb)(void *session, ezStreamMsg_t msg, void *arg);

#if 0 //Obsoleted.
	void *wifiConnect;
#endif
};

typedef void (*ezStreamMsgCb_t)(void *session, ezStreamMsg_t msg, void *arg);


int ezStreamOnBusy(void);

int ezStreamOnExit(void);

int ezStreamOnAbort(void);

int ezStreamOnReset(void);

void *ezStreamFOpen(char *path, char *mode);

int ezStreamFClose(void *stream);

int ezStreamFRead(void *stream, unsigned char *data, int size);

long ezStreamFSeekSet(void *stream, int64 offset);

long ezStreamFSeekEnd(void *stream, int64 offset);

int64 ezStreamGetFilePos(void *stream);

int64 ezStreamGetFileLen(void *stream);

int ezStreamGetSeekable(void *stream);

void ezStreamMsg(ezStreamMsg_t msg, void *arg);

int ezStreamStart(ezStream_t *ezStream, void *session, ezStreamOpMode_t mode, ezStreamMsgCb_t msgCb);

void ezStreamStop(void);

int ezStreamPlay(ezStreamOpType_t contentType, int seekable, long long contentLength);

int ezStreamReplay(void);

void ezStreamQuit(void);

void ezStreamAbort(int reason);

int ezStreamSimplePlay(ezStreamOpType_t contentType, int seekable, long long contentLength);

void ezStreamSimpleStop(void);

void ezStreamInit(ezStream_t *ezStream, unsigned long logBuf, unsigned long phyBuf, long bufSize);

void ezStreamSimpleInit(void);


#endif //_EZ_STREAM_H_

