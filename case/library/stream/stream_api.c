#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"
#include "stream_api.h"
#include "am_types.h"
#include <pthread.h>
#include "fast_memcpy.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


#if 1 /* EZStream porting */
#include "ez_stream.h"
#include "ez_gstream.h"

#define STREAM_DEBUG_EN  1
#define SEEK_BUFFER_SIZE 4096
static char seek_buf[SEEK_BUFFER_SIZE];
/**
* callback function prototype for the player.
*/
typedef int (*player_cb)(void);

#define N_AMSTREAM_MAX 10
#define I_AMSTREAM_INIT (0)
#define I_AMSTREAM_LAST (N_AMSTREAM_MAX-1)
typedef struct amStreamPriv_s amStreamPriv_t;
struct amStreamPriv_s
{
	char *file;

	/** lower level stream like http,ftp,file etc. */
	stream_t *amStream[N_AMSTREAM_MAX];
	int amStreamIdx;
	int amStreamNum;
	off_t amStreamOffset;

	/** the ez stream instance */
	ezStream_t ezStream;

	/** the pause and resume callback*/
	int (*player_pause)(void);
	int (*player_resume)(void);
};
static amStreamPriv_t amStreamPriv;

static int am_stream_ezseek(void *stream, long long contentOffset)
{
	int i, iFree = -1, iSeekFwd = -1, format;
	stream_t *amStream = NULL;
	off_t amContentOffset, maxContentOffset = 0;

	for (i = 0; i < amStreamPriv.amStreamNum; i++)
	{
		if (amStreamPriv.amStream[i] == NULL)
		{
			if (iFree < 0) iFree = i;
		}
		else
		{
			stream_seek(amStreamPriv.amStream[i], (off_t)contentOffset);
			amContentOffset = stream_tell(amStreamPriv.amStream[i]);
			if (amContentOffset == (off_t)contentOffset)
			{
				amStreamPriv.amStreamIdx = i;
				amStreamPriv.amStreamOffset = amContentOffset;
				return 0;
			}
			else if (amContentOffset < (off_t)contentOffset)
			{
				if (amContentOffset > maxContentOffset)
				{
					amStreamPriv.amStreamIdx = i;
					amStream = amStreamPriv.amStream[i];
					maxContentOffset = amContentOffset;
				}
			}
			//printf("%s::%d::Seek(%lld) @ AM stream[.%d/%lld].\n",
			//		__FUNCTION__, __LINE__, contentOffset, i, amContentOffset);
		}
	}

	if (amStream == NULL)
	{
//FIXME: Is there re-entrance problem???
		if ((amStream = open_stream(amStreamPriv.file, NULL, &format)) != NULL)
		{
			if (amStreamPriv.amStreamNum > 0 && iFree >= 0)
			{ //Re-open stream.
				i = amStreamPriv.amStreamIdx = iFree;
				amStreamPriv.amStream[i] = amStream;
			}
			else if (amStreamPriv.amStreamNum < N_AMSTREAM_MAX)
			{ //New stream.
				i = amStreamPriv.amStreamIdx = amStreamPriv.amStreamNum;
				amStreamPriv.amStream[i] = amStream;
				amStreamPriv.amStreamNum++;
				//printf("%s::%d::Open AM stream[.%d] @ content offset(%lld).\n",
				//		__FUNCTION__, __LINE__, i, contentOffset);
			}
			else 
			{ //Too many streams.
				i = amStreamPriv.amStreamIdx = I_AMSTREAM_LAST;
				if (amStreamPriv.amStream[i] != NULL)
				{
					stream_reset(amStreamPriv.amStream[i]);
					free_stream(amStreamPriv.amStream[i]);
					printf("%s::%d::Re-use AM stream[.%d] @ content offset(%lld).\n",
							__FUNCTION__, __LINE__, i, contentOffset);
				}
				amStreamPriv.amStream[i] = amStream;
			}
		}
	}

	//stream_seek() failed, probably non-seekable stream.
	if (amStream != NULL)
	{
		long long readSize = (long long)stream_tell(amStream);
		if (readSize < contentOffset)
		{
			char buf[EZGSTREAM_DEF_BUFSIZE];
			int size, ret;
			size = sizeof(buf);
			if (contentOffset < readSize + (long long)size)
			{
				size = (int)(contentOffset - readSize);
			}
			ret = stream_read(amStream, buf, size);
			if (ret <= 0)
			{
				stream_reset(amStream);
				free_stream(amStream);
				amStreamPriv.amStream[amStreamPriv.amStreamIdx] = NULL;
				return -1;
			}
			readSize += (long long)ret;
		}
		if (readSize == contentOffset)
		{
			//if (!stream_seekable(amStream))
			//{
			//	printf("%s::%d::Non-seekable stream_tell(%lld)!!\n",
			//			__FUNCTION__, __LINE__, (long long)stream_tell(amStream));
			//}
			amStreamPriv.amStreamOffset = (off_t)contentOffset;
			return 0;
		}
		return 1; //Tell ezGstream to continue seek until 'contentOffset'.
	}
	return -1;
}

static int am_stream_ezread(void *stream, unsigned char *buf, long size)
{
	int i = amStreamPriv.amStreamIdx;
	stream_t *amStream = amStreamPriv.amStream[i];
	if (amStream != NULL)
	{
		int ret;
		off_t amStreamOffset = stream_tell(amStream);
		if (amStreamPriv.amStreamOffset != amStreamOffset)
		{
			//printf("%s::%d::Mis-match offset(%lld/%lld) @ AM stream[.%d]!!\n",
			//		__FUNCTION__, __LINE__,
			//		(long long)amStreamPriv.amStreamOffset, 
			//		(long long)amStreamOffset, i);
		}
//FIXME: Is there re-entrance problem???
		ret = stream_read(amStream, (char *)buf, (int)size);
		if (ret > 0)
		{
			off_t amStreamLen = stream_fsize(amStream);
			if (amStreamLen > 0 && amStreamLen < (amStreamOffset + (off_t)ret))
			{
				printf("%s::%d::Out of range content @ AM stream[.%d]!!\n",
						__FUNCTION__, __LINE__, i);
			}
			amStreamPriv.amStreamOffset += (off_t)ret;
			return ret;
		}
		else
		{
			stream_reset(amStream);
			free_stream(amStream);
			amStreamPriv.amStream[i] = NULL;
			printf("%s::%d::Read content failed @ AM stream[.%d]!!\n",
					__FUNCTION__, __LINE__, i);
		}
	}
	return -1;
}

static int am_player_ezpause(void)
{
	if (amStreamPriv.player_pause)
	{
		return amStreamPriv.player_pause();
	}
	return 0;
}

static int am_player_ezresume(void)
{
	if (amStreamPriv.player_resume)
	{
		return amStreamPriv.player_resume();
	}
	return 0;
}

EXPORT_SYMBOL
void am_cache_setavseeking(long indicate)
{
	ezCacheSetAvSeeking(indicate);
}

EXPORT_SYMBOL
void am_cache_setdatapos(long long file_pos)
{
	ezCacheSetDataPos(file_pos);
}

EXPORT_SYMBOL
void am_cache_avseekplay(void)
{
	ezCacheAvSeekPlay();
}

EXPORT_SYMBOL
void* am_stream_open(const char *filename,char**options,int*file_format)
{
	int ret, i = I_AMSTREAM_INIT;
	int seekable;
	long long contentLen;

	if (amStreamPriv.amStream[i] != NULL)
	{
		printf("[%s]--> Should close previous stream before open a new one\n",__FUNCTION__);
		return NULL;
	}

	/** start the low level streaming protocol */
	amStreamPriv.amStream[i] = open_stream(filename,options,file_format);
	if (amStreamPriv.amStream[i] == NULL)
	{
		printf("[%s]--> Open Low Level Stream Error!\n",__FUNCTION__);
		return NULL;
	}
	amStreamPriv.amStreamIdx = i;
	amStreamPriv.amStreamNum = 1;

	amStreamPriv.player_pause = NULL;
	amStreamPriv.player_resume = NULL;

	/**
	* a temporary patch for some HTTP server.
	*/
	seekable = stream_seekable(amStreamPriv.amStream[i]);
	contentLen = (long long)stream_fsize(amStreamPriv.amStream[i]);
/*****marked by chenshouhui ****/
#if 0
	if(seekable && contentLen==0){
		free_stream(amStreamPriv.amStream[i]);
		amStreamPriv.amStream[i] = NULL;
		printf("[%s]--> Warning Server seekable but content length is zero!\n",__FUNCTION__);
		return NULL;
	}
	
#endif
	ezStreamInit(&amStreamPriv.ezStream, 0, 0, 0);

	ezGstreamInit(ezStreamModeDlna);

	ret = ezGstreamStart(&amStreamPriv.ezStream, am_stream_ezseek, am_stream_ezread,
				am_player_ezpause, am_player_ezresume);
	if (ret == 0)
	{
		printf("--------seekable : %d\n", seekable);
		printf("--------contentLen : %lld\n", contentLen);
		ezGstreamPlay(ezStreamTypeNone, seekable, contentLen);
	}
	else
	{
		stream_reset(amStreamPriv.amStream[i]);
		free_stream(amStreamPriv.amStream[i]);
		amStreamPriv.amStream[i] = NULL;
		printf("[%s]--> ezGstreamStart error!\n",__LINE__);
		return NULL;
	}

	if (amStreamPriv.file != NULL)
	{
		free(amStreamPriv.file);
	}
	amStreamPriv.file = strdup(filename);

	return ezStreamFOpen(NULL, NULL);
}

EXPORT_SYMBOL
void am_stream_close(void *s)
{
	int i;

	/** stop the EZStream first */
	ezGstreamStop();
	ezStreamFClose(s);

	/** release the low level protocol */
	for (i = 0; i < amStreamPriv.amStreamNum; i++)
	{
		if (amStreamPriv.amStream[i] != NULL)
		{
			stream_reset(amStreamPriv.amStream[i]);
			free_stream(amStreamPriv.amStream[i]);
			amStreamPriv.amStream[i] = NULL;
		}
	}
	amStreamPriv.amStreamIdx = amStreamPriv.amStreamNum = 0;

	amStreamPriv.player_pause = NULL;
	amStreamPriv.player_resume = NULL;

	if (amStreamPriv.file != NULL)
	{
		free(amStreamPriv.file);
		amStreamPriv.file = NULL;
	}
	return;
}

EXPORT_SYMBOL
void am_stream_stop(void *s)
{
	ezGstreamStop();
	return;
}

EXPORT_SYMBOL
int am_stream_read(void *s,char* mem,int total)
{
	int nr;
	nr = ezStreamFRead(s, (unsigned char *)mem, total);
	return nr;
}


EXPORT_SYMBOL
int am_stream_seek_set(void*s,stream_i64 pos)
{
	int ret;
	ret = ezStreamFSeekSet(s, pos);
	return ret;
}

EXPORT_SYMBOL
int am_stream_seek_end(void *s,stream_i64 offset)
{
	int ret;
	ret = ezStreamFSeekEnd(s, offset);
	return ret;
}

EXPORT_SYMBOL
int am_stream_seek_cur(void *s,stream_i64 pos)
{
	stream_i64 cur;
	int ret;
	cur = (stream_i64)ezStreamGetFilePos(s);
	cur += pos;
	ret = ezStreamFSeekSet(s, cur);
	return ret;
}

EXPORT_SYMBOL
stream_i64 am_stream_get_filesize(void *s)
{
	stream_i64 size;
	size = ezStreamGetFileLen(s);
	return size;
}

EXPORT_SYMBOL
int am_stream_seekable(void *s)
{
	return ezStreamGetSeekable(s);
}

EXPORT_SYMBOL
off_t am_stream_get_pos(void *s)
{
	off_t pos;
	pos = (off_t)ezStreamGetFilePos(s);
	return pos;
}

EXPORT_SYMBOL
void am_stream_set_player_pause_cb(player_cb player_pause)
{
	if(player_pause){
		amStreamPriv.player_pause = player_pause;
	}
}


EXPORT_SYMBOL
void am_stream_set_player_resume_cb(player_cb player_resume)
{
	if(player_resume){
		amStreamPriv.player_resume= player_resume;
	}
}

/**
* for the ugly audio interface.
*/

EXPORT_SYMBOL
void* am_stream_audio_open(const char *filename,char**options,int*file_format)
{
	return am_stream_open(filename,options,file_format);
}

EXPORT_SYMBOL
inline int am_stream_audio_read(void *s,char* mem,int total)
{
	return am_stream_read(s,mem,total);
}

EXPORT_SYMBOL
int am_stream_audio_seek_set(void *s,long pos)
{
	return am_stream_seek_set(s,(stream_i64)pos);
}

EXPORT_SYMBOL
int am_stream_audio_seek_cur(void *s,long pos)
{
	return am_stream_seek_cur(s,(stream_i64)pos);
}

EXPORT_SYMBOL
int am_stream_audio_seek_end(void *s,long offset)
{
	return am_stream_seek_end(s,(stream_i64)offset);
}

EXPORT_SYMBOL
inline off_t am_stream_audio_get_pos(void *s)
{
	return am_stream_get_pos(s);
}

EXPORT_SYMBOL
int am_stream_audio_get_filesize(void *s)
{
	return (int)am_stream_get_filesize(s);
}

EXPORT_SYMBOL
void am_stream_audio_close(void *s)
{
	am_stream_close(s);
}

static int am_stream_internal_raw_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	
	nr = stream_read(si->streaming,mem,total);

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif

	return nr;
}


static int am_stream_read_internal(void *s,off_t remain)
{
	int rd;

	if(remain < 0){
		
	#ifdef STREAM_DEBUG_EN
		printf("%s error: remain < 0 \n",__FUNCTION__);
	#endif
	
		return -1;
	}
	
	while(remain >0){
		int should_rd;

		if(remain > SEEK_BUFFER_SIZE){
			should_rd = SEEK_BUFFER_SIZE;
		}
		else{
			should_rd = remain;
		}

		rd = am_stream_internal_raw_read(s,seek_buf,should_rd);
		if(rd < 0){

#ifdef STREAM_DEBUG_EN
			printf("%s error:seek error happends\n",__FUNCTION__);
#endif
			return -1;
		}
		else if(rd ==0){
			
#ifdef STREAM_DEBUG_EN
			printf("%s: Already seek to end\n",__FUNCTION__);
#endif
			return 0;			
		}
		remain -= rd;
	}

	return 0;
}

EXPORT_SYMBOL
int am_stream_seek_internal(void*s,off_t pos)
{
	int err;
	off_t internal_pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t after;
	int file_format;
	int rd;

	internal_pos = (off_t)pos;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	err = stream_seek(si->streaming,internal_pos);
	if(err <=0){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Internal Seek Error [%d]!\n",err);
	#endif
	
		return -1;
	}

	after = stream_tell(si->streaming);
	if(after != internal_pos){

		if(internal_pos > after){
			if(am_stream_read_internal(s,internal_pos-after)>=0){
				
			#ifdef STREAM_DEBUG_EN
				printf("seek1 to %llx,succeed=%d\n",stream_tell(si->streaming),stream_tell(si->streaming)==internal_pos);
			#endif
				return 0;
			}
			else{
				
			#ifdef STREAM_DEBUG_EN
				printf("seek1 to %d error\n",stream_tell(si->streaming));
			#endif	
			
			}
		}
		
		/**
		* If the position after seeking is not what we expected,
		* let's do all from scratch.
		*/
		stream_reset(si->streaming);
		free_stream(si->streaming);
		si->streaming = NULL;
		si->streaming = open_stream(si->url,NULL,&file_format);
		if(si->streaming == NULL){	
		
		#ifdef STREAM_DEBUG_EN
			printf("stream seek error: Stream reopen error!\n");
		#endif

			return -1;
		}
		if(am_stream_read_internal(s,internal_pos)>=0){
				
			#ifdef STREAM_DEBUG_EN
				printf("seek2 to %llx,succeed=%d\n",stream_tell(si->streaming),stream_tell(si->streaming)==internal_pos);
			#endif
			
				err = 1;
		}
		else{
			err = 0;
		}
		/**
		* Set err to correct value.
		*/
				
	}

	if(err > 0){
		err = 0;
	}
	else{
		
	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Return error %d\n",err);
	#endif
	
		err = -1;
	}

	return err;
}

EXPORT_SYMBOL
void* am_ota_open(const char *filename,char**options,int*file_format)
{
	struct wrap_stream_t *s=NULL;

	s = (struct wrap_stream_t *)calloc(1,sizeof(struct wrap_stream_t));
	if(s == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error--1: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}

	s->url = strdup(filename);
	if(s->url == NULL){
		free(s);
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error--2: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}
	
	s->streaming = open_stream(filename,options,file_format);
	if(s->streaming == NULL){
		free(s->url);
		free(s);

	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}
	am_stream_seek_internal(s,0);
	return (void *)s;
}

EXPORT_SYMBOL
inline int am_ota_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	
	nr = stream_read(si->streaming,mem,total);

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif

	return nr;
}

EXPORT_SYMBOL
inline off_t am_ota_get_pos(void *s)
{
	off_t pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream get pos error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	pos = stream_tell(si->streaming);

	return pos;

	//return stream_tell((stream_t *)s);
}

EXPORT_SYMBOL
stream_i64 am_ota_get_filesize(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return 0;
	}
	
	return stream_fsize(si->streaming);
}


EXPORT_SYMBOL
void am_ota_close(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream close error: Pointer NULL!\n");
	#endif
	
		return;
	}

	free_stream(si->streaming);
	free(si->url);
	free(si);
	si = NULL;
}

/*
static void dump_read_data(const unsigned char *pdata,unsigned int len){
	unsigned int tmp=0;
	while(len--){
		if(tmp%16 == 0){
			printf("\n%02x",*(pdata+tmp));
		}else{
			printf(" %02x",*(pdata+tmp));	
		}
		tmp++;
	}
	printf("\n");
}
*/
#else

#define BUFFERED_STREAM
#define STREAM_DEBUG_EN
#define NO_DATA_SLEEP_INTERVAL 1
#define NO_DATA_TIMEOUT 30
//#define NET_SPEED_STATISTIC_EN

#ifdef NET_SPEED_STATISTIC_EN
#include <sys/time.h>
#endif

#define SEEK_BUFFER_SIZE 4096
static char seek_buf[SEEK_BUFFER_SIZE];

static int am_stream_read_internal(void *s,off_t remain);
static int am_stream_seek_internal(void*s,off_t pos);


static void * fast_memcpy_local(void * dest,const void *src,unsigned int count)
{
	char *d = (char *) dest, *s = (char *) src;

	if( (((int) d | (int) s) & 3) == 0 ) {
		while(count >= 4) {
			*(int *) d = *(int *)s;
			s += 4;
			d += 4;
			count -= 4;
		}
		goto copy_residual;
	} else {
copy_residual:
		while (count--)
			*d++ = *s++;
	}

	return dest;
}


#define fast_memcpy fast_memcpy_local


#ifdef BUFFERED_STREAM

#define AM_STREAM_BUFFER_SIZE (30*1024*1024)
#define AM_STREAM_BUFFER_READ_SIZE (24*1024)
#define AM_STREAM_BUFFER_INOUT_GAP (1*1024)

struct wrap_stream_t{
	stream_t *streaming;
	char *url;

	char *buf;
	char *readbuf;
	/**
	* the two pointers indicate the read/write position.
	*/
	int inbuf,outbuf;
	int bufsize;

	/**
	* operation for the read buffer.
	*/
	int read_start,read_end;
	int read_bufsize;
	/**
	* locker to protect the buffer access.
	*/
	pthread_mutex_t buflock;
	pthread_mutex_t readbuflock;

	/**
	* thread identification of the stream buffering.
	*/
	pthread_t buf_tid;

	int rollover;
};

/**
* the only function of the stream thread is to read data from
* a stream to the buffer.
*/
static void stream_thread_release_lock(void *arg)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)arg;

	if(si == NULL){
		return;
	}
	
	pthread_mutex_unlock(&si->readbuflock);
	pthread_mutex_unlock(&si->buflock);
	
}

static void *stream_thread(void * arg)
{
	struct wrap_stream_t *s=(struct wrap_stream_t *)arg;
	int rdsize,shouldsize;
	int tmpsize;
	
#ifdef NET_SPEED_STATISTIC_EN
	struct timeval tv;
	unsigned int milisec_begin,milisec_end;
	unsigned int total_bytes=0,total_time=0;
	unsigned int cnt=0;
#endif
	
	if(s == NULL){
		goto STREAM_THREAD_OUT;
	}

	pthread_cleanup_push(stream_thread_release_lock,s);
	
	while(1){
		/**
		* set cancellation point.
		*/
		pthread_testcancel();

		/**
		* check if we already have data. If have, just copy to the 
		* main buffer or we have to read from the stream.
		*/
		pthread_mutex_lock(&s->readbuflock);
		if(s->read_end <= s->read_start){
			s->read_end = 0;
			s->read_start = 0;
			
		#ifdef NET_SPEED_STATISTIC_EN	
			gettimeofday(&tv,NULL);
			milisec_begin = (unsigned int)(tv.tv_sec*1000 + tv.tv_usec/1000);
		#endif

		if(s->streaming){
			rdsize = stream_read(s->streaming,s->readbuf,s->read_bufsize);
		}
		else{
			rdsize = 0;
		}

		#ifdef NET_SPEED_STATISTIC_EN	
			gettimeofday(&tv,NULL);
			milisec_end = (unsigned int)(tv.tv_sec*1000 + tv.tv_usec/1000);
			if(rdsize > 0){
				total_bytes += rdsize;
				total_time += milisec_end - milisec_begin;
				cnt++;
				if(cnt >= 10){
					int quotient;
					unsigned int total_bits;
					int i;
					
					cnt = 0;
					total_bits = total_bytes*8*1000;
					quotient = 0;

					printf("speed:");

					for(i=0;i<3;i++){
						quotient = total_bits/total_time;
						printf("%d",quotient);
						if(i==0){
							printf(".");
						}
						total_bits -= quotient*total_time;
						total_bits *= 10;
					}

					printf("bps \n");

					total_bytes = 0;
					total_time = 0;
				}
			}
		#endif
		
			if(rdsize > 0){
				s->read_end += rdsize;
			}
			else{
				pthread_mutex_unlock(&s->readbuflock);
				sleep(1);
				continue;
			}
		}
		pthread_mutex_unlock(&s->readbuflock);

		
		/**
		* begin fill buffer.
		*/
		pthread_mutex_lock(&s->buflock);
		pthread_mutex_lock(&s->readbuflock);
		
		tmpsize = s->read_end - s->read_start;
		if(tmpsize <=0 ){
			/**
			* no data currently,just continue.
			*/
			pthread_mutex_unlock(&s->readbuflock);
			pthread_mutex_unlock(&s->buflock);
			continue;
		}
		
		if(s->inbuf >= s->outbuf){
			if((s->inbuf + tmpsize)>=s->bufsize){
				shouldsize = s->bufsize - s->inbuf;
			}
			else{
				shouldsize = tmpsize;
			}

			if(((s->inbuf + shouldsize)%s->bufsize)==s->outbuf){
				shouldsize--;
			}

			if(shouldsize > 0){
				int bk;
				fast_memcpy((void *)(s->buf+s->inbuf),(void *)(s->readbuf+s->read_start),shouldsize);
				s->read_start += shouldsize;
				
				bk = s->inbuf;
				s->inbuf = (s->inbuf + shouldsize)%s->bufsize;
				if(bk > s->inbuf){
					s->rollover = 1;
				}
			}

		}
		else{
			if((s->inbuf + AM_STREAM_BUFFER_INOUT_GAP) < s->outbuf){
				if((s->inbuf + tmpsize) >= (s->outbuf-AM_STREAM_BUFFER_INOUT_GAP)){
					shouldsize = s->outbuf - AM_STREAM_BUFFER_INOUT_GAP - s->inbuf;
				}
				else{
					shouldsize = tmpsize;
				}
				
				if(shouldsize > 0){
					int bk;
					fast_memcpy((void *)(s->buf+s->inbuf),(void *)(s->readbuf+s->read_start),shouldsize);
					s->read_start += shouldsize;
				
					bk = s->inbuf;
					s->inbuf = (s->inbuf + shouldsize)%s->bufsize;
					if(bk > s->inbuf){
						s->rollover = 1;
					}
				}
			}
		}
		pthread_mutex_unlock(&s->readbuflock);
		pthread_mutex_unlock(&s->buflock);
		
	}

	pthread_cleanup_pop(1);

STREAM_THREAD_OUT:
	pthread_exit(NULL);
	
}

EXPORT_SYMBOL
void* am_stream_open(const char *filename,char**options,int*file_format)
{
	struct wrap_stream_t *s=NULL;
	int err,res;
	pthread_attr_t attr;

	s = (struct wrap_stream_t *)calloc(1,sizeof(struct wrap_stream_t));
	if(s == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory1!\n");
	#endif
	
		return NULL;
	}

	s->url = strdup(filename);
	if(s->url == NULL){
		free(s);
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory2!\n");
	#endif
	
		return NULL;
	}

	/**
	* malloc the read buffer.
	*/
	s->read_bufsize = AM_STREAM_BUFFER_READ_SIZE;
	s->readbuf = (char *)malloc(s->read_bufsize+1);
	if(s->readbuf == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory3_1 When malloc %d bytes\n",s->read_bufsize);
	#endif
		free(s->url);
		free(s);
		return NULL;
	}

	/**
	* Malloc the streaming buffer.
	*/
	s->bufsize = AM_STREAM_BUFFER_SIZE;
	s->buf = (char *)malloc(s->bufsize);
	if(s->buf == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory3_2 When malloc %d bytes\n",s->bufsize);
	#endif
		free(s->readbuf);
		free(s->url);
		free(s);
		return NULL;
	}
	s->inbuf = 0;
	s->outbuf = 0;
	s->rollover = 0;
	s->read_start=0;
	s->read_end = 0;
	pthread_mutex_init(&s->buflock,NULL);
	pthread_mutex_init(&s->readbuflock,NULL);

	s->streaming = open_stream(filename,options,file_format);
	if(s->streaming == NULL){
		free(s->url);

		pthread_mutex_lock(&s->readbuflock);
		free(s->readbuf);
		s->readbuf = NULL;
		pthread_mutex_unlock(&s->readbuflock);
		pthread_mutex_destroy(&s->readbuflock);

		pthread_mutex_lock(&s->buflock);
		free(s->buf);
		s->buf = NULL;
		pthread_mutex_unlock(&s->buflock);
		pthread_mutex_destroy(&s->buflock);

		free(s);

	#ifdef STREAM_DEBUG_EN
		printf("stream open error:HTTP open error!\n");
	#endif
	
		return NULL;
	}

	/**
	* Create the buffering thread.
	*/
	res = pthread_attr_init(&attr);
	if(res !=0 ){
		printf("stream open thread attr init error\n");
	}
	else{
		res = pthread_attr_setschedpolicy(&attr,SCHED_RR);
		if(res !=0){
			pthread_attr_destroy(&attr);
		}
	}

	if(res !=0){
		err = pthread_create(&s->buf_tid,NULL,stream_thread,(void *)s);
	}
	else{
		err = pthread_create(&s->buf_tid,&attr,stream_thread,(void *)s);
		pthread_attr_destroy(&attr);
	}
	
	if(err != 0){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Stream Thread Create Error!\n");
	#endif
	
		free_stream(s->streaming);
		s->streaming = NULL;
		free(s->url);
		
		pthread_mutex_lock(&s->readbuflock);
		free(s->readbuf);
		s->readbuf = NULL;
		pthread_mutex_unlock(&s->readbuflock);
		pthread_mutex_destroy(&s->readbuflock);

		pthread_mutex_lock(&s->buflock);
		free(s->buf);
		s->buf = NULL;
		pthread_mutex_unlock(&s->buflock);
		pthread_mutex_destroy(&s->buflock);
		
		free(s);
	
		return NULL;
	}

	return (void *)s;
}

EXPORT_SYMBOL
void am_stream_close(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream close error: Pointer NULL!\n");
	#endif
	
		return;
	}

	/**
	* terminate the streaming thread.
	*/
	pthread_cancel(si->buf_tid);
	pthread_join(si->buf_tid, NULL);
	free_stream(si->streaming);
	si->streaming = NULL;
	free(si->url);

	pthread_mutex_lock(&si->readbuflock);
	free(si->readbuf);
	si->readbuf = NULL;
	pthread_mutex_unlock(&si->readbuflock);
	pthread_mutex_destroy(&si->readbuflock);

	pthread_mutex_lock(&si->buflock);
	free(si->buf);
	si->buf = NULL;
	pthread_mutex_unlock(&si->buflock);
	pthread_mutex_destroy(&si->buflock);
	
	free(si);
	si = NULL;
}


EXPORT_SYMBOL
int am_stream_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	int remain;
	int timeout_max = NO_DATA_TIMEOUT/NO_DATA_SLEEP_INTERVAL;
	int timeout_cnt = 0;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	//printf("read size: %d\n",total);

WAIT_READ:	
	pthread_mutex_lock(&si->buflock);

	//printf("out=%d,in=%d\n",si->outbuf,si->inbuf);

	if(si->outbuf == si->inbuf){
		/**
		* no data in buf
		*/
		if(stream_eof(si->streaming)){
			nr=0;
			pthread_mutex_unlock(&si->buflock);
		}
		else{
			//printf("out=%d,in=%d\n",si->outbuf,si->inbuf);
			pthread_mutex_unlock(&si->buflock);
		
		#ifdef STREAM_DEBUG_EN
			printf("No DATA 1\n");
		#endif
			if(timeout_cnt<timeout_max){
				timeout_cnt++;
				sleep(NO_DATA_SLEEP_INTERVAL);
				goto WAIT_READ;
			}
			else{
				nr = 0;
			}
	
		}
	}
	else if(si->outbuf < si->inbuf){
		remain = si->inbuf - si->outbuf;
		if(remain >= total){
			fast_memcpy((void *)mem,(void *)(si->buf+si->outbuf),total);
			si->outbuf += total;
			nr = total;
			timeout_cnt = 0;
			///printf("read-- out=%d,in=%d\n",si->outbuf,si->inbuf);
			pthread_mutex_unlock(&si->buflock);
			
		}
		else{

			if(stream_eof(si->streaming)){
				fast_memcpy((void *)mem,(void *)(si->buf+si->outbuf),remain);
				si->outbuf += remain;
				nr = remain;
				timeout_cnt = 0;
				///printf("read-- out=%d,in=%d\n",si->outbuf,si->inbuf);
				pthread_mutex_unlock(&si->buflock);
				
			}
			else{
			
			#ifdef STREAM_DEBUG_EN
				printf("Not Enough Data In Buffer1,%d,%d,%d\n", si->inbuf, si->outbuf,remain);
			#endif
				pthread_mutex_unlock(&si->buflock);

				if(timeout_cnt<timeout_max){
					timeout_cnt++;
					sleep(NO_DATA_SLEEP_INTERVAL);
					goto WAIT_READ;
				}
				else{
					nr = 0;
				}
			}
		}
	}
	else{
		int remain1;
		
		remain = si->bufsize - si->outbuf + si->inbuf;
		remain1 = si->bufsize - si->outbuf;
		
		if(remain >= total){
			if(remain1 >= total){
				fast_memcpy((void *)mem,(void *)(si->buf+si->outbuf),total);
				si->outbuf = (si->outbuf + total)%si->bufsize;
			}
			else{
				fast_memcpy((void *)mem,(void *)(si->buf+si->outbuf),remain1);
				fast_memcpy((void *)(mem+remain1),(void *)si->buf,total-remain1);
				si->outbuf = (total-remain1)%si->bufsize;
			}
			nr = total;
			timeout_cnt = 0;
			///printf("read-- out=%d,in=%d\n",si->outbuf,si->inbuf);
			pthread_mutex_unlock(&si->buflock);
			
		}
		else{

			if(stream_eof(si->streaming)){
				fast_memcpy((void *)mem,(void *)(si->buf+si->outbuf),remain1);
				fast_memcpy((void *)(mem+remain1),(void *)si->buf,si->inbuf);
				si->outbuf = si->inbuf;
				nr = remain;
				timeout_cnt = 0;
				pthread_mutex_unlock(&si->buflock);
			}
			else{
			
			#ifdef STREAM_DEBUG_EN
				printf("Not Enough Data In Buffer2\n");
			#endif
				pthread_mutex_unlock(&si->buflock);

				if(timeout_cnt<timeout_max){
					timeout_cnt++;
					sleep(NO_DATA_SLEEP_INTERVAL);
					goto WAIT_READ;
				}
				else{
					nr = 0;
				}
			}
		}
	}

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif



	return nr;
}


/**
* NOTE: Before call this function, you should hold the two
* lockers and after call this function, you should release the 
* two lockers.
*/
static int am_stream_copy_tmp_buffer(void* s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int tmpsize;
	int remain;
	int bk;

	if(si == NULL){
		return -1;
	}

	tmpsize = si->read_end - si->read_start;
	if(tmpsize > 0){
		bk = si->inbuf;
		if((si->inbuf + tmpsize) <= si->bufsize){
			fast_memcpy((void *)(si->buf+si->inbuf),(void *)(si->readbuf+si->read_start),tmpsize);
			si->read_start += tmpsize;
			si->inbuf = (si->inbuf + tmpsize)%si->bufsize;
		}
		else{
			remain = si->bufsize-si->inbuf;
			fast_memcpy((void *)(si->buf+si->inbuf),(void *)(si->readbuf+si->read_start),remain);
			si->read_start += remain;
			fast_memcpy((void *)si->buf,(void *)(si->readbuf+si->read_start),tmpsize - remain);
			si->read_start += (tmpsize - remain);
			si->inbuf = (tmpsize - remain)%si->bufsize;
		}

		if(bk > si->inbuf){
			si->rollover = 1;
		}
	}

	return 0;
}

static int am_stream_buffered_seek_internal(void*s,off_t pos)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t curpos,curinpos;
	int remain,offset;
	int err;
	int tmpsize;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return -1;
	}

	pthread_mutex_lock(&si->buflock);
	pthread_mutex_lock(&si->readbuflock);
	
	curpos = stream_tell(si->streaming);

	if(pos > curpos){
		/**
		* My god, we have not enough data buffered.
		*/
		goto _SHOULD_SEEK;
	}
	else if(pos == curpos){
		/**
		* Oh, we have exactly read to this pointer.
		*/
		am_stream_copy_tmp_buffer((void*) si);
		si->outbuf = si->inbuf;
		pthread_mutex_unlock(&si->readbuflock);
		pthread_mutex_unlock(&si->buflock);
		
		return 0;
	}
	else{

		/**
		* seek according to bytes remained.
		*/
		off_t calpos;
		off_t remain_large,offset_large;
		
		tmpsize = si->read_end - si->read_start;
		curinpos = curpos - tmpsize;
		if(pos <= curinpos){
			calpos = curinpos;
		}
		else{
			am_stream_copy_tmp_buffer((void*) si);
			calpos = curpos;
		}
		
		if(si->rollover){
			remain = si->bufsize-1;
		}
		else{
			remain = si->inbuf;
		}

		/** 
		* ugly code @_@.
		* but the 32bits and 64bits date type conversion must be 
		* handled carefully.
		*/
		remain_large = (off_t)remain;
		offset_large = calpos - pos;
		if(offset_large <= remain_large){
			offset = (int)offset_large;
			si->outbuf = (si->inbuf + si->bufsize - offset)%si->bufsize;
			pthread_mutex_unlock(&si->readbuflock);
			pthread_mutex_unlock(&si->buflock);
			return 0;
		}
		
	}
	
_SHOULD_SEEK:
	err = am_stream_seek_internal((void*)si,pos);
	if(err < 0){
		pthread_mutex_unlock(&si->readbuflock);
		pthread_mutex_unlock(&si->buflock);
		return -1;
	}
	si->inbuf = 0;
	si->outbuf = 0;
	si->rollover = 0;
	si->read_start = 0;
	si->read_end = 0;
	pthread_mutex_unlock(&si->readbuflock);
	pthread_mutex_unlock(&si->buflock);

	return 0;
}

static int am_stream_buffered_seek_cur_internal(void*s,off_t pos)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t curpos,transpos,curinpos;
	int err;
	int remain,offset;
	int tmpsize;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return -1;
	}

	pthread_mutex_lock(&si->buflock);
	pthread_mutex_lock(&si->readbuflock);

	tmpsize = si->read_end - si->read_start;
	curpos = stream_tell(si->streaming);

	if(si->inbuf == si->outbuf){
		transpos = curpos-tmpsize;
	}
	else if(si->inbuf > si->outbuf){
		transpos = curpos-tmpsize - (si->inbuf - si->outbuf);
	}
	else{
		transpos = curpos-tmpsize - (si->inbuf + si->bufsize - si->outbuf);
	}
	pos = transpos+pos;

	if(pos > curpos){
		/**
		* My god, we have not enough data buffered.
		*/
		goto _SHOULD_SEEK;
	}
	else if(pos == curpos){
		/**
		* Oh, we have exactly read to this pointer.
		*/
		am_stream_copy_tmp_buffer((void*) si);
		si->outbuf = si->inbuf;
		pthread_mutex_unlock(&si->readbuflock);
		pthread_mutex_unlock(&si->buflock);
		
		return 0;
	}
	else{

		/**
		* seek according to bytes remained.
		*/
		off_t calpos;
		off_t remain_large,offset_large;
		
		tmpsize = si->read_end - si->read_start;
		curinpos = curpos - tmpsize;
		if(pos <= curinpos){
			calpos = curinpos;
		}
		else{
			am_stream_copy_tmp_buffer((void*) si);
			calpos = curpos;
		}

		if(si->rollover){
			remain = si->bufsize-1;
		}
		else{
			remain = si->inbuf;
		}

		remain_large = remain;

		offset_large = calpos - pos;
		if(offset_large <= remain_large){
			offset = (int)offset_large;
			si->outbuf = (si->inbuf + si->bufsize - offset)%si->bufsize;
			pthread_mutex_unlock(&si->readbuflock);
			pthread_mutex_unlock(&si->buflock);

			return 0;
		}
		
	}

	
_SHOULD_SEEK:

	err = am_stream_seek_internal((void*)si,pos);
	if(err < 0){
		pthread_mutex_unlock(&si->readbuflock);
		pthread_mutex_unlock(&si->buflock);
		return -1;
	}
	si->inbuf = 0;
	si->outbuf = 0;
	si->rollover = 0;
	si->read_start = 0;
	si->read_end = 0;
	pthread_mutex_unlock(&si->readbuflock);
	pthread_mutex_unlock(&si->buflock);
	
	return 0;

}

static int am_stream_buffered_seek_end_internal(void *s,off_t offset)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t len;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return -1;
	}

	len = stream_fsize(si->streaming);
	if(len > 0){
		if(offset < 0){
			len += offset;
		}
		else{
			len -= offset;
		}
		return am_stream_buffered_seek_internal(s,len);
	}

	return -1;
}

EXPORT_SYMBOL
int am_stream_seek_set(void*s,stream_i64 pos)
{
	off_t internal_pos;

	internal_pos = (off_t)pos;

	return am_stream_buffered_seek_internal(s,internal_pos);
}

EXPORT_SYMBOL
int am_stream_seek_cur(void *s,stream_i64 pos)
{
	off_t internal_pos;

	internal_pos = (off_t)pos;

	return am_stream_buffered_seek_cur_internal(s,internal_pos);
}

EXPORT_SYMBOL
int am_stream_seek_end(void *s,stream_i64 offset)
{
	off_t internal_pos;

	internal_pos = (off_t)offset;
	return am_stream_buffered_seek_end_internal(s,internal_pos);
}

EXPORT_SYMBOL
off_t am_stream_get_pos(void *s)
{
	off_t pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t transpos;
	off_t offset;
	int tmpsize;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream get pos error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	pthread_mutex_lock(&si->buflock);
	pthread_mutex_lock(&si->readbuflock);

	pos = stream_tell(si->streaming);
	tmpsize = si->read_end - si->read_start;

	if(si->inbuf == si->outbuf){
		transpos = pos-tmpsize;
	}
	else if(si->inbuf > si->outbuf){
		transpos = pos - tmpsize - (si->inbuf - si->outbuf);
	}
	else{
		transpos = pos - tmpsize - (si->inbuf + si->bufsize - si->outbuf);
	}
	
	pthread_mutex_unlock(&si->readbuflock);
	pthread_mutex_unlock(&si->buflock);

	return transpos;

	//return stream_tell((stream_t *)s);
}

EXPORT_SYMBOL
void* am_stream_audio_open(const char *filename,char**options,int*file_format)
{

	return am_stream_open(filename,options,file_format);
}

EXPORT_SYMBOL
inline int am_stream_audio_read(void *s,char* mem,int total)
{
	return am_stream_read(s,mem,total);
}

EXPORT_SYMBOL
//int am_stream_audio_seek_set(void*s,off_t pos)
int am_stream_audio_seek_set(void *s,long pos)
{
	return am_stream_buffered_seek_internal(s,(off_t)pos);
}

EXPORT_SYMBOL
//int am_stream_audio_seek_cur(void *s,off_t pos)
int am_stream_audio_seek_cur(void *s,long pos)
{
	return am_stream_buffered_seek_cur_internal(s,(off_t)pos);
}

EXPORT_SYMBOL
//int am_stream_audio_seek_end(void *s,off_t offset)
int am_stream_audio_seek_end(void *s,long offset)
{
	return am_stream_buffered_seek_end_internal(s,(off_t)offset);
}

EXPORT_SYMBOL
inline off_t am_stream_audio_get_pos(void *s)
{
	return am_stream_get_pos(s);
}

EXPORT_SYMBOL
int am_stream_audio_get_filesize(void *s)
{
	return (int)am_stream_get_filesize(s);
}

EXPORT_SYMBOL
void am_stream_audio_close(void *s)
{

	am_stream_close(s);
}


#else
/**
* We wrap the stream_t in our own structure.
*/
struct wrap_stream_t{
	stream_t *streaming;
	char *url;
};

EXPORT_SYMBOL
void* am_stream_open(const char *filename,char**options,int*file_format)
{
	struct wrap_stream_t *s=NULL;

	s = (struct wrap_stream_t *)calloc(1,sizeof(struct wrap_stream_t));
	if(s == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}

	s->url = strdup(filename);
	if(s->url == NULL){
		free(s);
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}
	
	s->streaming = open_stream(filename,options,file_format);
	if(s->streaming == NULL){
		free(s->url);
		free(s);

	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}

	return (void *)s;
}

EXPORT_SYMBOL
inline int am_stream_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	
	nr = stream_read(si->streaming,mem,total);

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif

	return nr;
}



EXPORT_SYMBOL
int am_stream_seek_set(void*s,stream_i64 pos)
{
	off_t internal_pos;

	internal_pos = (off_t)pos;

	return am_stream_seek_internal(s,internal_pos);
}


static int am_stream_seek_cur_internal(void *s,off_t pos)
{
	off_t cur;
	int err;
	off_t internal_pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if((si==NULL) || (si->streaming == NULL)){

#ifdef STREAM_DEBUG_EN
		printf("stream seek cur error: Pointer NULL!\n");
#endif

		return -1;
	}

	internal_pos = (off_t)pos;
	cur = stream_tell(si->streaming);
	cur += internal_pos;

	return am_stream_seek_internal(s,cur);
}

EXPORT_SYMBOL
int am_stream_seek_cur(void *s,stream_i64 pos)
{
	off_t internal_pos;

	internal_pos = (off_t)pos;

	return am_stream_seek_cur_internal(s,internal_pos);
}



static int am_stream_seek_end_internal(void *s,off_t offset)
{
	off_t len = 0;
	unsigned long skipped_num = 0;
	int err;
	off_t internal_pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream seek end error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	internal_pos = (off_t)offset;

	/**
	* Can get the stream size easily.
	*/
	len = stream_fsize(si->streaming);
	if(len > 0){
		if(internal_pos < 0){
			len += internal_pos;
		}
		else{
			len -= internal_pos;
		}
		return am_stream_seek_internal(s,len);
	}
	
	/**
	* A very slow seek end by reading all the data.
	*/
	stream_reset(si->streaming);

	
	while(!stream_eof(si->streaming)) {
		skipped_num = stream_read(si->streaming,seek_buf,SEEK_BUFFER_SIZE);
		len += skipped_num;
	}

	stream_reset(si->streaming);

	if(internal_pos < 0){
		len += internal_pos;
	}
	else{
		len -= internal_pos;
	}

	return am_stream_seek_internal(s,len);
	
}

EXPORT_SYMBOL
int am_stream_seek_end(void *s,stream_i64 offset)
{
	off_t internal_pos;

	internal_pos = (off_t)offset;
	return am_stream_seek_end_internal(s,internal_pos);
}

EXPORT_SYMBOL
inline off_t am_stream_get_pos(void *s)
{
	off_t pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream get pos error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	pos = stream_tell(si->streaming);

	return pos;

	//return stream_tell((stream_t *)s);
}

EXPORT_SYMBOL
stream_i64 am_stream_get_filesize(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return 0;
	}
	
	return stream_fsize(si->streaming);
}


EXPORT_SYMBOL
void am_stream_close(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream close error: Pointer NULL!\n");
	#endif
	
		return;
	}

	free_stream(si->streaming);
	free(si->url);
	free(si);
	si = NULL;
}


EXPORT_SYMBOL
void* am_stream_audio_open(const char *filename,char**options,int*file_format)
{

	return am_stream_open(filename,options,file_format);
}

EXPORT_SYMBOL
inline int am_stream_audio_read(void *s,char* mem,int total)
{
	return am_stream_read(s,mem,total);
}

EXPORT_SYMBOL
int am_stream_audio_seek_set(void *s,long pos)
//int am_stream_audio_seek_set(void*s,off_t pos)
{
	return am_stream_seek_internal(s,(off_t)pos);
}

EXPORT_SYMBOL
//int am_stream_audio_seek_cur(void *s,off_t pos)
int am_stream_audio_seek_cur(void *s,long pos)
{
	return am_stream_seek_cur_internal(s,(off_t)pos);
}

EXPORT_SYMBOL
//int am_stream_audio_seek_end(void *s,off_t offset)
int am_stream_audio_seek_end(void *s,long offset)
{
	return am_stream_seek_end_internal(s,(off_t)offset);
}

EXPORT_SYMBOL
inline off_t am_stream_audio_get_pos(void *s)
{
	return am_stream_get_pos(s);
}

EXPORT_SYMBOL
int am_stream_audio_get_filesize(void *s)
{
	return (int)am_stream_get_filesize(s);
}

EXPORT_SYMBOL
void am_stream_audio_close(void *s)
{

	am_stream_close(s);
}
#endif /** BUFFERED_STREAM */

static int am_stream_internal_raw_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	
	nr = stream_read(si->streaming,mem,total);

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif

	return nr;
}


static int am_stream_read_internal(void *s,off_t remain)
{
	int rd;

	if(remain < 0){
		
	#ifdef STREAM_DEBUG_EN
		printf("%s error: remain < 0 \n",__FUNCTION__);
	#endif
	
		return -1;
	}
	
	while(remain >0){
		int should_rd;

		if(remain > SEEK_BUFFER_SIZE){
			should_rd = SEEK_BUFFER_SIZE;
		}
		else{
			should_rd = remain;
		}

		rd = am_stream_internal_raw_read(s,seek_buf,should_rd);
		if(rd < 0){

#ifdef STREAM_DEBUG_EN
			printf("%s error:seek error happends\n",__FUNCTION__);
#endif
			return -1;
		}
		else if(rd ==0){
			
#ifdef STREAM_DEBUG_EN
			printf("%s: Already seek to end\n",__FUNCTION__);
#endif
			return 0;			
		}
		remain -= rd;
	}

	return 0;
}

/**
* Seek from the beginning of the file.
*/
static int am_stream_seek_internal(void*s,off_t pos)
{
	int err;
	off_t internal_pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	off_t after;
	int file_format;
	int rd;

	internal_pos = (off_t)pos;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	err = stream_seek(si->streaming,internal_pos);
	if(err <=0){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Internal Seek Error [%d]!\n",err);
	#endif
	
		return -1;
	}

	after = stream_tell(si->streaming);
	if(after != internal_pos){

		if(internal_pos > after){
			if(am_stream_read_internal(s,internal_pos-after)>=0){
				
			#ifdef STREAM_DEBUG_EN
				printf("seek1 to %llx,succeed=%d\n",stream_tell(si->streaming),stream_tell(si->streaming)==internal_pos);
			#endif
				return 0;
			}
			else{
				
			#ifdef STREAM_DEBUG_EN
				printf("seek1 to %d error\n",stream_tell(si->streaming));
			#endif	
			
			}
		}
		
		/**
		* If the position after seeking is not what we expected,
		* let's do all from scratch.
		*/
		stream_reset(si->streaming);
		free_stream(si->streaming);
		si->streaming = NULL;
		si->streaming = open_stream(si->url,NULL,&file_format);
		if(si->streaming == NULL){	
		
		#ifdef STREAM_DEBUG_EN
			printf("stream seek error: Stream reopen error!\n");
		#endif

			return -1;
		}
		if(am_stream_read_internal(s,internal_pos)>=0){
				
			#ifdef STREAM_DEBUG_EN
				printf("seek2 to %llx,succeed=%d\n",stream_tell(si->streaming),stream_tell(si->streaming)==internal_pos);
			#endif
			
				err = 1;
		}
		else{
			err = 0;
		}
		/**
		* Set err to correct value.
		*/
				
	}

	if(err > 0){
		err = 0;
	}
	else{
		
	#ifdef STREAM_DEBUG_EN
		printf("stream seek error: Return error %d\n",err);
	#endif
	
		err = -1;
	}

	return err;
}

EXPORT_SYMBOL
int am_stream_seekable(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if(si == NULL){
		return 0;
	}
	
	return stream_seekable(si->streaming);
}

EXPORT_SYMBOL
void* am_ota_open(const char *filename,char**options,int*file_format)
{
	struct wrap_stream_t *s=NULL;

	s = (struct wrap_stream_t *)calloc(1,sizeof(struct wrap_stream_t));
	if(s == NULL){
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error--1: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}

	s->url = strdup(filename);
	if(s->url == NULL){
		free(s);
		
	#ifdef STREAM_DEBUG_EN
		printf("stream open error--2: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}
	
	s->streaming = open_stream(filename,options,file_format);
	if(s->streaming == NULL){
		free(s->url);
		free(s);

	#ifdef STREAM_DEBUG_EN
		printf("stream open error: Not Enough Memory!\n");
	#endif
	
		return NULL;
	}

	return (void *)s;
}

EXPORT_SYMBOL
inline int am_ota_read(void *s,char* mem,int total)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	int nr;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream read error: Pointer NULL!\n");
	#endif
	
		return -1;
	}
	
	nr = stream_read(si->streaming,mem,total);

#ifdef STREAM_DEBUG_EN
	if(nr < 0){
		printf("stream read error: %d\n",nr);
	}
#endif

	return nr;
}

EXPORT_SYMBOL
inline off_t am_ota_get_pos(void *s)
{
	off_t pos;
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;

	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream get pos error: Pointer NULL!\n");
	#endif
	
		return -1;
	}

	pos = stream_tell(si->streaming);

	return pos;

	//return stream_tell((stream_t *)s);
}

EXPORT_SYMBOL
stream_i64 am_ota_get_filesize(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("%s error: Pointer NULL!\n",__FUNCTION__);
	#endif
	
		return 0;
	}
	
	return stream_fsize(si->streaming);
}


EXPORT_SYMBOL
void am_ota_close(void *s)
{
	struct wrap_stream_t *si = (struct wrap_stream_t *)s;
	
	if((si==NULL) || (si->streaming == NULL)){

	#ifdef STREAM_DEBUG_EN
		printf("stream close error: Pointer NULL!\n");
	#endif
	
		return;
	}

	free_stream(si->streaming);
	free(si->url);
	free(si);
	si = NULL;
}
#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"

#define ENOMEM  12
#define EAGAIN  11

static void dump_read_data(const unsigned char *pdata,unsigned int len){
	unsigned int tmp=0;
	while(len--){
		if(tmp%16 == 0){
			printf("\n%02x",*(pdata+tmp));
		}else{
			printf(" %02x",*(pdata+tmp));	
		}
		tmp++;
	}
	printf("\n");
}

EXPORT_SYMBOL
int am_http_download(const char *url,unsigned int try_time){
	struct wrap_stream_t *s_ota=NULL;
	 long file_len;
	 FILE *fp;
	int fd,times,res=0,format;
	ota_data_t	ota_temp;
	if((NULL==url))
		return -ENOMEM;	
	s_ota=(struct wrap_stream_t *)am_ota_open(url,NULL,&format);
	if(NULL==s_ota){
		printf("can not get stream!\n");
		return -EAGAIN;
	}
	while((file_len=am_ota_read(s_ota,(char *)ota_temp.buf,OTA_RWSIZE))>0){	
			printf("read %d Bytes from http server!\n",file_len);
			dump_read_data((unsigned char *)ota_temp.buf,file_len);
			fp=fopen("/mnt/udisk/test.txt","w+");
			res=fwrite((char *)ota_temp.buf,sizeof(char),file_len,fp);
			printf("write %d Bytes to test.txt!\n",res);
		}
		return 0;			
}

EXPORT_SYMBOL
int am_ota_download(const char *url,unsigned int try_time){
	void *s_ota=NULL;
	long file_len;
	FILE *fp=NULL;
	char callbuf[256];
	int fd,times,res=0,format;
	ota_data_t	ota_temp;
	if((NULL==url))
		return -ENOMEM;
	if(access("/sys/module/am7x_ota_upgrade",F_OK)==-1){
		sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_ota_upgrade.ko");
		system(callbuf);
	}
	fd=open("/dev/otadrv",O_RDWR);
	if(fd<0){
		printf("open /dev/otadrv failed!\n");
		return -EAGAIN;
	}
	s_ota=am_ota_open(url,NULL,&format);
	if(NULL==s_ota){
		printf("can not get stream!\n");
		return -EAGAIN;
	}
	ioctl(fd,OTA_ENTER,format);
	printf("enter ota!\n");
	printf("file_len is valid ,download data\n");	
	ota_temp.offset = 0;
/*
	while(file_len>OTA_RWSIZE){
		am_ota_read(s_ota,mem,OTA_RWSIZE);
		printf("read stream now!\n");
		ota_temp.offset += (OTA_RWSIZE/512);
		ota_temp.sector_num = (OTA_RWSIZE/512);
		ota_temp.buf = mem;
		times = 0;		
		do{
			if(ioctl(fd,OTA_DOWNLOAD_FW,ota_temp))
				times++;
			else
			 	break;
		}while(times<try_time);
		if(times == try_time){
			printf("ota download error\n");
			res = -1;
			break;
		}
		
		file_len -= OTA_RWSIZE;
	}
	if(0 == res){
		am_ota_read(s_ota,mem,file_len);
	
		ota_temp.offset += OTA_RWSIZE/512;
		ota_temp.sector_num = OTA_RWSIZE/512;
		ota_temp.buf = mem;
		times = 0;
		do{
			if(ioctl(fd,OTA_DOWNLOAD_FW,ota_temp))
				times++;
			else
			 	break;
		}while(times<try_time);
		if(times==try_time)
			res = -1;	
	}

	while(file_len>OTA_RWSIZE){
		am_ota_read(s_ota,(char *)ota_temp.buf,OTA_RWSIZE);
		printf("read stream now!\n");
		ota_temp.sector_num = (OTA_RWSIZE/512);
		printf("offset:%d,len:%d\n",ota_temp.offset,ota_temp.sector_num);	
		ioctl(fd,OTA_DOWNLOAD_FW,(ota_data_t *)&ota_temp);
		ota_temp.offset += (OTA_RWSIZE/512);		
		file_len =file_len - OTA_RWSIZE;
	}
		am_ota_read(s_ota,(char *)ota_temp.buf,file_len);	
		ota_temp.sector_num = file_len/512;
		ioctl(fd,OTA_DOWNLOAD_FW,(ota_data_t *)&ota_temp);	
		*/
	printf("new read stream now!\n");	
	while((file_len=am_ota_read(s_ota,(char *)ota_temp.buf,OTA_RWSIZE))>0){
		ota_temp.sector_num = (file_len/512);	
		ioctl(fd,OTA_DOWNLOAD_FW,(ota_data_t *)&ota_temp);
		ota_temp.offset += (file_len/512);
	}
		
OTA_OUT:
	close(fd);
	am_ota_close(s_ota);
	return res;
}


#endif

