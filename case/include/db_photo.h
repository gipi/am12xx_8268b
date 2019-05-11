//#ifndef _DB_AUDIO_H

#include "sqlite3.h"



enum DBAudioSortType{
	DBPHOTO_SORT_BY_NAME=0,
	DBPHOTO_SORT_BY_FILESIZE,
	DBPHOTO_SORT_BY_FILEEXTENSION,
	DBPHOTO_SORT_BY_TIME,

};

/** 
* for album management 
*/

/** 
* for songs view management.
*/
typedef struct _DBPhotoInfo DBphotoInfo;
struct _DBPhotoInfo
{
	int totalphotos;
	int *sortIndex;
};
typedef struct _DBphotoViewInfo DBPhotoViewInfo;
struct _DBphotoViewInfo
{
	DBphotoInfo photos;
};


typedef struct _DBPhotoManager DBPhotoManager;

struct _DBPhotoManager
{
	/// the SQLite handler.
	sqlite3 *sqlHandler;
	DBPhotoViewInfo photoView;


};

/**
* @brief Open the audio database file.
*
* @param path : the full path of the data base file.
*
* @return return an audio data base handler.
*/
extern DBPhotoManager *DBPhotoOpen(char *path);


/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
extern void DBPhotoClose(DBPhotoManager *handler);

/**
* @brief Get the information for a specific view such as Album,Artist,Genre etc.
*
* @param handler : the data base handler.
* @param viewType: the specific viewType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
extern int DBPhotoGetViewInfo(DBPhotoManager *handler);


/**
* @brief Sort the album view.
*
* @param handler : the data base handler.
* @param album : the album name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/

extern int DBPhotoDoViewSort(DBPhotoManager *handler,int sortType);


extern char * DBPhotoGetSortedViewItem(DBPhotoManager *handler,int index);


/**
* @brief Create the audio database.
*
* @param filesDir: the audio file base directory.
* @param dbFullPath: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbphotoCreateDatabase(char *filesDir,char *dbFullPath);

/**
* @brief Delete the audio database.
*
* @param path: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbPhotoDeleteDatabase(char *path);

/**
* @brief Get the status for creating a database.
*
* @return Return the percentage for creating, e.g. 50 stands for 50% and 100 for done.
*/
extern int DbPhotoGetCreateStat();

/**
* @brief Stop creating the database.
*/
extern int DbPhotoStopCreate();

//#endif
