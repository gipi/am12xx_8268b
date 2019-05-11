#ifndef _FACEBOOK_JSON_ANALY_H_

#define _FACEBOOK_JSON_ANALY_H_

#include "webalbum_api.h"
#include "facebook_common.h"

#define CURL_TIME_OUT 60	//the unit is second
#define BUF_LENGTH	256

#if 1
char * delete_char(char * str);

char * delete_quotation(char * str);

int facebook_get_keyindex(char * key);

int facebook_func_prog(void *p, double dltotal, double dlnow, double ult, double uln);

int get_picture(CURL * curl_p, char * location);

int facebook_fill_atomic(const char * value,facebook_atomic * atomic_t);

int facebook_fill_fromelement(char * key, json_object * obj, facebook_from * from_t);

int analy_each_from_info(json_object * obj, facebook_from * from_t);

int facebook_fill_commentelement(char * key, json_object * obj, facebook_comment * comments);

int analy_each_comment_info(json_object * obj, facebook_comment * comments);

int analy_comment_info(json_object * obj,facebook_comment * comments);

int facebook_fill_albumelement(char * key, json_object * obj,facebook_album * albuminfo);

int analy_each_album_info(facebook_album * albuminfo, json_object * each_obj);

facebook_photoalbums * analy_albums_info(char * str);

facebook_photoalbums * get_albums_info(facebook_data * f_data, char * id);

facebook_photoalbums * facebook_get_albums_info(facebook_data *f_data, facebook_contact *contact, int which_friend);

char * get_album_id(facebook_album * albuminfo);

int get_album_cover(CURL * curl_p, facebook_data * f_data, facebook_album * albuminfo);

int facebook_fill_imageelement(char * key, json_object * obj, facebook_picture * image);

int analy_each_image_info(json_object * obj, facebook_picture * image);

int analy_images_info(json_object * obj,facebook_picture * image);

int facebook_fill_photoelement(char * key, json_object * obj, facebook_photo * photoinfo);

int analy_each_photo_info(facebook_photo * photoinfo, json_object * each_obj);

int analy_photo_info(facebook_photo * index, char * str);

int get_albumphoto_info(facebook_data * f_data, int index, facebook_photoalbums * photoalbums);

int download_photo_big(CURL * curl_p, facebook_album * album_entry, int index);

int download_photo_small(CURL * curl_p, facebook_album * album_entry, int index);

int facebook_fill_userelement(char * key, json_object * obj,facebook_user * userinfo);

facebook_user * analy_user_info(char * str);

facebook_user * get_user_info(facebook_data * f_data);

int get_user_picture(CURL * curl_p, facebook_data * f_data, facebook_user * userinfo);

int facebook_fill_friendlistelement(char * key, json_object * obj, facebook_friendlist * friendlist);

int analy_each_friendlist(facebook_friendlist * friendlist, json_object * obj);

facebook_friendlists * analy_friends_list(char * str);

int facebook_fill_memberelement(char * key, json_object * obj, facebook_member * member);

int analy_each_member(facebook_member * member, json_object *obj);

int analy_listmembers(facebook_friendlist * friendlist_entry, char * str);

int get_listmembers_info(facebook_data * f_data, facebook_friendlist * friendlist_entry);

facebook_friendlists * get_friends_list(facebook_data * f_data);

facebook_contact * analy_contact(char * str);

facebook_contact * get_contact(facebook_data * f_data);

int facebook_init_fdata(facebook_data * f_data);

int facebook_free_fdata(facebook_data * f_data);

int facebook_free_photoalbums(facebook_photoalbums *photoalbums);

int facebook_free_atomic(facebook_atomic * atomic);

int facebook_free_from(facebook_from * from);

int facebook_free_comments(facebook_comment *comments);

int facebook_free_photoimage(facebook_picture * images);

int facebook_free_photo(facebook_photo * photo);

int facebook_free_basicalbum(facebook_album * album);

int facebook_free_album(facebook_album * album);

int facebook_free_photoalbums(facebook_photoalbums *photoalbums);

int facebook_free_memberlist(facebook_member *member);

int facebook_free_friendlist(facebook_friendlist *friendlist);

int facebook_free_friendlists(facebook_friendlists *friendlists);

int facebook_free_contact(facebook_contact *contact);

int facebook_free_userinfo(facebook_user *userinfo);

int facebook_data_write_init(facebook_write_data *data,FacebookDataWrite_e data_type,void* fp);

int facebook_data_write_free(facebook_write_data *data);

int debugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream);

int facebook_create_dir(char * dir_path);

int facebook_set_longin(const char* email, const char* pwd);

int facebook_init_fdata(facebook_data *fdata);

int facebook_free_fdata(facebook_data * f_data);

int facebook_authentication(facebook_data * f_data);

int facebook_query_auth_status(facebook_data * f_data, int query_cmd);

facebook_photo_down_info *facebook_init_download_info(int iscache,char* cache_dir,facebook_feed *feed,int which_entry,int isthumbnail);

int facebook_free_download_info(facebook_photo_down_info* down_info);

facebook_feed * facebook_feed_type(facebook_type type, void * feed_p);

int facebook_free_type(facebook_feed * f_feed);

int facebook_download_photo(facebook_data * f_data,	facebook_photo_down_info * f_down_info);

char * upload_feed(facebook_data * f_data, char * profile_id, char * message);

char * upload_comments(facebook_data * f_data, char * id, char * message);

char * create_album(facebook_data * f_data, char * name, char * message);

void  facebook_sem_wait(sem_t *sem);

void  facebook_sem_post(sem_t *sem);

int  facebook_get_msg(facebook_data *gdata,facebook_ioctrl * req);

void  *facebook_thread(void *arg);

int facebook_create_thread(facebook_data * f_data);

int facebook_thread_exit(facebook_data *f_data);

unsigned long facebook_get_timestamp();

int facebook_send_msg(facebook_data *f_data,facebook_ioctl_cmd cmd,void * para);

int facebook_query_download_status(facebook_data *f_data,facebook_photo_down_info * down_info,FacebookwebQueryCmd query_cmd);

int facebook_get_cache_path(facebook_feed *feed,int which_entry,int isthumbnail,char *pathbuf,int buf_len);

int facebook_save_albumsfeed(facebook_data *f_data,facebook_photoalbums * feed_albums,char* filename);

int facebook_save_albumfeed(facebook_data *f_data,facebook_album * feed_album,char* filename);

int facebook_save_contactfeed(facebook_data *f_data,facebook_contact * feed_contact,char* filename);

facebook_photoalbums * facebook_load_albumsfeed(facebook_data *gdata,char* filename);

int facebook_load_albumfeed(facebook_data *gdata,char* filename,facebook_album * album);

facebook_contact * facebook_load_contactsfeed(facebook_data *f_data,char* filename);

int facebook_save_albums_info(facebook_data *f_data,facebook_photoalbums * feed_albums,char* cache_dir,char* filename);

int facebook_save_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir);

int facebook_save_contact_info(facebook_data *f_data,facebook_contact * feed_contact,char* cache_dir);

facebook_photoalbums * facebook_load_albums_info(facebook_data *f_data,char* cache_dir,char *filename);

int facebook_load_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir);

facebook_contact* facebook_load_contact_info(facebook_data *f_data,char* cache_dir);
#endif
#endif
