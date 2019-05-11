#ifndef AUDIO_COMMON_H
#define AUDIO_COMMON_H

#include "fileselector.h"
#include "audio_midware.h"
#include "mmm_music.h"

#define AUDIO_ENGINE_MAX 8

extern void *_audio_fopen(char *path, char *mode);
extern void *_audio_fopen(char *path, char *mode);
extern long  _audio_read_packet(void *opaque,void *buf,int buf_size);
extern long _audio_file_seek_set(void *opaque, long offset);
extern long _audio_file_seek_cur(void *opaque, long offset);
extern long _audio_file_seek_end(void *opaque, long offset);
extern long _audio_get_pos(void *opaque);


struct _audio_decode_info{
	void*	audio_engine;
	unsigned char*	buf;
	int		size;
	unsigned char	*head,*tail,*pos;
	int DeviceId;
	int type; //0x55: event sound; 0xaa:streamsound
	int loop_count;
	int nodate_count;
	int stop_flag;
	int owner;
	int s; 
	struct file_info{
	file_iocontext_s file_iocontext;
	audio_header_info_t header;
	}file;
	music_file_info_t media_info;
	music_status_t player_status;
};

// Add By VICTOR for PCM  2013.01.10
typedef struct {
	unsigned int file_type;
	pcm_info_t 	 pcm_info;
}audio_para_t;
// End By VICTOR

typedef void *(*audio_dec_open)(void *);
typedef int (*audio_dec_cmd)(void *,unsigned int,unsigned int);
typedef int (*audio_dec_close)(void *);

#endif