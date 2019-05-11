#include "stdio.h"
#include "errno.h"
#include <stdlib.h>
/*************************************
	IO Layter Interface
***************************************/

///for image
void *fui_os_fopen(char *path, char *mode){
	return (void*)fopen(path, mode);
}

int fui_os_fclose(void *fp){
	return fclose((FILE*)fp);
}

long fui_os_fread(void *fp, unsigned char *ptr, unsigned long nbytes){
	return fread(ptr, sizeof(unsigned char),nbytes,(FILE*)fp);
}

long fui_os_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes){
	return fwrite(ptr, sizeof(unsigned char), nbytes,(FILE*)fp);
}

long fui_os_fseek_set(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_SET);
}

long fui_os_fseek_cur(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_CUR);
}

long fui_os_fseek_end(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_END);
}

long fui_os_ftell(void *fp){
	return ftell((FILE*)fp);
}

void *fui_os_malloc(unsigned long size)
{
	//return SWF_Malloc((unsigned int)size);
	return (void*)malloc((unsigned int)size);
}

int fui_os_fflush(void *fp)
{
	int fd;
	fflush((FILE *)fp);
	fd = fileno((FILE *)fp);
	if(fsync(fd)==-1){
		printf("%s,%d: Fflush Error=%d!\n",__FILE__,__LINE__,errno);
		return -1;
	}
	return 0;
}

void *fui_os_realloc(void *ptr,unsigned long size)
{
	#if 0
	char *new_buf;
	unsigned long oldsize=0;
	new_buf = (char*)SWF_Malloc(size);
	if(new_buf==NULL){
		printf("%s,%d:Realloc Faled!\n",__FILE__,__LINE__);
		return NULL;
	}
	memcpy(new_buf,ptr,oldsize);
	SWF_Free(ptr);
	return new_buf;
	#else 
	return (void*)realloc(ptr,size);
	#endif
}

void fui_os_free(void * pfree){
	//SWF_Free(pfree);
	free(pfree);
}

////For Audio 
void *_audio_fopen(char *path, char *mode)
{
	return (void*)fopen(path, mode);
}

int _audio_fclose(void *fp)
{
	return fclose((FILE*)fp);
}

long  _audio_read_packet(void *opaque,void *buf,int buf_size)
{
	return (int)fread(buf, sizeof(unsigned char), buf_size, (FILE*)opaque);
}

int _audio_write_packet(void *buf, int size, int buf_size,void *opaque)
{
	return (int)fwrite(buf, size, buf_size, (FILE*)opaque);
}

long _audio_file_seek_set(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_SET);
}

long _audio_file_seek_cur(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_CUR);
}

long _audio_file_seek_end(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_END);
}

long _audio_get_pos(void *opaque)
{
	return ftell((FILE*)opaque);
}





