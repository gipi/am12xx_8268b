#ifndef _FACEBOOK_COMMON_H_

#define _FACEBOOK_COMMON_H_

#include "webalbum_api.h"

size_t facebook_func_write_data(void *ptr, size_t size, size_t nmemb, void * userdata);
char * facebook_malloc(unsigned int size);
int facebook_free(void * buffer);
char * facebook_realloc(char *ptr,unsigned int newsize);
int facebook_fflush(void *fp);
int facebook_fremove(char* file);
int facebook_get_ini_albumsname(facebook_data *f_data,char* cache_dir,char*filename,char* namebuf,int len);
int facebook_get_ini_albumname(facebook_photoalbums *feed_albums,int which_album,char* cache_dir,char* namebuf,int len);
int facebook_get_ini_contactname(facebook_data *f_data,char* cache_dir,char* namebuf,int len);
int facebook_reset_atomic(facebook_atomic *atomic,unsigned int *offset);
int facebook_reset_from(facebook_from *from,unsigned int *offset);
int facebook_reset_comments(facebook_comment *comments,unsigned int *offset);
int facebook_reset_images(facebook_picture *images,unsigned int *offset);
int facebook_reset_albumsentry(facebook_album *album,unsigned int offset);
int facebook_reset_photoentry(facebook_photo *photo,unsigned int offset);
int facebook_reset_memberentry(facebook_member *member,unsigned int offset);
int facebook_write_atomic(void *file_handle,facebook_atomic * atomic);
int facebook_write_from(void *file_handle,facebook_from *from);
int facebook_write_comments(void *file_handle,facebook_comment *comments);
int facebook_write_images(void *file_handle,facebook_picture *images);
int facebook_write_albumsentry(void *file_handle,unsigned int offset,facebook_album *album);
int facebook_write_photoentry(void *file_handle,unsigned int offset,facebook_photo *photo);
int facebook_write_memberentry(void *file_handle,unsigned int offset,facebook_member *member);
int facebook_write_albumsfeed(void *file_handle,unsigned int offset,facebook_photoalbums *albums);
int facebook_write_albumfeed(void *file_handle,unsigned int offset,facebook_album *album);
int facebook_write_contactsfeed(void *file_handle,unsigned int offset,facebook_contact *contacts);
int facebook_read_atomic(void *file_handle,facebook_atomic *atomic);
int facebook_read_from(void *file_handle,facebook_from * from);
int facebook_read_comments(void *file_handle,facebook_comment * comments);
int facebook_read_images(void *file_handle,facebook_picture *images);
int facebook_read_albumsentry(void* file_handle,unsigned int offset,facebook_album * album);
int facebook_read_photoentry(void* file_handle,unsigned int offset,facebook_photo * photo);
int facebook_read_memberentry(void* file_handle,unsigned int offset,facebook_member * member);
int facebook_read_albumsfeed(void * file_handle,unsigned int offset,facebook_photoalbums * albums);
int facebook_read_contactsfeed(void * file_handle,unsigned int offset,facebook_contact* contacts);
int facebook_printf_atomic(char* name,facebook_atomic *atomic_name);
int facebook_printf_from(facebook_from *from);
int facebook_printf_comments(facebook_comment *comments);
int facebook_printf_images(facebook_picture *images);
int facebook_printf_albumsentry(facebook_album *album);
int facebook_printf_photoentry(facebook_photo *photo);
int facebook_printf_memberentry(facebook_member *member);
int facebook_printf_albumsfeed(facebook_photoalbums *feed_albums);
int facebook_printf_contactsfeed(facebook_contact *feed_contacts);

#endif
