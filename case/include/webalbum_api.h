#ifndef _WEBALBUM_API_H_
#define _WEBALBUM_API_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "json/json.h"

#define PICASA_WEBALBUM_DEBUG

//picasa_rtn see PicasawebRtn_e, curlcode see CURLcode, httpstatus see PicasawebHttpStatus_e
#define get_status(picasa_rtn,curlcode,httpstatus) (((picasa_rtn)&0xf)<<24|((curlcode)&0xff)<<16|(httpstatus))
#define get_curlcode(status) ((status)>>16&0xff)
#define get_httpstatus(status) ((status)&0xffff)
#define get_iscancel(status) ((status>>24)%0xf)


#ifdef PICASA_WEBALBUM_DEBUG
#define picasa_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define picasa_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define picasa_info(fmt,arg...) do{}while(0);
#define picasa_err(fmt,arg...) do{}while(0);
#endif

#define TARGET_DISK "/mnt/udisk/"
#define FREE_SPACE	20		//20M
#define UPDATE_INTERVAL	3600	//3600s,1hour

typedef enum{
	PICASA,
	FACEBOOK,
	FLICKR_YAHOO,	///< use yahoo account to login flickr
	FLICKR_PICASA,	///< use picasa account to login flickr
	FLICKR_FACEBOOK,	///< use facebook account to login flickr
}Webalbum_type;

typedef enum{
	FEED_ALBUMS,
	FEED_PHOTOS,	
	FEED_CONTACT,
}PicasawebFeedKind_e;

typedef enum{
	Picasaweb_data_write_buffer=0,		///< write the data to the buffer
	Picasaweb_data_write_file=1,		///< write the data to the file which had been opened
}PicasawebDataWrite_e;

typedef enum{
	///< see http://code.google.com/intl/zh-CN/apis/picasaweb/docs/2.0/reference.html#Path_Values
	Kind_album=0,		///< Feed includes some or all of the albums the specified user has in their gallery.
	Kind_photo=1,		///< Feed includes the photos in an album (album-based), recent photos uploaded by a user (user-based) or 
	Kind_comment=2,		///< Feed includes the comments that have been made on a photo.
	Kind_tag=3,		///< Includes all tags associated with the specified user, album, or photo. For user-based and album-based 					
	Kind_user=4,	///< Feed includes entries representing users.
	Kind_MAX,
}PicasawebKindValue_e;

enum{
	PICASA_RTN_OK,	//nothing error happend
	PICASA_RTN_ERR_LOGIN,
	PICASA_RTN_ERR_CREATALBUM,
	PICASA_RTN_ERR_DELETEALBUM,
	PICASA_RTN_ERR_UPDATEALBUM,
	PICASA_RTN_ERR_UPLOAD_OPENFILE,
	PICASA_RTN_ERR_UPLOADPHOTO,
	PICASA_RTN_ERR_UPDATEPHOTO,
	PICASA_RTN_ERR_DELETEPHOTO,
	PICASA_RTN_ERR_DOWNLOADPHOTO,
	PICASA_RTN_ERR_MEMORY, // memory error
	PICASA_RTN_ERR_PARA,
	PICASA_RTN_CANCEL, // the process had been canceled
}PicasawebRtn_e;

typedef enum{
	///< see http://code.google.com/intl/zh-CN/apis/gdata/docs/2.0/reference.html#HTTPStatusCodes
	HttpStatus_OK=200,		///< No error.
	HttpStatus_CREATED=201,		///< Creation of a resource was successful.
	HttpStatus_FOUND=302,		///< The data requested actually resides under a different URL, however, the redirection may be altered on occasion as for "Forward".
	HttpStatus_NOT_MODIFIED=304,	///< The resource hasn't changed since the time specified in the request's If-Modified-Since header.
	HttpStatus_BAD_REQUEST=400,	///< Invalid request URI or header, or unsupported nonstandard parameter.
	HttpStatus_UNAUTHORIZED=401,	///< Authorization required.
	HttpStatus_FORBIDDEN=403,	///< Unsupported standard parameter, or authentication or authorization failed.
	HttpStatus_NOT_FOUND=404,	///< Resource (such as a feed or entry) not found.
	HttpStatus_CONFLICT=409,	///< Specified version number doesn't match resource's latest version number.
	HttpStatus_GONE=410,		///< Requested change history is no longer available on the server. Refer to service-specific
	HttpStatus_SERVER_ERROR=500,	///< Internal error. This is the default code that is used for all unrecognized server errors.
}PicasawebHttpStatus_e;

typedef enum{
	QUERY_CMD_RESULT,    ///< query the result 
	QUREY_CMD_PROGRESS,	///< query the progress
}PicasawebQueryCmd;

#if 0
typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_OBSOLETE4,               /* 4 - NOT USED */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                    due to lack of access - when login fails
                                    this is not returned. */
  CURLE_OBSOLETE10,              /* 10 - NOT USED */
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_OBSOLETE12,              /* 12 - NOT USED */
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_OBSOLETE16,              /* 16 - NOT USED */
  CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_OBSOLETE20,              /* 20 - NOT USED */
  CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_OBSOLETE24,              /* 24 - NOT USED */
  CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
  CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  /* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
           instead of a memory allocation error if CURL_DOES_CONVERSIONS
           is defined
  */
  CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
  CURLE_OBSOLETE29,              /* 29 - NOT USED */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_OBSOLETE32,              /* 32 - NOT USED */
  CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_OBSOLETE40,              /* 40 - NOT USED */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_OBSOLETE44,              /* 44 - NOT USED */
  CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
  CURLE_OBSOLETE46,              /* 46 - NOT USED */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_TELNET_OPTION,   /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE50,              /* 50 - NOT USED */
  CURLE_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
                                     wasn't verified fine */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized transfer encoding */
  CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
  CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
  CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
  CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                    that failed */
  CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
  CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                    accepted and we failed to login */
  CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
  CURLE_TFTP_PERM,               /* 69 - permission problem on server */
  CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
  CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
  CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
  CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
  CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
  CURLE_CONV_FAILED,             /* 75 - conversion failed */
  CURLE_CONV_REQD,               /* 76 - caller must register conversion
                                    callbacks using curl_easy_setopt options
                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                    CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
  CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                    or wrong format */
  CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
  CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                    generic so the error message will be of
                                    interest when this has happened */

  CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                    connection */
  CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                    wait till it's ready and try again (Added
                                    in 7.18.2) */
  CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                    wrong format (Added in 7.19.0) */
  CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                    7.19.0) */
  CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
  CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
  CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Identifiers */
  CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
  CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */

  CURL_LAST /* never use! */
} CURLcode;

#endif
typedef struct picasaweb_feed_s picasaweb_feed_t;

typedef struct picasa_semi_s{
	sem_t semi_req;
	sem_t semi_start;
	sem_t semi_end;
}picasa_semi_t;

typedef struct picasaweb_gdata_s
{
	char*login_name;	///< login name, it is an email addr
	char*login_pwd;		///< password 
	char*auth;		///< auth return from the server
	int auth_ok;		///< the authentication error code, see Picasaweb_Auth_BadAuthentication etc
	int login_name_len;	///< the length of the login name
	int login_pwd_len;	///< the length of the password
	int auth_len;		///< the length of the auth
	CURL *curl;
	picasa_semi_t syn_lock; ///< for the thread sync 
	pthread_t thread_id;
	unsigned is_thread_run:1;///<  note whether the thread is run
	int req_start_idx; ///< the req start idx

	pthread_t update_thread_id;
}picasaweb_gdata_t;


typedef struct picasaweb_data_write_s
{
	PicasawebDataWrite_e data_type;///< where the data to be written, buffer or file
	unsigned char cancel_write:1;	///< 1 : cancel the write, 0 continue
	void * file_handle;
	char *data_head;	///< the pointer to the data buffer
	unsigned int data_len;	///< the total bytes
	char *data_cur;		///< the pointer to the end of the data buffer which is used
	unsigned int data_used;	///< the bytes which had been used
}picasaweb_data_write_t;


typedef struct picasaweb_data_atomic_s
{
	char * data;		///< the pointer to the data
	unsigned int data_len;	///< the length of the data
}picasaweb_data_atomic_t;


typedef struct picasaweb_author_s
{
	picasaweb_data_atomic_t name;
	picasaweb_data_atomic_t email;
	picasaweb_data_atomic_t uri;
}picasaweb_author_t;

////< media:group element
typedef struct picasaweb_media_content_s
{
	picasaweb_data_atomic_t url;	///< The URL of the full version of the media for the current entry.
	picasaweb_data_atomic_t type;	///< The MIME type of the media.
	picasaweb_data_atomic_t medium;	///< Either image or video.
	picasaweb_data_atomic_t height;	///< The height of the image or video 
	picasaweb_data_atomic_t width;	///< The width of the image or video
	picasaweb_data_atomic_t filesize;	///< The file size in bytes of the image or video
}picasaweb_media_content_attr_t;	///< Container element for all media elements.

typedef struct picasaweb_media_thumbnail_s
{
	char is_stored;	///< maybe there are some thumbnail can be choose to store,just choose one 
	picasaweb_data_atomic_t url;	///< The URL of the thumbnail image.
	picasaweb_data_atomic_t height;	///< The height of the thumbnail image.
	picasaweb_data_atomic_t width;	///< The width of the thumbnail image.
}picasaweb_media_thumbnail_attr_t;

typedef struct picasaweb_media_group_s
{
	picasaweb_media_content_attr_t content;	///< Contains the URL and other information about the full version of the entry's media content.
	picasaweb_data_atomic_t credit;		///< Contains the nickname of the user who created the content. 
	picasaweb_data_atomic_t description;	///< Contains a description of the entry's media content. 
	picasaweb_data_atomic_t keywords;	///< Lists the tags associated with the entry.
	picasaweb_media_thumbnail_attr_t thumbnail;	///< Contains the URL of a thumbnail of a photo or album cover.
	picasaweb_data_atomic_t title;		///< Contains the title of the entry's media content, in plain text.
}picasaweb_media_group_t;

///< exif:tags element
typedef struct picasaweb_exif_tags_s
{
	picasaweb_data_atomic_t distance;	///< The distance to the subject.
	picasaweb_data_atomic_t exposure;	///< The exposure time used.
	picasaweb_data_atomic_t flash;		///< Boolean value indicating whether the flash was used.
	picasaweb_data_atomic_t focallength;	///< The focal length used.
	picasaweb_data_atomic_t fstop;		///< The fstop value used.
	picasaweb_data_atomic_t img_uniqueID;	///< The unique image ID for the photo.
	picasaweb_data_atomic_t iso;	///< The iso equivalent value used.
	picasaweb_data_atomic_t make;	///< The make of the camera used.
	picasaweb_data_atomic_t model;	///< The model of the camera used.
	picasaweb_data_atomic_t time;	///< The date/time the photo was taken, represented as the number of milliseconds since January 1st,1970.
}picasaweb_exif_tags_t;	///< The container for all exif elements.



typedef struct picasaweb_link_s
{
	picasaweb_data_atomic_t rel;
	picasaweb_data_atomic_t type;
	picasaweb_data_atomic_t href;
}picasaweb_link_attr_t;

typedef struct picasaweb_Point_s
{
	picasaweb_data_atomic_t pos;///< Specifies a latitude and longitude, separated by a space.
}picasaweb_Point_t;///< Specifies a particular geographical point

typedef struct picasaweb_where_s
{
	picasaweb_Point_t point;
}picasaweb_where_t;///< Specifies a geographical location or region.

typedef struct picasaweb_element_userkind_s
{
	picasaweb_data_atomic_t maxPhotoPerAlbum;
	
}picasaweb_element_userkind_t;

typedef struct picasaweb_entry_s
{
	picasaweb_feed_t *child_feed;	///< if the entry is an album it is the photos feed, else it is NULL
	PicasawebKindValue_e kind_value;
	picasaweb_data_atomic_t attr_etag;
	picasaweb_exif_tags_t tags;	
	///< see http://code.google.com/intl/zh-CN/apis/picasaweb/docs/2.0/reference.html#gphoto_albumid
	picasaweb_data_atomic_t albumid; ///< The album's ID.
	picasaweb_data_atomic_t id;///< The ID of the current element.
	picasaweb_data_atomic_t published;
	picasaweb_data_atomic_t updated;
	picasaweb_data_atomic_t edited;
	picasaweb_data_atomic_t rights;
	picasaweb_data_atomic_t summary; 
	picasaweb_data_atomic_t title;

	picasaweb_author_t author;

	picasaweb_media_group_t group;

	picasaweb_where_t where;

	picasaweb_link_attr_t link_edit;
	picasaweb_link_attr_t link_feed;
	///<  element is valid with the user kind.
	picasaweb_data_atomic_t maxPhotoPerAlbum;///< The maximum number of photos allowed in an album.
	picasaweb_data_atomic_t nickname;///< The user's nickname. 
	picasaweb_data_atomic_t quotacurrent;///< The number of bytes of storage currently in use by the user.
	picasaweb_data_atomic_t quotalimit;///< The total amount of space allotted to the user.
	picasaweb_data_atomic_t thumbnail;///< The URL of a thumbnail-sized portrait of the user. 
	picasaweb_data_atomic_t user;///< The user's username. This is the name that is used in all feed URLs.

	///< element is valid with album kind
	picasaweb_data_atomic_t access;///< The album's access level. 
	picasaweb_data_atomic_t bytesUsed;///< The number of bytes of storage that this album uses.
	picasaweb_data_atomic_t location;///< The user-specified location associated with the album.
	picasaweb_data_atomic_t numphotos;///< The number of photos in the album.
	picasaweb_data_atomic_t numphotosremaining;///< The number of remaining photo uploads allowed in this album. 
	
	///< element is valid with photo kind
	picasaweb_data_atomic_t checksum;///< The checksum on the photo. 
	picasaweb_data_atomic_t commentCount;	///< The number of comments on an element.
	picasaweb_data_atomic_t height;	///< The height of the photo in pixels.
	picasaweb_data_atomic_t width;	///< The width of the photo in pixels.
	picasaweb_data_atomic_t rotation;///< The rotation of the photo in degrees, used to change the rotation of the photo.
	picasaweb_data_atomic_t size;	///< The size of the photo in bytes.
	picasaweb_data_atomic_t timestamp;	///< The photo's timestamp, represented as the number of milliseconds since January 1st, 1970.
	picasaweb_data_atomic_t videostatus;	///< The current processing status of a video.

	///< The following element appears only in feeds of the comment kind.
	picasaweb_data_atomic_t photoid;///< The ID of the photo associated with the current comment.

	///< The following element appears only in feeds of the tag kind.
	picasaweb_data_atomic_t weight;	///< the weight of the tag. 
					///< The weight is the number of times the tag appears in photos under the current element. 

	///< The following elements appear only in photo entries in search result feeds.
	picasaweb_data_atomic_t albumtitle;	///< Title of the album this photo is in.
	picasaweb_data_atomic_t albumdesc;	///< Description of the album this photo is in.
	picasaweb_data_atomic_t snippet;	///< Snippet that matches the search text.
	picasaweb_data_atomic_t snippettype; 	///< Describes where the match with the search query was found
	picasaweb_data_atomic_t truncated;	///< Indicates whether search results are truncated or not.
}picasaweb_entry_t;


struct picasaweb_feed_s
{
	PicasawebFeedKind_e feed_kind;
	PicasawebKindValue_e kind_value;
	///< see http://code.google.com/intl/zh-CN/apis/picasaweb/docs/2.0/reference.html#gphoto_albumid
	picasaweb_data_atomic_t attr_etag;

	picasaweb_data_atomic_t albumid; ///< The album's ID.
	picasaweb_data_atomic_t id;///< The ID of the current element.

	picasaweb_data_atomic_t updated;
	picasaweb_data_atomic_t title;
	picasaweb_data_atomic_t subtitle;
	picasaweb_data_atomic_t icon;

	picasaweb_author_t author;

	picasaweb_data_atomic_t totalResults;
	picasaweb_data_atomic_t startIndex;
	picasaweb_data_atomic_t itemsPerPage;
	
	///<  element is valid with the user kind.
	picasaweb_data_atomic_t maxPhotoPerAlbum;///< The maximum number of photos allowed in an album.
	picasaweb_data_atomic_t nickname;///< The user's nickname. 
	picasaweb_data_atomic_t quotacurrent;///< The number of bytes of storage currently in use by the user.
	picasaweb_data_atomic_t quotalimit;///< The total amount of space allotted to the user.
	picasaweb_data_atomic_t thumbnail;///< The URL of a thumbnail-sized portrait of the user. 
	picasaweb_data_atomic_t user;///< The user's username. This is the name that is used in all feed URLs.

	///< element is valid with album kind
	picasaweb_data_atomic_t access;///< The album's access level. 
	picasaweb_data_atomic_t bytesUsed;///< The number of bytes of storage that this album uses.
	picasaweb_data_atomic_t location;///< The user-specified location associated with the album.
	picasaweb_data_atomic_t numphotos;///< The number of photos in the album.
	picasaweb_data_atomic_t numphotosremaining;///< The number of remaining photo uploads allowed in this album. 

	///< element is valid with photo kind
	picasaweb_data_atomic_t checksum;///< The checksum on the photo. 
	picasaweb_data_atomic_t commentCount;	///< The number of comments on an element.
	picasaweb_data_atomic_t height;	///< The height of the photo in pixels.
	picasaweb_data_atomic_t width;	///< The width of the photo in pixels.
	picasaweb_data_atomic_t rotation;///< The rotation of the photo in degrees, used to change the rotation of the photo.
	picasaweb_data_atomic_t size;	///< The size of the photo in bytes.
	picasaweb_data_atomic_t timestamp;	///< The photo's timestamp, represented as the number of milliseconds since January 1st, 1970.
	picasaweb_data_atomic_t videostatus;	///< The current processing status of a video.
	
	///< The following element appears only in feeds of the comment kind.
	picasaweb_data_atomic_t photoid;///< The ID of the photo associated with the current comment.

	///< The following element appears only in feeds of the tag kind.
	picasaweb_data_atomic_t weight;	///< the weight of the tag. 
					///< The weight is the number of times the tag appears in photos under the current element. 
	int entry_num;	///< how many the album included or how many the photos included
	unsigned char * download_thumbnailbitmap; ///< check whether the thumbnail is download ,each entry had a bit
	unsigned char * download_browsebitmap; ///< check whether the browse is download, each entry had a bit 
	picasaweb_entry_t *entry;
};


typedef struct picasa_path_s
{
	picasaweb_data_atomic_t cache_dir;	///< where the photo will be store
	picasaweb_data_atomic_t album_id;	///< the album name
	picasaweb_data_atomic_t photo_name;	///< the photo name
}picasa_path_t;

typedef struct photo_down_info_s ///< the info for downloading photo
{
	unsigned char needcache:1;		///< whether it is cached, 0: the data of the photo will be stored at the member of photo_data
	unsigned char workdone:1;		///< whether the work is finished 0:unfinish 1: finished
	unsigned char isthumbnail:1;		///< whether it is a thumbnail	
	int prog_down;				///< the progress of download	
	int status;				///< see macro get_status()
	picasaweb_feed_t *feed; 	///< which feed does the photo included
	int which_entry;			///< which entry does the photo in
	picasaweb_data_atomic_t	photo_url; ///< url of the photo to be download
	///< for uncache
	picasaweb_data_write_t photo_data; 	///< if the cached==0, the photo data will be stored at here
	///< for cache	
	picasa_path_t img_path;
	void * (*func_download_callback)(void* para); ///< the callback function for download
	void * func_para;					///< the para of the callback function
}photo_down_info_t;


typedef struct photo_upload_info_s
{
	int photo_type;				///< the photo type, see picasaweb_photo_type_e
	char * photo_name;			///< the photo name
	unsigned long filesize_bytes;	///< the file size
	unsigned long uploaded_bytes; ///< the bytes that had been sent, using the filesize_bytes and the uploaded_bytes, can get the progress
	int status;	///< the upload status
	picasaweb_data_write_t photo_data_send;///< the handle of the file will the stored at there
	picasaweb_data_write_t photo_data_get;	///< the data retrive from the server will be stored at here
	picasaweb_feed_t * feed; // the feed that the albums include
	int which_entry;			///< which entry does the album in
}photo_upload_info_t;///< the information of the file for uploading 

typedef struct album_info_s ///< the infomation for creating or updating album 
{
	picasaweb_data_atomic_t title; 		///< new title
	picasaweb_data_atomic_t summary;		///< new summary
	picasaweb_data_atomic_t location;		///< new location
	picasaweb_data_atomic_t access;		///< new access: private,protected,public
	picasaweb_data_atomic_t timestamp;	///< newe stamp since 1970.1.1 the unit is milisec
	picasaweb_data_atomic_t gphoto_id; 	///< this value is needed when updating album it retrived from the entry
}album_info_t;


typedef struct photo_info_s ///< the infomation for updating the photo
{
	picasaweb_data_atomic_t summary;		///< new summary
	picasaweb_data_atomic_t description;		///< new description
}photo_info_t;



typedef enum{
	PICASA_CMD_DOWNLOADPHOTO,
	PICASA_CMD_STOPDOWNLOAD,
	PICASA_CMD_UPLOADPHOTO,
	PICASA_CMD_STOPUPLOAD,
}picasa_ioctl_cmd_e;

typedef enum{
	PICASA_REQ_INVALID,
	PICASA_REQ_ENQUEUE,
	PICASA_REQ_DEQUEUE,
	PICASA_REQ_DONE,
}picasa_req_status_e;


///the following are defined by using in checking path and file when downloading info
typedef enum{
	PICASA_PATH_OK,
	PICASA_PATH_CACHEDIR_ERROR,
	PICASA_PATH_ALBUMID_ERROR,
	PICASA_FILE_EXIST,
	PICASA_FILE_CREATE_FAIL,
	PICASA_FILE_BUFFER_ERR,
}picasa_file_check_err_e;

typedef struct picasa_ioctrl_s
{
	picasa_req_status_e active;
	unsigned long timestamp;
	picasa_ioctl_cmd_e iocmd;
	void *para;
	int para_size;
	
}picasa_ioctrl_t;

typedef struct picasa_update_s
{
	picasaweb_gdata_t * gdata;
	int iscache;
	char * cache_dir;
	int isthumbnail;
}picasa_update_t;
/**
@brief copy the content to the atomic element, call the picasa_free_atomic() to free the space 
@param[in] content	: the content 
@param[in] atomic_name	: which atomic element will be filled
@return 
	- -1	: failed
	- 0	: success
@see picasa_free_atomic()
**/
int  picasa_fill_atomic(xmlChar * content,picasaweb_data_atomic_t *atomic_name);

/**
@brief release the space which is occupied by calling picasa_fill_atomic()
@param[in] atimic_name	: where the content stored
@return 
	- -1	: failed
	- 0	: success
@see picasa_fill_atomic()
**/
int picasa_free_atomic(picasaweb_data_atomic_t *atomic_name);


/**
@biref call this function when the app is started for initializing system resource, such as hash table, call this function at the beginning 
@param[in] NULL
@return 
	- 0	: succeed
	- 1	: failed
**/
int picasa_init_resource();

/**
@brief call this function to set the email addr and password
@param[in] email	: email addr
@param[in] pwd	: password
@return
	- 0 	: success
	- -1	: failed
**/
int picasa_set_longin(const char* email, const char* pwd);


/**
@brief initializing the structure of picasaweb_gdata_t ,call the picasa_free_gdata() to free the space
remember to call this function first, for the reason that the other function will be used it
@param[in] gdata	: the pointer to the gdata which will be filled
@return 
	- -1	: failed
	- 0	: success
@see picasa_free_gdata()
**/
int picasa_init_gdata(picasaweb_gdata_t *gdata);

/**
@brief free the space which had been malloc when the picasa_init_gdata() is called
@param[in] gdata	: the pointer to the gdata which had been filled
@see picasa_init_gdata()
**/
int picasa_free_gdata(picasaweb_gdata_t *gdata);


/**
@brief : get the contact list, call this function after the authentication, and call picasa_free_contact() to free the space
@param[in] gdata : initialize by calling the picasa_init_gdata()
@return
	- NULL		: get the infomation of the contacts failed
	- others	: the pointer to the picasaweb_feed_t which the information of the contacts are inclued 
**/
picasaweb_feed_t* picasa_get_contact(picasaweb_gdata_t *gdata);


/**
@brief :  free the space which is occupied when the picasa_get_contact() is called 
@param[in] contact_feed :  the pointer to the picasaweb_feed_t which the information of the contacts are included when the picasa_get_contact() is called
@return 
	always return 0
**/
int picasa_free_contact(picasaweb_feed_t *contact_feed);


/**
@brief call this function after the authentication is ok for getting the albums in the account
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_contact 	: get from picasa_get_cotact(), if this value is null, get albums info of the login account
@param[in] which_friend	:  if the feed_contact is NULL, this value is useless,if the feed_contact is not NULL, specify which friend will be get, the range of this value is 0~ (feed_contact->entry_num-1)
@return 
	- NULL		: get the infomation of the albums failed
	- others	: the pointer to the picasaweb_feed_t which the information of the albums is inclued 
**/
picasaweb_feed_t * picasa_get_albums_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_contact,int which_friend);

/**
@brief free the space which is occupied when the picasa_get_albums_info() is called 
@param[in] albums_info 	: the pointer to the picasaweb_feed_t which the information of the album is included when the picasa_get_albums_info() is called
@return always 0
@see picasa_get_albums_info()
**/
int picasa_free_albums_info(picasaweb_feed_t * feed_albums);

/**
@brief getting the information of an album which is specified by the para used
@param[in] gdata	: initialize by calling the picasa_init_gdata();
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@return
	NULL	: get the infomation of the album failed
	others	: the pointer to the picasaweb_feed_t which the information of the album 
@see call picasa_free_album_info() to free the space which is malloced
**/
picasaweb_feed_t * picasa_get_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album);

/**
@brief free the space which is occupied when the picasa_get_album_info() is called 
@param[in] album_info 	: the pointer to the picasaweb_feed_t which the information of the photos are included when the picasa_get_album_info() is called
@param[in] feed_albums : which feed_albums does this album belong to, get it from calling picasa_get_albums_info();
@param[in] which_album : the album index in the feed_albums,the range of this value is 0~(feed_albums->entry_num-1)
@return always 0
@see picasa_get_album_info()
**/
int picasa_free_album_info(picasaweb_feed_t *feed_albums,int which_album);

/**
@brief call this function for passing the authentication
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
int picasa_authentication(picasaweb_gdata_t *gdata);


/**
@brief call this function for creating new albums
@param[in] album_info	: the initial info of the album which will be created
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED: success
**/
int picasa_create_new_album(album_info_t *album_info,picasaweb_gdata_t *gdata);

/**
@brief call this function for deleting albums
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@param[in] feed_albums	: the album information of the albums, see picasa_get_albums_info()
@param[in] which_album	: which album to be choosen
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
int picasa_delete_album(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album);

/**
*@brief call this function to update some info of the album
*@param[in] album_info	: change the info of the album if the element of album_info is not NULL, the gphoto_id member should be the value that retrived from the server.
*@param[in] gdata	: initialize by calling the picasa_init_gdata()
*@param[in] feed_albums	: the pointer to the picasaweb_feed_t where the info of the albums stored, see picasa_get_albums_info()
*@param[in] which_album	: which album to be changed
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
int picasa_update_album_info(album_info_t * album_info,picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album);


/**
@brief call this function to initializing the photo_upload_info_t which will be used in picasa_upload_photo(), call picasa_free_upload_info()
to release the resource
@param[in]	photo_fullpath	: the full path of the photo which will be uploaded
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info()
@param[in] which_entry 	: which album does the photo to be added
@return 
	- NULL	: failed
	- others	: the pointer to the photo_upload_info_t
@see picasa_free_upload_info()
**/
photo_upload_info_t *  picasa_init_upload_info(char* photo_fullpath,picasaweb_feed_t *feed,int which_entry);

/**
@brief call this function to to release the resource which had been occupied by calling picasa_init_upload_info()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling picasa_init_upload_info()
@return 
	always 0
**/
int picasa_free_upload_info(photo_upload_info_t * upload_info);


/**
@brief call this function to upload a photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling picasa_init_upload_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED	: success
**/
int picasa_upload_photo(picasaweb_gdata_t *gdata,photo_upload_info_t* upload_info);

/**
@brief query the upload staus,  call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] para	: it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not up yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
int picasa_query_upload_status(picasaweb_gdata_t *gdata,photo_upload_info_t * upload_info,PicasawebQueryCmd query_cmd);


/**
@brief call this function to update the information of a photo
@param[in] photo_info 	: the info of the photo which will be replaced
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_album	: the photo information in an album, see picasa_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
int picasa_update_photo_info(photo_info_t *photo_info,picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_album,int which_photo);

/**
@brief call this function to delete a photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_album	: the photo information in an album, see picasa_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
int picasa_delete_photo(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_album,int which_photo);


/**
@brief call this function to initialize the download info ,call __picasa_free_download_info() to free the space 
@param[in] iscache	: whether it is cached
@param[in] cache_dir	: if iscache==1, it is the path of cache dir, such as /mnt/udisk/
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info() or picasa_get_album_info()
@param[in] which_entry 	: which entry that the photo url in
@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
@param[out] err_status : when the return value is NULL, the reason of err is stored in here,see struct of picasa_file_check_err_e
@return 
	- NULL 	: failed
	- others : the pointer to the photo_down_info_t
@see __picasa_free_download_info()
**/
photo_down_info_t *picasa_init_download_info(int iscache,char* cache_dir,picasaweb_feed_t *feed,int which_entry,int isthumbnail,int *err_status);

/**
@brief release the space which is malloc when the __picasa_init_download_info() is called
@param[in] down_info 	: get from __picasa_init_download_info()
@return
	always return 0
**/
int picasa_free_download_info(photo_down_info_t* down_info);

/**
@brief download photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@parma[in] down_info	: choose the method to be download, cache or uncache ,see __picasa_init_download_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
int picasa_download_photo(picasaweb_gdata_t *gdata,photo_down_info_t *down_info);


/**
@brief query the download staus, call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] down_info	: it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
int picasa_query_download_status(picasaweb_gdata_t *gdata,photo_down_info_t * down_info,PicasawebQueryCmd query_cmd);


/**
@brief send the message to the thread
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cmd 	: see picasa_ioctl_cmd_e
@param[in] para	: 
			if cmd ==PICASA_CMD_DOWNLOADPHOTO
				it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
			if cmd==PICASA_CMD_UPLOADPHOTO
				it is the pointer to the photo_upload_info_t, call the picasa_init_upload_info() to get it
@return 
	 - -1	: failed
	 - 0 		: success
**/
int picasa_send_msg(picasaweb_gdata_t *gdata,picasa_ioctl_cmd_e cmd,void * para);


/**
@brief query the download staus, call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] down_info	: it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
int picasa_get_cache_path(picasaweb_feed_t *feed,int which_entry,int isthumbnail,char *pathbuf,int buf_len);

/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: get from picasa_get_albums_info()
@param[in] cache_dir	: under which folder will the file be created
@param[in] filename	: the name of the file to be created,if it is null, the default filename is the login name
@return :
	-1 	: failed
	0	: succeed
**/
int picasa_save_albums_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_albums,char* cache_dir,char* filename);


/**
@brief load albums info from the file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@param[in] filename	: the name of the file to be read
@return :
	NULL	: load albums info error
	others	: he pointer to the picasaweb_feed_t which include all information of the albums in the account
**/
picasaweb_feed_t * picasa_load_albums_info(picasaweb_gdata_t *gdata,char* cache_dir,char *filename);


/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
int picasa_save_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album,char* cache_dir);


/**
@brief load the album info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	NULL	: load album info error
	others	: the pointer to the picasaweb_feed_t which include all information of the album
**/
picasaweb_feed_t * picasa_load_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album,char* cache_dir);


/**
@brief save the contact info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_contact	: get from picasa_get_contact()
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
int picasa_save_contact_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_contact,char* cache_dir);


/**
@brief load contact info from the file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@return :
	NULL	: load albums info error
	others	: he pointer to the picasaweb_feed_t which include all information of the contact in the account
**/
picasaweb_feed_t * picasa_load_contact_info(picasaweb_gdata_t *gdata,char* cache_dir);



///<the following for facebook

#if 1
	#define facebook_dbg(fmt,arg...) printf("Facebook[%s,%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define facebook_dbg(fmt,arg...)
#endif

#define LOGIN_MAX_LEN		64
#define POST_FORM_ID_LEN	32
#define FB_DTSG_LEN		5
#define M_SESS_LEN		17

enum{
	FACEBOOK_RTN_OK,	//nothing error happend
	FACEBOOK_RTN_ERR_LOGIN,
	FACEBOOK_RTN_ERR_CREATALBUM,
	FACEBOOK_RTN_ERR_DELETEALBUM,
	FACEBOOK_RTN_ERR_UPDATEALBUM,
	FACEBOOK_RTN_ERR_UPLOAD_OPENFILE,
	FACEBOOK_RTN_ERR_UPLOADPHOTO,
	FACEBOOK_RTN_ERR_UPDATEPHOTO,
	FACEBOOK_RTN_ERR_DELETEPHOTO,
	FACEBOOK_RTN_ERR_DOWNLOADPHOTO,
	FACEBOOK_RTN_ERR_MEMORY, // memory error
	FACEBOOK_RTN_ERR_PARA,
	FACEBOOK_RTN_CANCEL, // the process had been canceled
	FACEBOOK_RTN_ERR_LOGINPASSWORD,
	FACEBOOK_RTN_ERR_ACCOUNTEXCEPTION,
	FACEBOOK_RTN_ERR_ACCOUNTCONFIRM,
}FacebookRtn_e;

typedef struct facebook_semi_t{
	sem_t semi_req;
	sem_t semi_start;
	sem_t semi_end;
}facebook_semi;

typedef struct facebook_data_t{
	char usermail[LOGIN_MAX_LEN];
	char userpwd[LOGIN_MAX_LEN];
	CURL * curl;
	char * access_token;
	int auth_status;
	char * data_cur;
	facebook_semi syn_lock; ///< for the thread sync 
	pthread_t thread_id;
	unsigned is_thread_run:1;///<  note whether the thread is run
	int req_start_idx; ///< the req start idx

	pthread_t update_thread_id;
}facebook_data;

typedef enum{
	Facebook_data_write_buffer=0,	///< write the data to the buffer
	Facebook_data_write_file=1,	///< write the data to the file which had been opened
}FacebookDataWrite_e;

typedef struct facebook_write_data_t{
	FacebookDataWrite_e data_type;	///< where the data to be written, buffer or file
	unsigned char cancel_write:1;	///< 1 : cancel the write, 0 continue
	void * file_handle;	///< the pointer to the data file
	char *data_head;	///< the pointer to the data buffer
	unsigned int data_len;	///< the total bytes
	char *data_cur;		///< the pointer to the end of the data buffer which is used
	unsigned int data_used;	///< the bytes which had been used
	int download_process;	///< the download process
	int status;				///< see macro get_status()
}facebook_write_data;

///<General Errors
#define API_EC_SUCCESS 0	///<Success (all) 
#define API_EC_UNKNOWN 1	///<An unknown error occurred (all) 
#define API_EC_SERVICE 2	///<Service temporarily unavailable (all) 
#define API_EC_METHOD 3	///<Unknown method 
#define API_EC_TOO_MANY_CALLS 4	///<API_EC_TOO_MANY_CALLS Application request limit reached (all) 
#define API_EC_BAD_IP 5	///<Unauthorized source IP address (all) 
#define API_EC_HOST_API 6	///<This method must run on api.facebook.com (all) 
#define API_EC_HOST_UP 7	///<This method must run on api-video.facebook.com 
#define API_EC_SECURE 8	///<API_EC_SECURE This method requires an HTTPS connection 
#define API_EC_RATE 9	///<User is performing too many actions 
#define API_EC_PERMISSION_DENIED 10	///<Application does not have permission for this action 
#define API_EC_DEPRECATED 11	///<This method is deprecated 
#define API_EC_VERSION 12	///<API_EC_VERSION This API version is deprecated 
#define API_EC_INTERNAL_FQL_ERROR 13	///<API_EC_INTERNAL_FQL_ERROR The underlying FQL query made by this API call has encountered an error. Please check that your parameters are correct. 
#define API_EC_HOST_PUP 14	///<This method must run on api-photo.facebook.com 
#define API_EC_SESSION_SECRET_NOT_ALLOWED 15	///<This method call must be signed with the application secret (You are probably calling a secure method using a session secret) 
#define API_EC_HOST_READONLY 16	///<This method cannot be run on this host, which only supports read-only calls 
#define API_EC_USER_TOO_MANY_CALLS 17	///<User request limit reached 
#define API_EC_REQUEST_RESOURCES_EXCEEDED 18	///<This API call could not be completed due to resource limits 

///<Parameter Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_PARAM 100	///<Invalid parameter (all) 
#define API_EC_PARAM_API_KEY 101	///<Invalid API key (all) 
#define API_EC_PARAM_SESSION_KEY 102	///<Session key invalid or no longer valid (all) 
#define API_EC_PARAM_CALL_ID 103	///<Call_id must be greater than previous 
#define API_EC_PARAM_SIGNATURE 104	///<Incorrect signature (all) 
#define API_EC_PARAM_TOO_MANY 105	///<The number of parameters exceeded the maximum for this operation 
#define API_EC_PARAM_USER_ID 110	///<Invalid user id photos.addTag 
#define API_EC_PARAM_USER_FIELD 111	///<Invalid user info field 
#define API_EC_PARAM_SOCIAL_FIELD 112	///<Invalid user field 
#define API_EC_PARAM_EMAIL 113	///<Invalid email 
#define API_EC_PARAM_USER_ID_LIST 114	///<Invalid user ID list 
#define API_EC_PARAM_FIELD_LIST 115	///<Invalid field list 
#define API_EC_PARAM_ALBUM_ID 120	///<Invalid album id 
#define API_EC_PARAM_PHOTO_ID 121	///<Invalid photo id 
#define API_EC_PARAM_FEED_PRIORITY 130	///<Invalid feed publication priority 
#define API_EC_PARAM_CATEGORY 140	///<Invalid category 
#define API_EC_PARAM_SUBCATEGORY 141	///<Invalid subcategory 
#define API_EC_PARAM_TITLE 142	///<Invalid title 
#define API_EC_PARAM_DESCRIPTION 143	///<Invalid description 
#define API_EC_PARAM_BAD_JSON 144	///<Malformed JSON string 
#define API_EC_PARAM_BAD_EID 150	///<Invalid eid 
#define API_EC_PARAM_UNKNOWN_CITY 151	///<Unknown city 
#define API_EC_PARAM_BAD_PAGE_TYPE 152	///<Invalid page type 
#define API_EC_PARAM_BAD_LOCALE 170	///<Invalid locale 
#define API_EC_PARAM_BLOCKED_NOTIFICATION 180	///<This notification was not delieved 
#define API_EC_PARAM_ACCESS_TOKEN 190	///<Invalid OAuth 2.0 Access Token 

///<User Permission Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_PERMISSION 200	///<Permissions error 
#define API_EC_PERMISSION_USER 210	///<User not visible 
#define API_EC_PERMISSION_NO_DEVELOPERS 211	///<Application has no developers. admin.setAppProperties 
#define API_EC_PERMISSION_OFFLINE_ACCESS 212	///<Renewing a session offline requires the extended permission offline_access 
#define API_EC_PERMISSION_ALBUM 220	///<Album or albums not visible 
#define API_EC_PERMISSION_PHOTO 221	///<Photo not visible 
#define API_EC_PERMISSION_MESSAGE 230	///<Permissions disallow message to user 
#define API_EC_PERMISSION_MARKUP_OTHER_USER 240	///<Desktop applications cannot set FBML for other users 
#define API_EC_PERMISSION_STATUS_UPDATE 250	///<Updating status requires the extended permission status_update. users.setStatus 
#define API_EC_PERMISSION_PHOTO_UPLOAD 260	///<Modifying existing photos requires the extended permission photo_upload photos.upload,photos.addTag 
#define API_EC_PERMISSION_VIDEO_UPLOAD 261	///<Modifying existing photos requires the extended permission photo_upload photos.upload,photos.addTag 
#define API_EC_PERMISSION_SMS 270	///<Permissions disallow sms to user. 
#define API_EC_PERMISSION_CREATE_LISTING 280	///<Creating and modifying listings requires the extended permission create_listing 
#define API_EC_PERMISSION_CREATE_NOTE 281	///<Managing notes requires the extended permission create_note. 
#define API_EC_PERMISSION_SHARE_ITEM 282	///<Managing shared items requires the extended permission share_item. 
#define API_EC_PERMISSION_EVENT 290	///<Creating and modifying events requires the extended permission create_event events.create, events.edit 
#define API_EC_PERMISSION_LARGE_FBML_TEMPLATE 291	///<FBML Template isn\'t owned by your application. 
#define API_EC_PERMISSION_LIVEMESSAGE 292	///<An application is only allowed to send LiveMessages to users who have accepted the TOS for that application. liveMessage.send 
#define API_EC_PERMISSION_XMPP_LOGIN 293	///<Logging in to chat requires the extended permission xmpp_login Integrating with FacebookChat 
#define API_EC_PERMISSION_ADS_MANAGEMENT 294	///<Managing advertisements requires the extended permission ads_management, and a participating API key Ads API 
#define API_EC_PERMISSION_CREATE_EVENT 296	///<Managing events requires the extended permission create_event API#Events_API_Methods 
#define API_EC_PERMISSION_READ_MAILBOX 298	///<Reading mailbox messages requires the extended permission read_mailbox message.getThreadsInFolder 
#define API_EC_PERMISSION_RSVP_EVENT 299	///<RSVPing to events requires the extended permission create_rsvp events.rsvp 

///<Data Editing Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_EDIT 300	///<Edit failure 
#define API_EC_EDIT_USER_DATA 310	///<User data edit failure 
#define API_EC_EDIT_PHOTO 320	///<Photo edit failure 
#define API_EC_EDIT_ALBUM_SIZE 321	///<Album is full 
#define API_EC_EDIT_PHOTO_TAG_SUBJECT 322	///<Invalid photo tag subject 
#define API_EC_EDIT_PHOTO_TAG_PHOTO 323	///<Cannot tag photo already visible on Facebook 
#define API_EC_EDIT_PHOTO_FILE 324	///<Missing or invalid image file 
#define API_EC_EDIT_PHOTO_PENDING_LIMIT 325	///<Too many unapproved photos pending 
#define API_EC_EDIT_PHOTO_TAG_LIMIT 326	///<Too many photo tags pending 
#define API_EC_EDIT_ALBUM_REORDER_PHOTO_NOT_IN_ALBUM 327	///<Input array contains a photo not in the album 
#define API_EC_EDIT_ALBUM_REORDER_TOO_FEW_PHOTOS 328	///<Input array has too few photos 
#define API_EC_MALFORMED_MARKUP 329	///<Template data must be a JSON-encoded dictionary, of the form {\'key-1\': \'value-1\', \'key-2\': \'value-2\', ...} 
#define API_EC_EDIT_MARKUP 330	///<Failed to set markup 
#define API_EC_EDIT_FEED_TOO_MANY_USER_CALLS 340	///<Feed publication request limit reached 
#define API_EC_EDIT_FEED_TOO_MANY_USER_ACTION_CALLS 341	///<Feed action request limit reached 
#define API_EC_EDIT_FEED_TITLE_LINK 342	///<Feed story title can have at most one href anchor 
#define API_EC_EDIT_FEED_TITLE_LENGTH 343	///<Feed story title is too long 
#define API_EC_EDIT_FEED_TITLE_NAME 344	///<Feed story title can have at most one fb:userlink and must be of the user whose action is being reported 
#define API_EC_EDIT_FEED_TITLE_BLANK 345	///<Feed story title rendered as blank 
#define API_EC_EDIT_FEED_BODY_LENGTH 346	///<Feed story body is too long 
#define API_EC_EDIT_FEED_PHOTO_SRC 347	///<Feed story photo could not be accessed or proxied 
#define API_EC_EDIT_FEED_PHOTO_LINK 348	///<Feed story photo link invalid 
#define API_EC_EDIT_VIDEO_SIZE 350	///<Video file is too large video.upload 
#define API_EC_EDIT_VIDEO_INVALID_FILE 351	///<Video file was corrupt or invalid video.upload 
#define API_EC_EDIT_VIDEO_INVALID_TYPE 352	///<Video file format is not supported video.upload 
#define API_EC_EDIT_VIDEO_FILE 353	///<Missing video file video.upload 
#define API_EC_EDIT_VIDEO_NOT_TAGGED 354	///<User is not tagged in this video 
#define API_EC_EDIT_VIDEO_ALREADY_TAGGED 355	///<User is already tagged in this video 
#define API_EC_EDIT_FEED_TITLE_ARRAY 360	///<Feed story title_data argument was not a valid JSON-encoded array 
#define API_EC_EDIT_FEED_TITLE_PARAMS 361	///<Feed story title template either missing required parameters, or did not have all parameters defined in title_data array 
#define API_EC_EDIT_FEED_BODY_ARRAY 362	///<Feed story body_data argument was not a valid JSON-encoded array 
#define API_EC_EDIT_FEED_BODY_PARAMS 363	///<Feed story body template either missing required parameters, or did not have all parameters defined in body_data array 
#define API_EC_EDIT_FEED_PHOTO 364	///<Feed story photos could not be retrieved, or bad image links were provided 
#define API_EC_EDIT_FEED_TEMPLATE 365	///<The template for this story does not match any templates registered for this application 
#define API_EC_EDIT_FEED_TARGET 366	///<One or more of the target ids for this story are invalid. They must all be ids of friends of the acting user 
#define API_EC_EDIT_FEED_MARKUP 367	///<The template data provided doesn't cover the entire token set needed to publish the story 
#define API_EC_USERS_CREATE_INVALID_EMAIL 370	///<The email address you provided is not a valid email address 
#define API_EC_USERS_CREATE_EXISTING_EMAIL 371	///<The email address you provided belongs to an existing account 
#define API_EC_USERS_CREATE_BIRTHDAY 372	///<The birthday provided is not valid 
#define API_EC_USERS_CREATE_PASSWORD 373	///<The password provided is too short or weak 
#define API_EC_USERS_REGISTER_INVALID_CREDENTIAL 374	///<The login credential you provided is invalid. 
#define API_EC_USERS_REGISTER_CONF_FAILURE 375	///<Failed to send confirmation message to the specified login credential. 
#define API_EC_USERS_REGISTER_EXISTING 376	///<The login credential you provided belongs to an existing account 
#define API_EC_USERS_REGISTER_DEFAULT_ERROR 377	///<Sorry, we were unable to process your registration. 
#define API_EC_USERS_REGISTER_PASSWORD_BLANK 378	///<Your password cannot be blank. Please try another. 
#define API_EC_USERS_REGISTER_PASSWORD_INVALID_CHARS 379	///<Your password contains invalid characters. Please try another. 
#define API_EC_USERS_REGISTER_PASSWORD_SHORT 380	///<Your password must be at least 6 characters long. Please try another. 
#define API_EC_USERS_REGISTER_PASSWORD_WEAK 381	///<Your password should be more secure. Please try another. 
#define API_EC_USERS_REGISTER_USERNAME_ERROR 382	///<Our automated system will not approve this name. 
#define API_EC_USERS_REGISTER_MISSING_INPUT 383	///<You must fill in all of the fields. 
#define API_EC_USERS_REGISTER_INCOMPLETE_BDAY 384	///<You must indicate your full birthday to register. 
#define API_EC_USERS_REGISTER_INVALID_EMAIL 385	///<Please enter a valid email address. 
#define API_EC_USERS_REGISTER_EMAIL_DISABLED 386	///<The email address you entered has been disabled. Please contact disabled@facebook.com with any questions. 
#define API_EC_USERS_REGISTER_ADD_USER_FAILED 387	///<There was an error with your registration. Please try registering again. 
#define API_EC_USERS_REGISTER_NO_GENDER 388	///<Please select either Male or Female. 

///<Authentication Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_AUTH_EMAIL 400	///<Invalid email address 
#define API_EC_AUTH_LOGIN 401	///<Invalid username or password 
#define API_EC_AUTH_SIG 402	///<Invalid application auth sig 
#define API_EC_AUTH_TIME 403	///<Invalid timestamp for authentication 

///<Session Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_SESSION_TIMED_OUT 450	///<Session key specified has passed its expiration time 
#define API_EC_SESSION_METHOD 451	///<Session key specified cannot be used to call this method 
#define API_EC_SESSION_INVALID 452	///<Session key invalid. This could be because the session key has an incorrect format, or because the user has revoked this session 
#define API_EC_SESSION_REQUIRED 453	///<A session key is required for calling this method 
#define API_EC_SESSION_REQUIRED_FOR_SECRET 454	///<A session key must be specified when request is signed with a session secret 
#define API_EC_SESSION_CANNOT_USE_SESSION_SECRET 455	///<A session secret is not permitted to be used with this type of session key 

///<Application Messaging Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_MESG_BANNED 500	///<Message contains banned content 
#define API_EC_MESG_NO_BODY 501	///<Missing message body 
#define API_EC_MESG_TOO_LONG 502	///<Message is too long 
#define API_EC_MESG_RATE 503	///<User has sent too many messages 
#define API_EC_MESG_INVALID_THREAD 504	///<Invalid reply thread id 
#define API_EC_MESG_INVALID_RECIP 505	///<Invalid message recipient 
#define API_EC_POKE_INVALID_RECIP 510	///<Invalid poke recipient 
#define API_EC_POKE_OUTSTANDING 511	///<There is a poke already outstanding 
#define API_EC_POKE_RATE 512	///<User is poking too fast 
#define API_EC_POKE_USER_BLOCKED 513	///<User cannot poke via API 

///<FQL Errors Errornumber PHP Constant name Error description Generated by methods 
#define FQL_EC_UNKNOWN_ERROR 600	///<An unknown error occurred in FQL fql.query,fql.multiquery 
#define FQL_EC_PARSER_ERROR 601	///<Error while parsing FQL statement fql.query,fql.multiquery 
#define FQL_EC_UNKNOWN_FIELD 602	///<The field you requested does not exist fql.query,fql.multiquery 
#define FQL_EC_UNKNOWN_TABLE 603	///<The table you requested does not exist fql.query,fql.multiquery 
#define FQL_EC_NO_INDEX 604	///<Your statement is not indexable fql.query,fql.multiquery 
#define FQL_EC_UNKNOWN_FUNCTION 605	///<The function you called does not exist fql.query,fql.multiquery 
#define FQL_EC_INVALID_PARAM 606	///<Wrong number of arguments passed into the function fql.query,fql.multiquery 
#define FQL_EC_INVALID_FIELD 607	///<FQL field specified is invalid in this context. fql.query*,fql.multiquery 
#define FQL_EC_INVALID_SESSION 608	///<An invalid session was specified fql.query,fql.multiquery 
#define FQL_EC_UNSUPPORTED_APP_TYPE 609	///<FQL field specified is invalid in this context. fql.query*,fql.multiquery 
#define FQL_EC_SESSION_SECRET_NOT_ALLOWED 610	///<FQL field specified is invalid in this context. fql.query*,fql.multiquery 
#define FQL_EC_DEPRECATED_TABLE 611	///<FQL field specified is invalid in this context. fql.query*,fql.multiquery 
#define FQL_EC_EXTENDED_PERMISSION 612	///<The stream requires an extended permission fql.query,fql.multiquery 
#define FQL_EC_RATE_LIMIT_EXCEEDED 613	///<Calls to stream have exceeded the rate of 100 calls per 600 seconds. fql.query,fql.multiquery 
#define FQL_EC_UNRESOLVED_DEPENDENCY 614	///<Unresolved dependency in multiquery fql.multiquery 
#define FQL_EC_INVALID_SEARCH 615	///<This search is invalid fql.query,fql.multiquery 
#define FQL_EC_TOO_MANY_FRIENDS_FOR_PRELOAD 617	///<The user you queried against has too many friends to be used with Preload FQL, in order to avoid out of memory errors fql.query,fql.multiquery 

/* This error is returned when the field name is sometimes valid, but not all the time. For example, if you run fql.query on the Metrics FQL table, you can get this error because some metrics are queryable only over the daily period, as opposed to the weekly or monthly periods.*/

///<Ref Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_REF_SET_FAILED 700	///<Unknown failure in storing ref data. Please try again. 

///<Application Integration Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_FB_APP_UNKNOWN_ERROR 750	///<Unknown Facebook application integration failure. 
#define API_EC_FB_APP_FETCH_FAILED 751	///<Fetch from remote site failed. 
#define API_EC_FB_APP_NO_DATA 752	///<Application returned no data. This may be expected or represent a connectivity error. 
#define API_EC_FB_APP_NO_PERMISSIONS 753	///<Application returned user had invalid permissions to complete the operation. 
#define API_EC_FB_APP_TAG_MISSING 754	///<Application returned data, but no matching tag found. This may be expected. 
#define API_EC_FB_APP_DB_FAILURE 755	///<The database for this object failed. 

///<Data Store API Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_DATA_UNKNOWN_ERROR 800	///<Unknown data store API error 
#define API_EC_DATA_INVALID_OPERATION 801	///<Invalid operation 
#define API_EC_DATA_QUOTA_EXCEEDED 802	///<Data store allowable quota was exceeded 
#define API_EC_DATA_OBJECT_NOT_FOUND 803	///<Specified object cannot be found 
#define API_EC_DATA_OBJECT_ALREADY_EXISTS 804	///<Specified object already exists 
#define API_EC_DATA_DATABASE_ERROR 805	///<database error occurred. Please try again 
#define API_EC_DATA_CREATE_TEMPLATE_ERROR 806	///<Unable to add FBML template to template database. Please try again. 
#define API_EC_DATA_TEMPLATE_EXISTS_ERROR 807	///<No active template bundle with that ID or handle exists. 
#define API_EC_DATA_TEMPLATE_HANDLE_TOO_LONG 808	///<Template bundle handles must contain less than or equal to 32 characters. 
#define API_EC_DATA_TEMPLATE_HANDLE_ALREADY_IN_USE 809	///<Template bundle handle already identifies a previously registered template bundle, and handles can not be reused. 
#define API_EC_DATA_TOO_MANY_TEMPLATE_BUNDLES 810	///<Application has too many active template bundles, and some must be deactivated before new ones can be registered. 
#define API_EC_DATA_MALFORMED_ACTION_LINK 811	///<One of more of the supplied action links was improperly formatted. 
#define API_EC_DATA_TEMPLATE_USES_RESERVED_TOKEN 812	///<One …or more of your templates is using a token reserved by Facebook, such as {*mp3*} or {*video*}. 

///<Mobile/SMS Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_SMS_INVALID_SESSION 850	///<Invalid sms session. 
#define API_EC_SMS_MSG_LEN 851	///<Invalid sms message length. 
#define API_EC_SMS_USER_QUOTA 852	///<Over user daily sms quota. 
#define API_EC_SMS_USER_ASLEEP 853	///<Unable to send sms to user at this time. 
#define API_EC_SMS_APP_QUOTA 854	///<Over application daily sms quota/rate limit. 
#define API_EC_SMS_NOT_REGISTERED 855	///<User is not registered for Facebook Mobile Texts 
#define API_EC_SMS_NOTIFICATIONS_OFF 856	///<User has SMS notifications turned off 
#define API_EC_SMS_CARRIER_DISABLE SMS 857	///<application disallowed by mobile operator 

///<Application Information Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_NO_SUCH_APP 900	///<No such application exists. 

///<Batch API Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_BATCH_TOO_MANY_ITEMS 950	///<Each batch API can not contain more than 20 items 
#define API_EC_BATCH_ALREADY_STARTED 951	///<begin_batch already called, please make sure to call end_batch first. 
#define API_EC_BATCH_NOT_STARTED 952	///<end_batch called before begin_batch. 
#define API_EC_BATCH_METHOD_NOT_ALLOWED_IN_BATCH_MODE 953	///<This method is not allowed in batch mode. 

///<Events API Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_EVENT_INVALID_TIME 1000	///<Invalid time for an event. events.edit 
#define API_EC_EVENT_NAME_LOCKED 1001	///<You are no longer able to change the name of this event. events.edit 

///<Info Section Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_INFO_NO_INFORMATION 1050	///<No information has been set for this user profile.setInfo 
#define API_EC_INFO_SET_FAILED 1051	///<Setting info failed. Check the formatting of your info fields. profile.setInfo 

///<LiveMessage Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_LIVEMESSAGE_SEND_FAILED 1100	///<An error occurred while sending the LiveMessage. liveMessage.send 
#define API_EC_LIVEMESSAGE_EVENT_NAME_TOO_LONG 1101	///<The event_name parameter must be no longer than 128 bytes. liveMessage.send 
#define API_EC_LIVEMESSAGE_MESSAGE_TOO_LONG 1102	///<The message parameter must be no longer than 1024 bytes. liveMessage.send 

///<Chat Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_CHAT_SEND_FAILED 1200	///<An error occurred while sending the message. 

///<Facebook Page Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_PAGES_CREATE 1201	///<You have created too many pages 

///<Facebook Links Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_SHARE_BAD_URL 1500	///<The url you supplied is invalid 

///<Facebook Notes Errors Error number PHP Constant name Error description Generated by methods 
#define API_EC_NOTE_CANNOT_MODIFY 1600	///<The user does not have permission to modify this note. 

///>Comment Errors Errornumber PHP Constant name Error description Generated by methods 
#define API_EC_COMMENTS_UNKNOWN 1700	///<An unknown error has occurred. 
#define API_EC_COMMENTS_POST_TOO_LONG 1701	///<The specified post was too long. 
#define API_EC_COMMENTS_DB_DOWN 1702	///<The comments database is down. 
#define API_EC_COMMENTS_INVALID_XID 1703	///<The specified xid is not valid. xids can only contain letters, numbers, and underscores 
#define API_EC_COMMENTS_INVALID_UID 1704	///<The specified user is not a user of this application 
#define API_EC_COMMENTS_INVALID_POST 1705	///<There was an error during posting. 
#define API_EC_COMMENTS_INVALID_REMOVE 1706	///<While attempting to remove the post. 

typedef struct facebook_atomic_data{
	char * data;
	unsigned int data_len;
}facebook_atomic;

typedef struct facebook_user_data{
	facebook_atomic id;		///<The user's Facebook ID
	facebook_atomic name;		///<The user's full name
	facebook_atomic firstname;	///<The user's first name
	facebook_atomic lastname;	///<The user's last name
	facebook_atomic link;		///<The URL of the profile for the user on Facebook
	facebook_atomic gender;		///<The user's gender
	facebook_atomic third_party_id;	///<An anonymous, but unique identifier for the user
	facebook_atomic email;		///<The proxied or contact email address granted by the user
	int		timezone;	///<The user's timezone offset from UTC
	facebook_atomic locale;		///<The user's locale
	boolean		verified;	///<The user's account verification status
	facebook_atomic updated_time;	///<The last time the user's profile was updated
	facebook_atomic about;		///<The blurb that appears under the user's profile picture
	facebook_atomic bio;		///<The user's biography
	facebook_atomic birthday;	///<The user's birthday
}facebook_user;

typedef struct facebook_from_data{
	facebook_atomic name;		///<The name of profile
	facebook_atomic id;		///<The ID of profile
}facebook_from;

typedef struct facebook_comment_data{
	facebook_atomic id;		///<the comment ID
	facebook_from from;		///<the profile that posted this comment
	facebook_atomic message;	///<the message of this comment
	facebook_atomic created_time;	///<The time the comment was initially published
}facebook_comment;

typedef struct facebook_picture_data{
	int height;			///<picture height
	int width;			///<picture width
	facebook_atomic source;		///<picture download address
}facebook_picture;

typedef struct facebook_image_data{
	facebook_picture picture_1024_768;
	facebook_picture picture_720_540;
	facebook_picture picture_180_135;
	facebook_picture picture_130_97;
	facebook_picture picture_75_56;
}facebook_image;

typedef struct facebook_photo_info{
	facebook_atomic id;		///<The photo ID
	facebook_from   from;		///<The profile (user or page) that posted this photo
	facebook_atomic name;		///<The caption given to this photo
	facebook_atomic picture;	///<the address of this photo in 130*97 size
	facebook_atomic source;		///<The full-sized source of the photo
	int		height;		///<The height of the photo in pixels
	int		width;		///<The width of the photo in pixels
	facebook_picture images[5];	///<The photo information(height width source) of different size
	facebook_atomic link;		///<A link to the photo on Facebook
	facebook_atomic icon;		///<The icon that Facebook displays when photos are published to the Feed
	facebook_atomic created_time;	///<The time the photo was initially published
	facebook_atomic updated_time;	///<The last time the photo or its caption was updated
	facebook_comment comments;	///<the comments of this photo
}facebook_photo;

typedef struct facebook_album_info{
	facebook_atomic id;		///<The photo album ID
	facebook_from   from;		///<The profile that created this album
	facebook_atomic name;		///<The title of the album
	facebook_atomic description;	///<The description of the album
	facebook_atomic link;		///<A link to this album on Facebook
	facebook_atomic cover_photo;	///<The cover photo
	facebook_atomic privacy;	///<The privacy settings for the album
	int		count;		///<The number of photos in this album
	facebook_atomic type;		///<The type of this album
	facebook_atomic created_time;	///<The time the photo album was initially created
	facebook_atomic updated_time;	///<The last time the photo album was updated
	facebook_comment comments;	///<The comments of this album
	facebook_photo * photo_entry;	///<The pointer to photos of this album
}facebook_album;

typedef struct facebook_photoalbums_data{
	int album_num;			///number of album
	facebook_album * album_entry;	///<The pointer to the albums
}facebook_photoalbums;

typedef struct facebook_member_data{
	facebook_atomic id;		///<The member ID
	facebook_atomic name;		///<The member name
}facebook_member;

typedef struct facebook_friendlist_data{
	facebook_atomic id;		///<The friend list ID
	facebook_atomic name;		///<The friend list name
	int		member_num;	///<The member num of this friendlist
	facebook_member * member_entry;	///<The pointer to member of this friendlist
}facebook_friendlist;

typedef struct facebook_friendlists_info{
	int friendlist_num;			///<The num of friendlists
	facebook_friendlist * friendlist_entry;	///<The pointer to friendlist
}facebook_friendlists;

typedef struct facebook_contact_info{
	int contact_num;
	facebook_member * member_entry;
}facebook_contact;

typedef enum facebook_type_s{
	ALBUM_FEED,
	PHOTO_FEED,
	USER_PROFILE,
	CONTACT_FEED,
}facebook_type;

typedef struct facebook_feed_s{
	facebook_type type;
	void * feed_p;
}facebook_feed;

typedef struct facebook_photo_down_info_s ///< the info for downloading photo
{
	facebook_feed * f_feed;
	unsigned char isthumbnail;		///< whether it is a thumbnail	
	int which_entry;				///< which entry does the photo in
	int is_cache;
	facebook_atomic cache_dir;
	facebook_atomic album_id;
	///< for uncache
	facebook_write_data *photo_data; 	///< if the cached==0, the photo data will be stored at here
}facebook_photo_down_info;

typedef struct faecbook_update_s
{
	facebook_data * fdata;
	int iscache;
	char * cache_dir;
	int isthumbnail;
}facebook_update_t;

typedef enum{
	///<for album amd common
	ID,
	FROM,
	NAME,
	DESCRIPTION,
	LINK,
	COVER_PHOTO,
	PRIVACY,
	COUNT,
	TYPE,
	CREATED_TIME,
	UPDATED_TIME,
	COMMENTS,

	///<for photo
	PICTURE,
	SOURCE,
	HEIGHT,
	WIDTH,
	IMAGES,
	ICON,

	///<for comments
	MESSAGE,

	///<for user
	FIRST_NAME,
	LAST_NAME,
	GENDER,
	THIRD_PARTY_ID,
	EMAIL,
	TIMEZONE,
	LOCALE,
	VERIFIED,
	ABOUT,
	BIO,
	BIRTHDAY,
}facebook_element_type;

typedef enum{
	USER_INFO,
	USER_PICTURE,
	ALBUMS_INFO,
	ALBUM_COVER,
	ALBUM_COMMENT,
	ALBUMPHOTO_INFO,
	DOWNLOAD_PHOTO_BIG,
	DOWNLOAD_PHOTO_SMALL,
	FRIENDS_LIST,
	FRIENDSLIST_MEMBERS,
	FRIEND_ALBUMS_INFO,
	FRIEND_ALBUMPHOTO_INFO,
	FRIEND_DOWNLOAD_PHOTO_BIG,
	FRIEND_DOWNLOAD_PHOTO_SMALL,
	FRIENDS,
	FRIEND_ALBUM_COVER,
	UPLOAD_FEED,
	UPLOAD_COMMENTS,
	CREATE_ALBUM,
	UPLOAD_PHOTO,
}facebook_cmd;

typedef enum{
	FACEBOOK_CMD_DOWNLOADPHOTO,
	FACEBOOK_CMD_STOPDOWNLOAD,
	FACEBOOK_CMD_UPLOADPHOTO,
	FACEBOOK_CMD_STOPUPLOAD,
	FACEBOOK_CMD_AUTH,
	FACEBOOK_CMD_STOPAUTH,
}facebook_ioctl_cmd;

typedef enum{
	FACEBOOK_QUERY_CMD_RESULT,    ///< query the result 
	FACEBOOK_QUREY_CMD_PROGRESS,	///< query the progress
}FacebookwebQueryCmd;

typedef enum{
	FACEBOOK_REQ_INVALID,
	FACEBOOK_REQ_ENQUEUE,
	FACEBOOK_REQ_DEQUEUE,
	FACEBOOK_REQ_DONE,
}facebook_req_status;

typedef struct facebook_ioctrl_t
{
	facebook_req_status active;
	unsigned long timestamp;
	facebook_ioctl_cmd iocmd;
	void *para;
	int para_size;
}facebook_ioctrl;

facebook_photoalbums * facebook_get_albums_info(facebook_data *f_data, facebook_contact *contact, int which_friend);

int get_albumphoto_info(facebook_data * f_data, int index, facebook_photoalbums * photoalbums);

facebook_user * get_user_info(facebook_data * f_data);

int get_listmembers_info(facebook_data * f_data, facebook_friendlist * friendlist_entry);

facebook_friendlists * get_friends_list(facebook_data * f_data);

facebook_contact * get_contact(facebook_data * f_data);

int facebook_free_album(facebook_album * album);

int facebook_free_photoalbums(facebook_photoalbums *photoalbums);

int facebook_free_friendlist(facebook_friendlist *friendlist);

int facebook_free_friendlists(facebook_friendlists *friendlists);

int facebook_free_contact(facebook_contact *contact);

int facebook_free_userinfo(facebook_user *userinfo);

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

int facebook_send_msg(facebook_data *f_data,facebook_ioctl_cmd cmd,void * para);

int facebook_query_download_status(facebook_data *f_data,facebook_photo_down_info * down_info,FacebookwebQueryCmd query_cmd);

int facebook_get_cache_path(facebook_feed *feed,int which_entry,int isthumbnail,char *pathbuf,int buf_len);

int facebook_save_albums_info(facebook_data *f_data,facebook_photoalbums * feed_albums,char* cache_dir,char* filename);

int facebook_save_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir);

int facebook_save_contact_info(facebook_data *f_data,facebook_contact * feed_contact,char* cache_dir);

facebook_photoalbums * facebook_load_albums_info(facebook_data *f_data,char* cache_dir,char *filename);

int facebook_load_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir);

facebook_contact* facebook_load_contact_info(facebook_data *f_data,char* cache_dir);
#endif
