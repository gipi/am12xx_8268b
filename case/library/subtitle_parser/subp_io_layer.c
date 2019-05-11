#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "swfdec.h"
/*************************************
	IO Layter Interface
***************************************/
void *subp_fopen(char *path, char *mode)
{
	return (void*)fopen(path, mode);
}

int subp_fclose(void *fp)
{
	return fclose((FILE*)fp);
}

long subp_fread(void *fp, unsigned char *ptr, unsigned long nbytes)
{
	return fread(ptr, sizeof(unsigned char),nbytes,(FILE*)fp);
}

long subp_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes)
{
	return fwrite(ptr, sizeof(unsigned char), nbytes,(FILE*)fp);
}

long subp_fseek_set(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_SET);
}

long subp_fseek_cur(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_CUR);
}

long subp_fseek_end(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_END);
}

long subp_ftell(void *fp)
{
	return ftell((FILE*)fp);
}

void *subp_malloc(int size)
{
	char * buf=NULL;
	buf =  malloc((unsigned int)size);
	if(buf!=NULL)
		memset(buf,0,size);
	else
		printf("Sorry Malloc Failed!\n");
	return (void*)buf;
}

void subp_free(void * pfree)
{
	free(pfree);
}


void *subp_remalloc(void*buf,unsigned int size)
{
	return realloc(buf,size);
}

int subp_feof(void* fp)
{
	return feof((FILE*)fp);
}
/************************************************/