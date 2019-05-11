#ifndef _ACT_PICASA_COMMON_H_
#define _ACT_PICASA_COMMON_H_


#include "webalbum_api.h"

#define CURL_OPT_TIMEOUT 60 //unit is second
#define ATOM_PRES_LEN 68  //define the max length of the atom presentation
#define TMP_BUF_LEN 256


typedef enum{
	VStatus_pending=0,	///< the video is still being processed
	VStatus_ready=1,	///< the video has been processed but still needs a thumbnail
	VStatus_final=2,	///< the video has been processed and has received a thumbnail
	VStatus_failed=3,	///< a processing error has occured and the video should be deleted
}PicasawebVideoStatus_e;

typedef enum{
	///< see http://code.google.com/intl/zh-CN/apis/picasaweb/docs/2.0/reference.html#Path_Values
	Query_access=0,		///< Visibility parameter
	Query_alt=1,		///< Alternative representation type
	Query_bbox=2,		///< bounding-box search of geo coordinates
	Query_fields=3,		///< Response filter
	Query_imgmax=4,		///< Image size parameter
	Query_kind=5,		///< Picasa Web Albums-specific query parameter for kind queries.
	Query_location=6,	///< named search of geo data
	Query_max_result=7,	///< Maximum number of results to be retrieved
	Query_prettyprint=8,	///< Returns an XML response with identations and line breaks
	Query_q=9,		///< Full-text query string
	Query_start_index=10,	///< 1-based index of the first result to be retrieved
	Query_tag=11,		///< Tag filter parameter
	Query_thumbsize=12,	///< Thumbnail size parameter
	Query_MAX
}PicasawebQueryPara_e;


typedef enum{
	///< see http://code.google.com/intl/zh-CN/apis/picasaweb/docs/2.0/reference.html
	Picasaweb_Photo_Visibility_All=0,	///< Shows all data, both public and private.
	Picasaweb_Photo_Visibility_Private=1,	///< Shows only private data.
	Picasaweb_Photo_Visibility_Public=2,	///< Shows only public data.
	Picasaweb_Photo_Visibility_Visible=3,	///< Shows all data the user has access to, including both all public photos or albums and any 						///< photos or albums that the owner has explicitly given the authenticated user rights to (using ACLs).
}PicasawebPhotoVisibility_e;


///< using the following num to find the atom presentation
typedef enum
{
	FEED_ALL=0,
	FEED_TITLE,
	FEED_ID,
	FEED_HTML_LINK,
	FEED_DESCRIPTION,	
	FEED_LANGUAGE,
	FEED_COPYRIGHT,
	FEED_AUTHOR_NAME,
	FEED_AUTHOR_EMAIL,
	FEED_LAST_UPDATE_DATE,
	FEED_CATEGORY,
	FEED_CATEGORY_SCHEME,
	FEED_GENERATOR,
	FEED_GENERATOR_URL,
	FEED_ICON,
	FEED_LOGO,

	OPENSEARCH_RESULT,
	OPENSEARCH_START_IDX,
	OPENSEARCH_NUM_PER_PAGE,

	ENTRY_ALL,
	ENTRY_ID,
	ENTRY_TITLE,
	ENTRY_LINK,
	ENTRY_SUMMARY,
	ENTRY_CONTENT,
	ENTRY_AUTHOR_NAME,
	ENTRY_AUTHOR_EMAIL,
	ENTRY_CATEGORY,
	ENTRY_CATEGORY_SCHEME,
	ENTRY_PUBLIC_DATE,
	ENTRY_UPDATE_DATE,
	
	PHOTO_ID,
}picasaweb_atom_pres_e;

///< using the following num to get the operation of the node
typedef enum{
	///< namespace
	NS_ATOM,
	NS_GPHOTO,
	NS_MEDIA,
	NS_GEORSS,

	///both atom and gphoto
	NODE_ID,///both feed and entry used
	NODE_ALBUMID,

	///both atom and media
	NODE_TITLE,///both feed and entry used
	NODE_CONTENT,

	///< for atom space
	NODE_LINK,	
	NODE_SUMMARY,

	NODE_AUTHOR,
	NODE_AUTHOR_NAME,
	NODE_AUTHOR_EMAIL,
	NODE_AUTHOR_URI,
	NODE_CATEGORY,///both feed and entry used
	NODE_PUBLIC_DATE,
	NODE_UPDATE_DATE,///both feed and entry used
	NODE_EDIT_DATE,
	NODE_RIGHTS,

	CATEGORY_ATTR_TERM,
	CATEGORY_ATTR_SCHEME,
	LINK_ATTR_REL,
	LINK_ATTR_TYPE,
	LINK_ATTR_HREF,
	
	///< for gphoto space using in entry
	NODE_PHOTO_LOCATION,
	NODE_PHOTO_ACCESS,
	NODE_PHOTO_TIMESTAMP,
	NODE_PHOTO_NUMPHOTOS,
	NODE_PHOTO_NUMPHOTOREMAIN,
	NODE_PHOTO_BYTESUSED,
	NODE_PHOTO_USER,
	NODE_PHOTO_NICHNAME,
	
	///< for media space using in entry
	NODE_GROUP,
	NODE_GROUP_CREDIT,
	NODE_GROUP_DESC,
	NODE_GROUP_KEYWORDS,

	NODE_THUMBNAIL,///using in entry and feed
	
	///< for georss space using in entry
	NODE_WHERE,
	NODE_POINT,
	NODE_POS,

	///<using in feed
	NODE_SUBTITLE,
	NODE_TOTALRESULT,
	NODE_STARTINDEX,
	NODE_ITEMSPERPAGE,

	NODE_QUOTALIMIT,
	NODE_QUOTACURRENT,
	NODE_MAXPHOTOSPERALBUM,

	///< for exif space
	NODE_TAGS,
	NODE_FSTOP,
	NDDE_MAKE,
	NODE_MODEL,
	NODE_EXPOSURE,
	NODE_FLASH,
	NODE_FOCALLENGTH,
	NODE_ISO,
	NODE_TIME,
	NODE_IMAGEUNIQUEID,
	NODE_DISTANCE,
}picasaweb_entry_node_e;

typedef enum{
	PICASA_PHOTO_TYPE_JPEG,
	PICASA_PHOTO_TYPE_BMP,
	PICASA_PHOTO_TYPE_PNG,
	PICASA_PHOTO_TYPE_GIF
}picasaweb_photo_type_e;

typedef struct picasaweb_schema_item_s
{
	unsigned short schema;
	char atom_pres[ATOM_PRES_LEN];
}picasaweb_schema_item_t;


int  picasa_fill_atomic(xmlChar * content,picasaweb_data_atomic_t *atomic_name);
int picasa_free_atomic(picasaweb_data_atomic_t *atomic_name);
int picasa_get_ini_albumsname(picasaweb_gdata_t *gdata,char* cache_dir,char*filename,char* namebuf,int len);
int picasa_get_ini_albumname(picasaweb_feed_t *feed_albums,int which_album,char* cache_dir,char* namebuf,int len);

#endif
