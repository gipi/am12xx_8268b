#ifndef _WEBALBUM_ENGINE_H_
#define _WEBALBUM_ENGINE_H_

#ifdef MODULE_CONFIG_WEBALBUM

typedef enum{
	PARA_TYPE_FEED,
	PARA_TYPE_ENTRY,
	PARA_TYPE_DOWNLOAD,
	PARA_TYPE_UPLOADE,
	PARA_TYPE_ERR_INIT_DOWNLOAD,
	PARA_TYPE_PHOTOALBUMS,
	PARA_TYPE_SINGLEALBUM,
	PARA_TYPE_SINGLEPHOTO,
	PARA_TYPE_CONTACT,
	PARA_TYPE_SINGLEMEMBER,
}webalbum_para_type_e;

typedef enum{
	FEED_ENTRY_NUM,
	FEED_ALBUMID,
	FEED_ID,
	FEED_UPDATED,
	FEED_TITLE,
	FEED_SUBTITLE,
	FEED_AUTHOR_NAME,
	FEED_USER,
	FEED_ACCESS,
	FEED_BYTESUSED,
	FEED_LOCATION,
	FEED_NUMPHOTOS,
	FEED_HEIGHT,
	FEED_WIDTH,
	FEED_TIMESTAMP,
	ENTRY_EXIF_DISTANCE,
	ENTRY_EXIF_EXPOSURE,
	ENTRY_EXIF_FLASH,
	ENTRY_EXIF_FOCALLENGTH,
	ENTRY_EXIF_FSTOP,
	ENTRY_EXIF_IMG_UNIQUEID,
	ENTRY_EXIF_ISO,
	ENTRY_EXIF_MAKE,
	ENTRY_EXIF_MODEL,
	ENTRY_EXIF_TIME,
	ENTRY_PUBLISHED,
	ENTRY_UPDATED,
	ENTRY_EDITED,
	ENTRY_RIGHT,
	ENTRY_SUMMARY,
	ENTRY_TITLE,
	ENTRY_AUTHOR_NAME,
	ENTRY_WHERE_POS,
	ENTRY_ACCESS,
	ENTRY_BYTESUSED,
	ENTRY_LOCATION,
	ENTRY_NUMPHOTOS,
	ENTRY_NUMPHOTOREMAINING,
	ENTRY_WIDHT,
	ENTRY_HEIGHT,
	ENTRY_TIMESTAMP,
	ENTRY_ALBUMTITLE,
	ENTRY_USERID,
	FEED_PHOTO_ADDR,
	FEED_PTOTO_THUMB_ADDR,
}webalbum_info_type_e;

typedef enum{
	ALBUM_NUM,
	ALBUM_ID,
	ALBUM_FROM_ID,
	ALBUM_FROM_NAME,
	ALBUM_NAME,
	ALBUM_DESCRIPTION,
	ALBUM_LINK,
	ALBUM_COVER_PHOTO,
	ALBUM_PRIVACY,
	ALBUM_PHOTOCOUNT,
	ALBUM_TYPE,
	ALBUM_CREATED_TIME,
	ALBUM_UPDATED_TIME,
	ALBUM_COMMENTS,
	PHOTO_ID,
	PHOTO_FROM_ID,
	PHOTO_FROM_NAME,
	PHOTO_NAME,
	PHOTO_PICTURE,
	PHOTO_SOURCE,
	PHOTO_HEIGHT,
	PHOTO_WIDTH,
	PHOTO_LINK,
	PHOTO_ICON,
	PHOTO_CREATED_TIME,
	PHOTO_UPDATED_TIME,
	PHOTO_COMMENTS,
	FRIENDS_NUM,
	MEMBER_ID,
	MEMBER_NAME,
	PHOTO_PATH,
	PHOTO_THUMB_PATH,
}facebook_info_type_e;

typedef enum{
	UPLOAD_STATUS_CMD_RESULT,
	UPLOAD_STATUS_CMD_PROGRESS,
}webalbum_upload_status_cmd;

typedef struct date_time_s
{	
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
}date_time_t;

/***for flickr only***/
typedef enum{
	FLICKR_FEED_ALBUMS,
	FLICKR_FEED_ALBUM,
	FLICKR_FEED_PHOTO,
}flickr_feed_type_e;

typedef enum{
	FLICKR_GET_ACCOUNT_TYPE,  				///< get account type  see flickr_account_type_e, should change to int by the caller
	FLICKR_GET_LOGIN_EMAIL,					///< get login email
	FLICKR_GET_LOGIN_PWD,					///< get login password
	FLICKR_GET_FROB,							///< get frob
	FLICKR_GET_PERMS,							///< get perms

	FLICKR_GET_USER_REALNAME,				///< user's realname
	FLICKR_GET_USER_USERNAME,				///< user's username
	FLICKR_GET_CONTACT_COUNT,			///< how many contacts does the login account has, should change to int by the caller
	FLICKR_GET_CONTACT_REALNAME,			///< contact's realname
	FLICKR_GET_CONTACT_USERNAME,			///< contact's username

	FLICKR_GET_PHOTOSET_ID,					///< photoset id
	FLICKR_GET_PHOTOSET_PHOTOS_COUNT, 		///< photos' num in a photoset, should change to int by the caller
	FLICKR_GET_PHOTOSET_TITLE,				///< title of the photoset		
	FLICKR_GET_PHOTOSET_DESCRIPTION,			///< description of the photoset
	FLICKR_GET_PHOTOSET_COUNT,				///< how many photosets in the account, should change to int by the caller

	FLICKR_GET_PHOTOS_COUNT,					///< photos num in the account,  should change to int by the caller
	FLICKR_GET_PHOTO_TITLE,					///< photo title
	FLICKR_GET_PHOTO_DESCRIPTION,			///< photo description
	FLICKR_GET_PHOTO_ID,						///< photo id
	FLICKR_GET_PHOTO_DATESPOSTED,			///< photo dates posted

	FLICKR_GET_PHOTO_DATESTAKEN,			///< photo dates taken
	FLICKR_GET_PHOTO_DATESLASTUPDATE,		///< photo dates last update

	FLICKR_GET_URL_COVER_SMALL,				///< get photoset cover url, small picture
	FLICKR_GET_URL_COVER_HUGE,				///< get photoset cover url, huge picture
	FLICKR_GET_URL_PHOTO_SMALL,				///< get photo url, small picture
	FLICKR_GET_URL_PHOTO_HUGE,				///< get photo url, huge picture
}flickr_info_type_e;

struct eg_flickr_feed_info_s{
	flickr_feed_type_e feed_type;
	flickr_gdata_t * gdata;
	int which_contact;
	int which_photoset;
	int which_photo;
	int is_thumbnail;
	int entry_num;
	struct eg_flickr_feed_info_s **child_feed_array;
};

typedef struct eg_flickr_feed_info_s eg_flickr_feed_info_t;

#endif	/** MODULE_CONFIG_WEBALBUM */

#endif
