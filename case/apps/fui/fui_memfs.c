#include "fui_memfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
* @brief open a memory buffer to operate.
*
* @param buffer: buffer to memory.
* @param size: size of the buffer.
*
* @return if open success, return a pointer to "struct fui_memfs",
*    if open failed,return NULL.
*/
void *fui_memfs_open(char *buffer, long size)
{
	struct fui_memfs *fp;

	if(buffer == NULL){
		return NULL;
	}

	if(size <= 0){
		return NULL;
	}

	fp = (struct fui_memfs *)malloc(sizeof(struct fui_memfs));
	if(fp == NULL){
		return NULL;
	}

	fp->addr = buffer;
	fp->dataoffset = 0;
	fp->size = size;

	return (void *)fp;	
}


/**
* @brief Close a memory FS.
*
* @param fp: A pointer to a memory FS that previously opened.
*
* @return On success returns 0. On error returns -1.
*/
long fui_memfs_close(void *fp)
{
	struct fui_memfs *ptr;

	ptr = (struct fui_memfs *)fp;
	if(ptr){
		free(ptr);
		ptr = NULL;
		return 0;
	}
	else{
		return -1;
	}

	return 0;
}


/**
* @brief read data from the memory FS pointed by fp.
*
* @param ptr: buffer to store the read data.
* @param size: size to be read.
* @param fp: pointer to the memory buffer FS.
*
* @return On success returns the number of bytes actually read. 
*    On error or to the end,return 0.
*/
long fui_memfs_read(void *fp,unsigned char *ptr, unsigned long size)
{
	char *pos=NULL;
	struct fui_memfs *pfp;
	int nread=0;
	
	if((ptr == NULL) || (fp == NULL)){
		return 0;
	}

	if(size <= 0){
		return 0;
	}

	pfp = (struct fui_memfs *)fp;

	/**
	* end of file.
	*/
	if(pfp->dataoffset == pfp->size){
		return 0;
	}

	/**
	* data not enough.
	*/
	if( (pfp->dataoffset + size) > pfp->size){
		nread = pfp->size - pfp->dataoffset;
		memcpy(ptr,(void *)(pfp->addr + pfp->dataoffset),nread);
		pfp->dataoffset = pfp->size;
		return nread;
	}

	/**
	* has enough data.
	*/
	memcpy(ptr,(void *)(pfp->addr + pfp->dataoffset),size);
	pfp->dataoffset += size;
	return size;
}


/**
* @brief Repositions the file pointer on a memory FS.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the start position.
* @param where: the start position from where to repositon.
*
* @return On success returns 0. On error -1.
*/
long fui_memfs_seek(void *fp, long offset, int where)
{
	int err;
	struct fui_memfs *pfp;
	
	if(fp==NULL){
		return -1;
	}

	pfp = (struct fui_memfs *)fp;

	switch(where){
		case FUI_MEMFS_SEEK_SET:

			/**
			* offset should be greater than 0 
			* if set from beginning.
			*/
			if(offset<0){
				printf("[memfs seek set:offset<0]\n");
				return -1;
			}

			/**
			* if offset too big,error.
			*/
			if(offset>pfp->size){
				printf("[memfs seek set:offset > file size]\n");
				return -1;
			}

			pfp->dataoffset = offset;

			return 0;
			
		break;

		case FUI_MEMFS_SEEK_CUR:

			if(((pfp->dataoffset+offset)<0) || ((pfp->dataoffset+offset)>pfp->size)){
				return -1;
			}
			pfp->dataoffset += offset;

			return 0;
			
		break;

		case FUI_MEMFS_SEEK_END:
			/**
			* offset should be greater than 0 
			* if set from beginning.
			*/
			if(offset<0){
				printf("[memfs seek end:offset<0]\n");
				return -1;
			}

			if((pfp->size - offset) < 0){
				printf("[memfs seek end:offset too big]\n");
				return -1;
			}

			pfp->dataoffset = pfp->size - offset;

			return 0;
			
		break;

		default:
			/// error set command.
		break;
	}

	return -1;
}



/**
* @brief Returns the current file pointer.
*
* @param fp: A pointer to the memory FS.
*
* @return On success returns the current file pointer position. 
*    On error -1.
*/
long fui_memfs_tell(void *fp)
{
	struct fui_memfs *pfp;
	
	if(fp == NULL){
		return -1;
	}

	pfp = (struct fui_memfs *)fp;

	return pfp->dataoffset;
}


/**
* @brief Repositions the file pointer on a memory FS from beginning.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the start position.
*
* @return On success returns 0. On error -1.
*/
long fui_memfs_seek_set(void *fp, long offset)
{
	return fui_memfs_seek(fp, offset,FUI_MEMFS_SEEK_SET);
}


/**
* @brief Repositions the file pointer on a memory FS from current.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the current position.
*
* @return On success returns 0. On error -1.
*/
long fui_memfs_seek_cur(void *fp, long offset)
{
	return fui_memfs_seek(fp, offset,FUI_MEMFS_SEEK_CUR);
}


/**
* @brief Repositions the file pointer on a memory FS from end.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the end position.
*
* @return On success returns 0. On error -1.
*/
long fui_memfs_seek_end(void *fp, long offset)
{
	return fui_memfs_seek(fp, offset,FUI_MEMFS_SEEK_END);
}

