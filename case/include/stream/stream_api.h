#ifndef STREAM_API_H
#define STREAM_API_H

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
typedef __int64	stream_i64;
typedef unsigned __int64 stream_u64;
#else
typedef long long stream_i64;
typedef unsigned long long stream_u64;
#endif


#ifdef __cplusplus
extern "C" {
#endif

void am_cache_setavseeking(long indicate);
void am_cache_setdatapos(long long file_pos);
void am_cache_avseekplay(void);

extern void* am_stream_open(const char *filename,char**options,int*file_format);
extern inline int am_stream_read(void *s,char* mem,int total);
extern int am_stream_seek_set(void *s,stream_i64 pos);
extern int am_stream_seek_cur(void *s,stream_i64 pos);
extern int am_stream_seek_end(void *s,stream_i64 offset);
extern inline off_t am_stream_get_pos(void *s);
extern void am_stream_close(void *s);
extern stream_i64 am_stream_get_filesize(void *s);
extern int am_stream_seekable(void *s);

//add for audio
extern void* am_stream_audio_open(const char *filename,char**options,int*file_format);
extern inline int am_stream_audio_read(void *s,char* mem,int total);
extern int am_stream_audio_seek_set(void *s,long pos);
extern int am_stream_audio_seek_cur(void *s,long pos);
extern int am_stream_audio_seek_end(void *s,long offset);
extern inline off_t am_stream_audio_get_pos(void *s);
extern void am_stream_audio_close(void *s);
extern int am_stream_audio_get_filesize(void *s);
extern int am_stream_seek_internal(void*s,off_t pos);

extern void* am_ota_open(const char *filename,char**options,int*file_format);

extern stream_i64 am_ota_get_filesize(void *s);
extern inline int am_ota_read(void *s,char* mem,int total);

extern void am_ota_close(void *s);

extern int am_ota_download(const char *url,unsigned int try_time);
extern int am_http_download(const char *url,unsigned int try_time);


#ifdef __cplusplus
}
#endif

#endif

