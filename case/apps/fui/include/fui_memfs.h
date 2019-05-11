#ifndef _FUI_MEMFS_H
#define _FUI_MEMFS_H

/**
* @file fui_memfs.h
* @brief This head file describes the memory file operation
*    API functions.
* @author: Simon Lee
* @date: 2010-11-17
* @version: 0.1
*/

/**
* @addtogroup CASE_lib
* @{
*/

#define FUI_MEMFS_SEEK_SET	0
#define FUI_MEMFS_SEEK_CUR	1
#define FUI_MEMFS_SEEK_END	2

struct fui_memfs{
	/// the address of the memory buffer.
	char *addr;
	/// size of this memory buffer.
	long size;
	/// offset of the current data.
	long dataoffset;
};



/**
* @brief open a memory buffer to operate.
*
* @param buffer: buffer to memory.
* @param size: size of the buffer.
*
* @return if open success, return a pointer to "struct fui_memfs",
*    if open failed,return NULL.
*/
extern void *fui_memfs_open(char *buffer, long size);


/**
* @brief Close a memory FS.
*
* @param fp: A pointer to a memory FS that previously opened.
*
* @return On success returns 0. On error returns -1.
*/
extern long fui_memfs_close(void *fp);


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
extern long fui_memfs_read(void *fp,unsigned char *ptr, unsigned long size);


/**
* @brief Repositions the file pointer on a memory FS.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the start position.
* @param where: the start position from where to repositon.
*
* @return On success returns 0. On error -1.
*/
extern long fui_memfs_seek(void *fp, long offset, int where);



/**
* @brief Returns the current file pointer.
*
* @param fp: A pointer to the memory FS.
*
* @return On success returns the current file pointer position. 
*    On error -1.
*/
extern long fui_memfs_tell(void *fp);

/**
* @brief Repositions the file pointer on a memory FS from beginning.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the start position.
*
* @return On success returns 0. On error -1.
*/
extern long fui_memfs_seek_set(void *fp, long offset);


/**
* @brief Repositions the file pointer on a memory FS from current.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the current position.
*
* @return On success returns 0. On error -1.
*/
extern long fui_memfs_seek_cur(void *fp, long offset);


/**
* @brief Repositions the file pointer on a memory FS from end.
*
* @param fp: pointer to the memory FS.
* @param offset: offset from the end position.
*
* @return On success returns 0. On error -1.
*/
extern long fui_memfs_seek_end(void *fp, long offset);


/**
 * @}
 */

#endif /** _FUI_MEMFS_H */

