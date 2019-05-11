//#ifndef _DB_AUDIO_H
#define _DB_AUDIO_H

#include "sqlite3.h"



enum DBAudioSortType{
	DBVIDEO_SORT_BY_NAME=0,
	DBVIDEO_SORT_BY_FILESIZE,
	DBVIDEO_SORT_BY_FILEEXTENSION,
	DBVIDEO_SORT_BY_TIME,

};

/** 
* for album management 
*/

/** 
* for songs view management.
*/
typedef struct _DBVideoInfo DBVideoInfo;
struct _DBVideoInfo
{
	int totalVideos;
	int *sortIndex;
};
typedef struct _DBVideoViewInfo DBVideoViewInfo;
struct _DBVideoViewInfo
{
	DBVideoInfo videos;
};


typedef struct _DBVideoManager DBVideoManager;

struct _DBVideoManager
{
	/// the SQLite handler.
	sqlite3 *sqlHandler;
	DBVideoViewInfo videoView;


};

/**
* @brief Open the audio database file.
*
* @param path : the full path of the data base file.
*
* @return return an audio data base handler.
*/
extern DBVideoManager *DBVideoOpen(char *path);


/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
extern void DBVideoClose(DBVideoManager *handler);

/**
* @brief Get the information for a specific view such as Album,Artist,Genre etc.
*
* @param handler : the data base handler.
* @param viewType: the specific viewType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
extern int DBVideoGetViewInfo(DBVideoManager *handler);


/**
* @brief Sort the album view.
*
* @param handler : the data base handler.
* @param album : the album name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/

extern int DBVideoDoViewSort(DBVideoManager *handler,int sortType);


extern char * DBVideoGetSortedViewItem(DBVideoManager *handler,int index);


/**
* @brief Create the audio database.
*
* @param filesDir: the audio file base directory.
* @param dbFullPath: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbvideoCreateDatabase(char *filesDir,char *dbFullPath);

/**
* @brief Delete the audio database.
*
* @param path: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbVideoDeleteDatabase(char *path);

/**
* @brief Get the status for creating a database.
*
* @return Return the percentage for creating, e.g. 50 stands for 50% and 100 for done.
*/
extern int DbVideoGetCreateStat();

/**
* @brief Stop creating the database.
*/
extern int DbVideoStopCreate();

//#endif
