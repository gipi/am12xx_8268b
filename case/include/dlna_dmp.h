#ifndef DLNA_DMP_H_
#define DLNA_DMP_H_


extern int dlna_start_work(void*handle);
extern int dlna_stop_work(void*handle);
extern int dlna_dmp_get_status(void *handle);
extern int dlna_dmp_get_total(void * handle);
extern char* dlna_dmp_get_name(int index);
extern char* dlna_dmp_get_uri(int index);
extern int dlna_dmp_enter(int index);
extern int dlna_dmp_escape(void *handle);
extern int dlna_dmp_get_obj_type(int index);
extern int dlna_download_file(int index,char* save_path);
extern int dlna_download_GetTotalBytesExpected(void*handle);
extern int dlna_download_GetBytesReceived(void*handle);
extern int dlna_get_download_status(void*handle);
extern int dlna_start_DMS(void*handle);
extern  int dlna_stop_DMS(void*handle);

 
#endif 



