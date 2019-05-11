#include <stdio.h>
#include <stdlib.h>
#include "fui_input.h"


typedef struct {
	stream_input_t input;
	void * fp;
}local_stream_input_t;


static int local_read(struct stream_input_s *input,unsigned int buf,unsigned int len)
{
	local_stream_input_t * f = (local_stream_input_t*)input;
	FILE *fp = (FILE *)f->fp;
	int nr=0;
	
	if((len <= 0) || (fp == NULL)){
		return -1;
	}
	nr=fread((void *)buf, sizeof(char), len, fp);

	return nr;
}

static int local_write(struct stream_input_s *input,unsigned int buf,unsigned int len)
{
	int count;
	int fd;
	local_stream_input_t * f = (local_stream_input_t*)input;

	if((f == NULL)||(f->fp == NULL) || (len <= 0)){
		return -1;
	}
	count = fwrite((void *)buf,sizeof(char),len, (FILE *)f->fp);
	fflush((FILE *)f->fp);
	fd = fileno((FILE *)f->fp);
	fsync(fd);
	//printf("%s,%d:count is %d\n",__FILE__,__LINE__,count);
	return count;
}

static int local_seek(struct stream_input_s *input,int offset,int original)
{
	local_stream_input_t * f = (local_stream_input_t*)input;
	
	static const int seek_cmd[4] = {
		0,
		SEEK_SET, //DSEEK_SET
		SEEK_END, //DSEEK_END
		SEEK_CUR, //DSEEK_CUR
	};

	if((f == NULL)||(f->fp == NULL)){
		return -1;
	}
	return fseek((FILE *)f->fp, offset, seek_cmd[original]);
	
}

static int local_tell(struct stream_input_s *input)
{
	local_stream_input_t * f = (local_stream_input_t*)input;

	if((f == NULL)||(f->fp == NULL)){
		return -1;
	}
	
	return ftell((FILE *)f->fp);
}

static int local_dispose(struct stream_input_s *input)
{
	int fd;
	local_stream_input_t * f = (local_stream_input_t*)input;

	if((f == NULL)||(f->fp == NULL)){
		return -1;
	}
	fflush((FILE *)f->fp);
	fd = fileno((FILE *)f->fp);
	fsync(fd);
	fclose((FILE *)f->fp);
	free(f);
	return 0;
}

static int fs_read(struct stream_input_s *input,unsigned int buf,unsigned int len)
{
	return local_read(input,buf,len);
}

static int fs_write(struct stream_input_s *input,unsigned int buf,unsigned int len)
{
	return local_write(input,buf,len);
}

static int fs_seek(struct stream_input_s *input,int offset,int original)
{
	return local_seek(input, offset, original);
}

static int fs_tell(struct stream_input_s *input)
{
	return local_tell(input);
}

static int fs_dispose(struct stream_input_s *input)
{
	return local_dispose(input);
}

stream_input_t * create_fui_input(char * filename)
{
	local_stream_input_t * f = (local_stream_input_t*)malloc(sizeof(local_stream_input_t));

	if(f==NULL){
		return NULL;
	}
	f->input.read = local_read;
	f->input.seek = local_seek;
	f->input.tell = local_tell;
	f->input.dispose = local_dispose;
	f->fp = (void *)fopen(filename, "rb");
	if(f->fp == NULL) {
		free(f);
		return NULL;
	}
	return (stream_input_t*)f;
}

stream_input_t * create_fui_output(char * filename)
{
	local_stream_input_t * f = (local_stream_input_t*)malloc(sizeof(local_stream_input_t));

	if(f==NULL){
		return NULL;
	}
	f->input.write = local_write;
	f->input.dispose = local_dispose;
	f->fp = (void *)fopen(filename, "wb");
	if(f->fp == NULL) {
		free(f);
		return NULL;
	}
	return (stream_input_t*)f;
}

