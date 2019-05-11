/**
*@addtogroup medialib_lib_s 
*@{
*/

#ifndef _MLIB_API_H
#define _MLIB_API_H


/**
@file: mlib_api.h

@abstract: medialib  api definations head file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Media Library;
 *  include photo,music,video Library	 
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
 */


/**
	data base attribute mask definations
			total 32  	
	1.basic file info:1~5
	2.minfo file info: 6~24
		photo: 6~10(9,10 reserved)
		music:11~15(16~24 reserved for video and other music attr)
	3.tag info:25~29
	4:reserved:20~31
*/

/**	basic file attr mask */
#define DB_FILE_PATH		1<<1   /**<must be contained */
#define DB_FILE_NAME			1<<2   /**<must be contained */
#define DB_FILE_SIZE			1<<3
#define DB_FILE_MTIME		1<<4

/** photo attr mask */
#define DB_EXIF_TIME		1<<6
#define DB_EXIF_HEIGHT		1<<7
#define DB_EXIF_WIDTH		1<<8

/** music attr mask */
#define DB_ID3_ARTISTS		1<<11
#define DB_ID3_ALBUM		1<<12
#define DB_ID3_GENRES		1<<13
#define DB_ID3_COMPOSER	1<<14
#define DB_ID3_TIME			1<<15

/** video attr mask**/
/** user define attr mask */
#define DB_USER_TAG1		1<<25
#define DB_USER_TAG2		1<<26
#define DB_USER_TAG3		1<<27
#define DB_USER_TAG4		1<<28
#define DB_USER_TAG5		1<<29



/** view mode defination */
#define VIEW_MODE_REMOVE_DUPLCATE			1<<1
#define VIEW_MODE_FITLER_BYVALUE_EQUAL		1<<2
#define VIEW_MODE_FITLER_BYVALUE_BITEQUAL	1<<3
#define VIEW_MODE_PURE_SORT					1<<4
//#define VIEW_MODE_SORT_ASCEND			1<<4
//#define VIEW_MODE_SORT_DECEND			0<<4

/** modify mode defination */
#define MODIFY_MODE_CHANGETO			1
#define MODIFY_MODE_XOR				2
#define MODIFY_MODE_AND				3

/** search mode defination  */
#define SEARCH_MODE_SEQUENCE			1
#define SEARCH_MODE_REVERSE			2

/** database view and attr	*/
typedef void*			medialib_t;
typedef	void*			medialib_view_t;
typedef unsigned int 	view_mode_t;
typedef unsigned int 	modify_mode_t;
typedef unsigned int    search_mode_t;
typedef	unsigned int 	medialib_attr_mask_t;

/** database media type*/
enum minfo_type{
	MINFO_TYPE_PHOTO = 0,
	MINFO_TYPE_MUSIC = 1,
	MINFO_TYPE_VIDEO = 2
};

/**  database storage type	*/
enum storage_type{
	VRAM_STORAGE = 0,
	SDRAM_STORAGE = 1,
	FILE_STORAGE = 2	
};

/** datastructure for create database*/
struct medialib_info{
	unsigned int max_support_file_num;
	unsigned int max_support_dir_num;
	
	medialib_attr_mask_t	attr_mask;
	enum minfo_type m_type;
	enum storage_type s_type;
	char*	file_format;
	
	char* rootpath;
	
	char* name;
	//char create_should_stop;

};

/**	datastructure for open view */
struct medialib_view_info{
	medialib_t medialib;
	medialib_view_t view;
	int  db_metadata;
	void* value;
	view_mode_t	mode;
	void* reseved;
};

struct user_info{
	unsigned int user_tag1;
	unsigned int user_tag2;
	unsigned int user_tag3;
	unsigned int user_tag4;
	unsigned int user_tag5;
};

struct db_info{
	unsigned int max_support_file_num;
	unsigned int max_support_dir_num;

	unsigned int file_num;	
	unsigned int dir_num;
	
	medialib_attr_mask_t	attr_mask;
	enum minfo_type m_type;
	enum storage_type s_type;
	char	file_format[24];
	
	char name[64];	
};


#define SDRAM_DB_NUM 3

struct sdram_dbname_s{
	unsigned int start_addr;		///< the start of the memory
	long mem_size;			///< the length of the memory
};

#define FULL_PATH_LEN		1024

/**
*@brief API for create medialib
*
*@param[in] info	: create info structure 
*@retval 0		: success
*@retval	!0		: error
*/
int medialib_create(struct medialib_info* info); 

/**
*@brief API for stop create medialib
*
*@param[in] type		: media info type
*@param[in] s_type	: storage type
*@param[in] name		: medialib name 
*@retval 0			: success
*@retval	!0			: error number
*/
int medialib_stop_create(struct medialib_info *info);

/**
*@brief API for open medialib
*
*@param[in] type		: media info type
*@param[in] s_type	: storage type
*@param[in] name		: medialib name
*@retval !0			: medialib handle
*@retval 0 			: error number
*/
medialib_t medialib_open(enum minfo_type type,enum storage_type s_type,char* name);

/**
*@brief API for close medialib
*
*@param[in] medialib	: medialib handle
*@retval 	0 			: success
*@retval 	!0			: error 
*/
int medialib_close(medialib_t medialib);

/**
*@brief API for get medialib info
*
*@param[in] medialib	: medialib handle
*@param[in] info		: memory which medialib info is to be stored
*@retval 0			: success
*@retval 	!0 			: error 
*/
int	medialib_getdb_info(medialib_t medialib,struct db_info* info);

/**
*@brief API for open view
*
*@param[in] info	: detailed view info
*@retval 	!0 		: view handle
*@retval 	0 		: error 
*/
medialib_view_t medialib_open_view(struct medialib_view_info *info);

/**
*@brief API for close view
*
*@param[in] view	: view handle
*@retval 0 		: success
*@retval	!0          : error 
*/
int medialib_close_view(medialib_view_t view);

/**
*@brief API for insert item
*
*@param[in] medialib	: medialib handle
*@param[in] filename	: media file name
*@param[in] u_info 	: user info for the file
*@retval 	0			: success
*@retval !0			: error 
*/
int medialib_insert(medialib_t medialib,char* filename,struct user_info* u_info);

/**
*@brief API for delete item
*
*@param[in] view		: view handle
*@param[in] index		: index array which store file indexs to be deleted
*@param[in] num		: index array size
*@retval 	0			: success
*@retval 	!0			: error 
*/
int medialib_delete(medialib_view_t view,unsigned int* index,unsigned int num);

/**
*@brief API for select item
*
*@param[in] view 		: view handle
*@param[in] index 	: index array which store file indexs to be selected
*@param[in] num		: index array size
*@param[in] file_path	: memory to store the select item filepath
*@param[in] u_info 	: memory to store the select item user info
*@retval 	0			: success
*@retval	!0			: error 
*/
int medialib_select(medialib_view_t view,unsigned int* index,unsigned int num,char file_path[][FULL_PATH_LEN],struct user_info *u_info);

/**
*@brief API for modify item
*
*@param[in] view		: view handle
*@param[in] metadata	: modified item
*@param[in] index		: index array which store file indexs to be modified
*@param[in] num  		: index array size
*@param[in] value 	: memory to store the mode item metadatavalue
*@param[in] mode		: modify mode
*@retval 	0			: success
*@retval 	!0			: error 
*/
int medialib_modify(medialib_view_t view,int metadata,unsigned int* index,unsigned int num,void* value,modify_mode_t mode);

/**
*@brief API for search item
*
*@param[in] view		: view handle
*@param[in] metadata	: modified item
*@param[in] value		: memory to store the mode item metadatavalue
*@param[in] mode		: search mode
*@retval 0			: success
*@retval	!0			: error 
*/
int medialib_search(medialib_view_t view,int metadata,void* value,search_mode_t mode);

/**
*@brief API for get item number
*
*@param[in] view	: view handle
*@retval			: item num
*/
int medialib_getnum(medialib_view_t view);

/**
*@brief API for update medialib
*
*@param[in] medialib	: medialib handle
*@retval 	0			: success
*@retval !0			: error 
*/
int medialib_update(medialib_t medialib);

/**
*@brief API for desroy medialib
*
*@param[in] type		: media info type
*@param[in] s_type	: storage type
*@param[in] name		: medialib name
*@retval 	0			: success
*@retval 	!0			: error 
*/
int medialib_destroy(enum minfo_type type,enum storage_type s_type,char* name);


#endif

/**
 *@}
 */
