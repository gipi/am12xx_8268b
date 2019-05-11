#ifndef _FLICKR_MAIN_H_
#define _FLICKR_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <flickcurl.h>
#include "curl.h"
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define FLICKR_DEBUG_EN

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	flickcurl_data_write_buffer=0,		///< write the data to the buffer
	flickcurl_data_write_file=1,		///< write the data to the file which had been opened
}FlickcurlDataWrite_e;

typedef struct flickcurl_data_write_s
{
	FlickcurlDataWrite_e data_type;///< where the data to be written, buffer or file
	unsigned char cancel_write:1;	///< 1 : cancel the write, 0 continue
	void * file_handle;
	char *data_head;	///< the pointer to the data buffer
	unsigned int data_len;	///< the total bytes
	char *data_cur;		///< the pointer to the end of the data buffer which is used
	unsigned int data_used;	///< the bytes which had been used
}flickr_data_write_t;

#define flickr_get_status(flickr_rtn,curlcode,httpstatus) (((flickr_rtn)&0xf)<<24|((curlcode)&0xff)<<16|(httpstatus))
#define flickr_get_curlcode(status) ((status)>>16&0xff)
#define flickr_get_httpstatus(status) ((status)&0xffff)
#define flickr_get_iscancel(status) ((status>>24)%0xf)

#ifdef FLICKR_DEBUG_EN
#define flickr_info(fmt,arg...) printf("MINF[%s,%d]:"fmt"\n",__func__,__LINE__,##arg)
#define flickr_err(fmt,arg...) printf("ERR[%s,%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define flickr_info(fmt,arg...) do{}while(0)
#define flickr_err(fmt,arg...) do{}while(0)
#endif

typedef enum{
	FLICKR_RTN_OK,
	FLICKR_RTN_AUTH_ERR,
	FLICKR_RTN_YAHOO_LOGIN_ERR,
	FLICKR_RTN_ACCOUTNTYPE_ERR,
	FLICKR_RTN_GOOGLE_LOGIN_ERR,
	
	FLICKR_RTN_GOOGLE_PF_ERR,			///get google post field err
	FLICKR_RTN_GOOGLE_AUTH_ERR,
	FLICKR_RTN_YAHOO_OPENID_ERR,
	FLICKR_RTN_YAHOO_OPENID_PF_ERR,   ///< post  field err
	FLICKR_RTN_YAHOO_REDIR_ERR,
	
	FLICKR_RTN_YAHOO_AUTHREQ_ERR,
	FLICKR_RTN_YAHOO_AUTHREQ_PF_ERR, ///< post field err
	FLICKR_RTN_YAHOO_AUTHDONE_ERR,    
	FLICKR_RTN_YAHOO_AUTHDONE_PF_ERR, ///< post field err
	FLICKR_RTN_FACEBOOK_LOGIN_ERR,
	
	FLICKR_RTN_FACEBOOK_REDIR_ERR,
	FLICKR_RTN_FACEBOOK_PF_ERR, 			///< post field err
	FLICKR_RTN_FACEBOOK_AUTH_ERR,
	FLICKR_RTN_FACEBOOK_AUTH_PF_ERR, 	///< post field err
	FLICKR_RTN_FACEBOOK_AUTHALLOW_PF_ERR,

	FLICKR_RTN_AUTH_CANCEL,	///< authentication is canceled

	FLICKR_RTN_FROB_ERR,			///< get frob err
	FLICKR_RTN_TOKEN_ERR,			///< get token err
	FLICKR_GET_USERNSID_ERR,		///< get user nisd err
	FLICKR_GET_USERINFO_ERR,		///< get user info err
	FLICKR_GET_PHOTOS_ERR,		///< get photos err
	
	FLICKR_GET_CONTACT_ERR,		///< get contacts err
	FLICKR_GET_PHOHOTSETS_ERR,	///< get photosets err
	FLICKR_GET_PHOTOSETID_ERR,	///< get photoset id err
	FLICKR_GET_PHOTOSET_PHOTOS_ERR,
	FLICKR_RTN_MALLOC_FAIL,		///< malloc space err
	
	FLICKR_RTN_ERR_DOWNLOADPHOTO,
	FLICKR_RTN_CACHE_BUF_ERR,
	FLICKR_RTN_DOWNLOAD_GETPHOTOSET_ERR, 	///< get the photoset addr information err
	FLICKR_RTN_DOWNLOAD_GETPHOTO_ERR,		///< get the photo addr information err
	FLICKR_RTN_DOWNLOAD_FILEBUF_ERR,		///< get file name which to be created failed
	
	FLICKR_RTN_DOWNLOAD_FILE_EXIST,			///< the file which will be downloaded had existed
	FLICKR_RTN_DOWNLOAD_FILE_CREATE_FAIL,	///< create the file err
	FLICKR_RTN_PATH_CACHEDIR_ERR,
	FLICKR_RTN_PATH_ALBUMID_ERR,
}flickr_rtn_e;

typedef enum{
	///< see http://code.google.com/intl/zh-CN/apis/gdata/docs/2.0/reference.html#HTTPStatusCodes
	FLICKR_HttpStatus_OK=200,		///< No error.
	FLICKR_HttpStatus_CREATED=201,		///< Creation of a resource was successful.
	FLICKR_HttpStatus_NOT_MODIFIED=304,	///< The resource hasn't changed since the time specified in the request's If-Modified-Since header.
	FLICKR_HttpStatus_BAD_REQUEST=400,	///< Invalid request URI or header, or unsupported nonstandard parameter.
	FLICKR_HttpStatus_UNAUTHORIZED=401,	///< Authorization required.
	FLICKR_HttpStatus_FORBIDDEN=403,	///< Unsupported standard parameter, or authentication or authorization failed.
	FLICKR_HttpStatus_NOT_FOUND=404,	///< Resource (such as a feed or entry) not found.
	FLICKR_HttpStatus_CONFLICT=409,	///< Specified version number doesn't match resource's latest version number.
	FLICKR_HttpStatus_GONE=410,		///< Requested change history is no longer available on the server. Refer to service-specific
	FLICKR_HttpStatus_SERVER_ERROR=500,	///< Internal error. This is the default code that is used for all unrecognized server errors.
}flickr_httpstatus_e;

typedef struct flickr_data_atomic_s
{
	char * data;		///< the pointer to the data
	unsigned int data_len;	///< the length of the data
}flickr_data_atomic_t;


typedef enum
{
	ACCOUNT_TYPE_GOOGLE,		///< using google to login
	ACCOUNT_TYPE_YAHOO,		///< using yahoo to login
	ACCOUNT_TYPE_FACEBOOK,	///< using facebook to login
}flickr_account_type_e;


typedef enum
{
	GET_ACCOUNT_TYPE,  				///< get account type  see flickr_account_type_e, should change to int by the caller
	GET_LOGIN_EMAIL,					///< get login email
	GET_LOGIN_PWD,					///< get login password
	GET_FROB,							///< get frob
	GET_PERMS,							///< get perms
	GET_USER_REALNAME,				///< user's realname
	GET_USER_USERNAME,				///< user's username
	GET_CONTACT_COUNT,			///< how many contacts does the login account has, should change to int by the caller
	GET_CONTACT_REALNAME,			///< contact's realname
	GET_CONTACT_USERNAME,			///< contact's username
	GET_PHOTOSET_ID,					///< photoset id
	GET_PHOTOSET_PHOTOS_COUNT, 		///< photos' num in a photoset, should change to int by the caller
	GET_PHOTOSET_TITLE,				///< title of the photoset		
	GET_PHOTOSET_DESCRIPTION,			///< description of the photoset
	GET_PHOTOSET_COUNT,				///< how many photosets in the account, should change to int by the caller
	GET_PHOTOS_COUNT,					///< photos num in the account,  should change to int by the caller
	GET_PHOTO_TITLE,					///< photo title
	GET_PHOTO_DESCRIPTION,			///< photo description
	GET_PHOTO_ID,						///< photo id
	GET_PHOTO_DATESPOSTED,			///< photo dates posted
	GET_PHOTO_DATESTAKEN,			///< photo dates taken
	GET_PHOTO_DATESLASTUPDATE,		///< photo dates last update

	GET_URL_COVER_SMALL,				///< get photoset cover url, small picture
	GET_URL_COVER_HUGE,				///< get photoset cover url, huge picture
	GET_URL_PHOTO_SMALL,				///< get photo url, small picture
	GET_URL_PHOTO_HUGE,				///< get photo url, huge picture
}flickr_info_get_cmd_e;

typedef struct flickr_info_query_s
{
	int which_contact;		///< which contact will be gotten, -1 means the login account
	int which_photoset;		///< which photoset will be gotten	-1 means the photos all contains, others the photoset
	int which_photo;			///< which photo will be gotten, 
	int err_status;			///< the err_status returned 
}flickr_info_query_t;

typedef enum
{
	PERMS_READ,				///< permission to read private information 
	PERMS_WRITE,				///<  permission to add, edit and delete photo metadata (includes 'read') 
	PERMS_DELETE,				///< permission to delete photos (includes 'write' and 'read') 
}flickr_perms_type_e;

typedef enum
{
///< Safe search setting: 1 for safe, 2 for moderate, 3 for restricted. (Please note: Un-authed calls can only see Safe content.)
	FLICKR_SEARCH_SAFE=1,
	FLICKR_SEARCH_MODERATE=2,
	FLICKR_SEARCE_RESTRICTED=3,
}flickr_search_mode_e;

typedef enum
{
 ///<Content Type setting: 1 for photos only, 2 for screenshots only, 3 for 'other' only, 4 for photos and screenshots, 5 for screenshots and 'other', 6 for photos and 'other', 7 for photos, screenshots, and 'other' (all) 
	FLICKR_PHOTOONLY=1,
	FLICKR_SCREENSHOTSONLY=2,
	FLICKR_OTHREONLY=3,
	FLICKR_PHOTOANDSCREENSHOT=4,
	FLICKR_SCREENSHOTOTHER=5,
	FLICKR_PHOTOOTHER=6,
	FLICKR_ALL=7,
}flickr_content_type_e;

typedef enum
{
///<1 public photos, 2 private photos visible to friends, 3 private photos visible to family, 4 private photos visible to friends & family, 5 completely private photos
	FLICKR_PRI_NONE=-1, ///< both the public and the private photos will be gotten
	FLICKR_PUBLIC=1,
	FLICKR_PRI_FRIENDS=2,
	FLICKR_PRI_FAMILY=3,
	FLICKR_PRI_FRIENDS_FAMILY=4,
	FLICKR_PRI_COMPLETE=5,
}flickr_privacy_fliter_e;

typedef enum
{
	FLCKR_PHOTOSIZE_NONE,
	FLICKR_PHOTOSIZE_S, 			///< little square 75*75
	FLICKR_PHOTOSIZE_T,			///< thumbnail max side 100
	FLICKR_PHOTOSIZE_M,			///< little pic max side 240
	FLICKR_PHOTOSIZE_Z,			///< max side 640
	FLICKR_PHOTOSIZE_B,			///< big size max 1024
	FLICKR_PHOTOSIZE_O,			///< original size , you should have the format first, jpg/png/gif 
}flickr_photosize_e;

typedef enum
{
	FLICKR_CONTACT_FRIENDS,		///< Only contacts who are friends (and not family)
	FLICKR_CONTACT_FAMILY,		///< Only contacts who are family (and not friends)
	FLICKR_CONTACT_BOTH,		///< Only contacts who are both friends and family
	FLICKR_CONTACT_NEITHER,		///< Only contacts who are neither friends nor family
	FLICKR_CONTACT_ALL,
}flickr_contact_filter_e;


///<A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: 
///<description, license, date_upload, date_taken, owner_name, icon_server, original_format, last_update, geo,
///<tags, machine_tags, o_dims, views, media, path_alias, url_sq, url_t, url_s, url_m, url_z, url_l, url_o 

#define 	FLICKR_EXTRA_DESCRIPTION		0x00000001
#define	FLICKR_EXTRA_LICENSE			0x00000002
#define	FLICKR_EXTRA_DATAUPLOAD		0x00000004
#define	FLICKR_EXTRA_DATETAKEN		0x00000008
#define	FLICKR_EXTRA_OWNERNAME		0x00000010
#define	FLICKR_EXTRA_ICONSERVER		0x00000020
#define	FLICKR_EXTRA_ORIGINALFORMAT	0x00000040
#define 	FLICKR_EXTRA_LASTUPDATE		0x00000080
#define 	FLICKR_EXTRA_GEO				0x00000100
#define 	FLICKR_EXTRA_TAGS				0x00000200
#define   FLICKR_EXTRA_MACHINETAGS		0x00000400
#define   FLICKR_EXTRA_ODIMS			0x00000800
#define   FLICKR_EXTRA_VIEWS			0x00001000
#define   FLICKR_EXTRA_MEDIA			0x00002000
#define   FLICKR_EXTRA_PATHALIAS		0x00004000
#define   FLICKR_EXTRA_URLSQ			0x00008000
#define   FLICKR_EXTRA_URLT				0x00010000
#define 	FLICKR_EXTRA_URLS				0x00020000
#define   FLICKR_EXTRA_URLM				0x00040000
#define   FLICKR_EXTRA_URLZ				0x00080000
#define	FLICKR_EXTRA_URLL				0x00100000
#define	FLICKR_EXTRA_URLO				0x00200000

#define	FLICKR_EXTRA_MAX_PARA		22

typedef enum{
	FLICKR_CMD_DOWNLOADPHOTO=0,		///< cmd for download photo
	FLICKR_CMD_STOPDOWNLOAD=1,			///< cmd for canceling download
	FLICKR_CMD_AUTH=4,					///< cmd for authentication
	FLICKR_CMD_STOPAUTH=5,				///< cmd for caceling authentication
}flickr_ioctl_cmd_e;

typedef enum{
	FLICKR_REQ_INVALID,
	FLICKR_REQ_ENQUEUE,
	FLICKR_REQ_DEQUEUE,
	FLICKR_REQ_DONE,
}flickr_req_status_e;

typedef enum{
	FLICKR_QUERY_CMD_RESULT,    ///< query the result 
	FLICKR_QUREY_CMD_PROGRESS,	///< query the progress
}flickr_query_cmd;

typedef enum{
	FLICKR_QUERY_STATUS_PARAERR=-2,  ///< the prarameters is error! 
	FLICKR_QUERY_STATUS_DOING=-1,	///< the task is doing 
}flickr_query_status_e;

typedef struct flickr_ioctrl_s
{
	flickr_req_status_e active;
	unsigned long timestamp;
	flickr_ioctl_cmd_e iocmd;
	void *para;
	int para_size;
	
}flickr_ioctrl_t;

typedef struct flickr_semi_s{
	sem_t semi_req;		///< lock the queue
	sem_t semi_start;		///< sync the downloding task
	sem_t semi_end;		///< ///< sync the downloding task
}flickr_semi_t;///< for synchronous downloading photo 

typedef struct flickr_task_info_s
{
	flickr_semi_t syn_lock; ///< for the thread sync 
	pthread_t thread_id;
	unsigned char is_thread_run:1;///<  note whether the thread is run
	int req_start_idx; ///< the req start idx
	unsigned int cnt_flickr_instance; ///< the number of the filckr instance
}flickr_task_info_t;

typedef struct flickr_photosets_s
{
	int counts;		///< how many photosets in the user account
	flickcurl_photoset** photoset_list;	///< photoset list	
	flickcurl_photos_list**photos_list_array;   ///< the photos in the a photoset, the length of the array is <counts>
}flickr_photosets_t;

typedef struct flickr_infors_s
{
	flickcurl_person * user_info;		///< user_info under the account
	flickcurl_photos_list* photos_list; 	///< photos under the account, total numbers of the photos will be gotten
	flickr_photosets_t photosets;		///< photosets under the account
	
}flickr_infors_t;

typedef struct flickr_contacts_s
{
	int counts;						///< how many contacts gets
	flickcurl_contact **pcontacts;		///< contacts list
	flickr_infors_t * infors_array;				///< each contact has his own informations, the array has <counts> members
}flickr_contacts_t;



typedef struct flickr_gdata_s
{
	flickcurl *fc;
	
	flickr_data_atomic_t user_email;    				///< login email
	flickr_data_atomic_t user_pwd;					///< login password
	flickr_account_type_e account_type;			///< account type
	
	flickr_data_atomic_t user_nsid;					///< nsid of the login account
	flickr_data_atomic_t frob;  						///< get from flickcurl_auth_getFrob()
	flickr_data_atomic_t perms;  					///< perms of auth, read, write or delete 
	char is_auth_cancel;							///< used for canceling authentication
	flickr_data_atomic_t auth_frob; 					///< get from the authentication
	flickr_data_atomic_t auth_token; 				///< token get  from authentication

	flickr_contacts_t contacts;						///< the contacts information
	flickr_infors_t infors;							///< the owner's information
	unsigned int err_status;							///< err_status when the function is called
	flickr_task_info_t * task_info;
}flickr_gdata_t;

typedef struct flickr_path_s
{
	flickr_data_atomic_t cache_dir;		///< cache dir, it is specified by the caller
	flickr_data_atomic_t album_id;		///< get from the photoset id
	flickr_data_atomic_t photo_name;	///< get from photo
}flickr_path_t;


typedef struct flickr_download_info_s{
	unsigned char needcache:1;
	unsigned char workdone:1;
	unsigned char isthumbnail:1;
	int prog_down;
	int status;
	flickr_data_atomic_t photo_url;
	flickr_data_write_t photo_data;
	flickr_path_t img_path;
	flickr_gdata_t *gdata;
}flickr_download_info_t;

#define FLICKR_TMP_BUF_LEN 256
#define FLICKR_CURL_TIMEOUT 60			///< unit is second

void flickr_set_login_info(flickr_gdata_t *gdata,const char*user_email,const char* user_pwd,flickr_account_type_e act_type);


flickr_gdata_t *flickr_init_gdata();
void flickr_free_gdata(flickr_gdata_t *gdata);
int  flickr_auth(flickr_gdata_t *gdata);


char * flickr_get_info(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query);
int flickr_get_user_info(flickr_gdata_t *gdata,int which_contact);
void flickr_free_user_info(flickr_gdata_t*gdata,int which_contact);

int flickr_get_photos_info(flickr_gdata_t *gdata,int which_contact,flickr_privacy_fliter_e priv_level);
void flickr_free_photos_info(flickr_gdata_t *gdata,int which_contact);

int flickr_get_photosets_info(flickr_gdata_t *gdata,int which_contact);
void flickr_free_photosets_info(flickr_gdata_t* gdata,int which_contact);

int flickr_get_photosets_photos_info(flickr_gdata_t *gdata,int which_contact,int which_photoset,flickr_privacy_fliter_e priv_level);
void flickr_free_photosets_photos_info(flickr_gdata_t *gdata,int which_contact,int which_photoset);


int flickr_get_contacts(flickr_gdata_t *gdata);
void flickr_free_contacts(flickr_gdata_t *gdata);

flickr_download_info_t *flickr_init_download_info(int iscache,char* cache_dir,flickr_gdata_t * gdata,int which_contact,int which_photoset,int which_photo,int isthumbnail);
int flickr_free_download_info(flickr_download_info_t* down_info);
int flickr_download_photo(flickr_gdata_t *gdata,flickr_download_info_t *down_info);

int flickr_get_cache_path(flickr_gdata_t * gdata,int which_contact,int which_photoset,int which_photo,int isthumbnail,char *pathbuf,int buf_len);

int flickr_send_msg(flickr_gdata_t *gdata,flickr_ioctl_cmd_e cmd,void * para);
int flickr_query_download_status(flickr_gdata_t *gdata,flickr_download_info_t * down_info,flickr_query_cmd query_cmd);

#ifdef __cplusplus
}
#endif

#endif
