#ifndef _ALBUM_H_
#define _ALBUM_H_

#include "swf_ext.h"
#include "mlib_api.h"

/**Attr Type:**/
//basic attr
#define ML_ATTR_FILE_NAME 				(1<<0)
#define ML_ATTR_FILE_SIZE 				(1<<1)
#define ML_ATTR_FILE_TIME				(1<<2)
#define ML_ATTR_FILE_PATH				(1<<3)

//photo attr
#define ML_ATTR_EXIF_TIME				(1<<4)
#define ML_ATTR_EXIF_HEIGHT			(1<<5)
#define ML_ATTR_EXIF_WIDTH			(1<<6)

//music attr
#define ML_ATTR_ID3_ARTISTS			(1<<8)
#define ML_ATTR_ID3_ALBUM				(1<<9)
#define ML_ATTR_ID3_GENRES				(1<<10)
#define ML_ATTR_ID3_COMPOSER			(1<<11)
#define ML_ATTR_ID3_TIME				(1<<12)

//tag attr
#define ML_ATTR_USER_TAG1				(1<<13)
#define ML_ATTR_USER_TAG2				(1<<14)
#define ML_ATTR_USER_TAG3				(1<<15)
#define ML_ATTR_USER_TAG4				(1<<16)
#define ML_ATTR_USER_TAG5				(1<<17)

enum info_type_e{
	INFO_TYPE_PHOTO                   = 0,   //photo info
	INFO_TYPE_MUSIC                   = 1,   //music info
	INFO_TYPE_VIDEO                   = 2,   //video info
	INFO_TYPE_PHOTO_USR_TAG  = 3,   //read photo info and usr tag(s)
	INFO_TYPE_USR_TAG_R           = 4,   //read usr tag(s) only
	INFO_TYPE_USR_TAG_W          = 5,    //write usr tag(s)
	INFO_TYPE_INVAILD			=6,
};


#endif
