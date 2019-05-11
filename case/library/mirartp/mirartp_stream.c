#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "am_types.h"
#include <pthread.h>
#include "simple_rtp.h"
#include "mira_player.h"

typedef struct mirartpStreamPriv_s mirartpStreamPriv_t;
struct mirartpStreamPriv_s
{

	/** the simple rtp session */
	struct simple_rtp_session *session;
	
	int inited;
	pthread_mutex_t locker;
};

static mirartpStreamPriv_t miracastRtpStreamPriv;

long _mirartpStreamSeek(void *fp, int64 contentOffset)
{
	/**
	* a fake seek operation
	*/
	//printf("--_mirartpStreamSeek,0x%llx\n",contentOffset);
	return 0;
}

int _mirartpStreamRead(void *fp, unsigned char *buf, int size)
{
	mirartpStreamPriv_t *pStream = &miracastRtpStreamPriv;
	int readSize = 0;
	pthread_mutex_lock(&pStream->locker);
	readSize = rtp_session_fetch_data2(pStream->session,buf,size);
	pthread_mutex_unlock(&pStream->locker);
	return readSize;

}

void *_mirartpStreamOpen(char *path, char *mode)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	return (void *)&miracastRtpStreamPriv;
}

int _mirartpStreamClose(void *fp)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	return 0;
}

int64 _mirartpStreamTell(void *fp)
{
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	return 0;
}

int64 _mirartpStreamGetLen(void *fp)
{
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	return 0;
}

int _mirartpStreamGetSeekable(void *fp)
{
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	return 0;
}


EXPORT_SYMBOL
int mirartpStreamStart(int rtpPort,unsigned char *vHeap,unsigned int pHeap,unsigned int heapSize,int w,int h)
{
	int ret;

	/**
	********************************************************
	* start the RTP session.
	********************************************************
	*/
	if(miracastRtpStreamPriv.session == NULL){
		miracastRtpStreamPriv.session = rtp_session_open("0.0.0.0",rtpPort);
		if(miracastRtpStreamPriv.session == NULL){
			printf("[%s]--> session open error\n",__FUNCTION__);
			return -1;
		}
		pthread_mutex_init(&miracastRtpStreamPriv.locker, NULL);
	}

	/**
	********************************************************
	* start the stream.
	********************************************************
	*/
	ezMiraInit();
	if(ezMiraOpen()==NULL)
	{
		printf("[%s]--> player open error!\n",__FUNCTION__);
		goto _rtp_error1;
		
	}
	ezMiraSetFile();
	ezMiraSetParam(vHeap,pHeap,heapSize,w,h);
	printf("%s,%d,w=%d,h=%d\n",__FUNCTION__,__LINE__,w,h);

	/**
	* RTP do not support seek and file length.
	*/
	miracastRtpStreamPriv.inited = 1;
	ezMiraPlay();
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	return 0;
	
_rtp_error1:

	rtp_session_close(miracastRtpStreamPriv.session);
	miracastRtpStreamPriv.inited = 0;
	pthread_mutex_destroy(&miracastRtpStreamPriv.locker);

	return -1;
}

EXPORT_SYMBOL
int mirartpStreamStop()
{
	if(!miracastRtpStreamPriv.inited){
		return -1;
	}

	/** 
	* for rtp direct read by ezStream, if read blocked, we should 
	* have some way to wake up it.
	*/
	rtp_session_abort(miracastRtpStreamPriv.session);

	ezMiraClose();
	
	pthread_mutex_lock(&miracastRtpStreamPriv.locker);
	if(miracastRtpStreamPriv.session){
		rtp_session_close(miracastRtpStreamPriv.session);
	}
	miracastRtpStreamPriv.session = NULL;
	pthread_mutex_unlock(&miracastRtpStreamPriv.locker);
	pthread_mutex_destroy(&miracastRtpStreamPriv.locker);

	miracastRtpStreamPriv.inited = 0;
	
	return 0;
}

/**
* Open the socket before do anything else.
*/

EXPORT_SYMBOL
int mirartpStreamInit(int rtpPort)
{
	int ret;

	/**
	********************************************************
	* start the RTP session.
	********************************************************
	*/
	if(miracastRtpStreamPriv.session == NULL){
		miracastRtpStreamPriv.session = rtp_session_open("0.0.0.0",rtpPort);
		if(miracastRtpStreamPriv.session == NULL){
			printf("[%s]--> session init error\n",__FUNCTION__);
			return -1;
		}
		pthread_mutex_init(&miracastRtpStreamPriv.locker, NULL);
		miracastRtpStreamPriv.inited = 1;
	}

	return 0;
	
}


