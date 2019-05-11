//#ifndef _DB_AUDIO_H
#define _DB_AUDIO_H

#include "sqlite3.h"



enum DBOfficeSortType{
	DBOFFICE_SORT_BY_NAME=0,
	DBOFFICE_SORT_BY_FILESIZE,
	DBOFFICE_SORT_BY_FILEEXTENSION,
	DBOFFICE_SORT_BY_TIME,

};

/** 
* for album management 
*/

/** 
* for songs view management.
*/
typedef struct _DBOfficeInfo DBOfficeInfo;
struct _DBOfficeInfo
{
	int totalOffices;
	int *sortIndex;
};
typedef struct _DBOfficeViewInfo DBOfficeViewInfo;
struct _DBOfficeViewInfo
{
	DBOfficeInfo offices;
};


typedef struct _DBOfficeManager DBOfficeManager;

struct _DBOfficeManager
{
	/// the SQLite handler.
	sqlite3 *sqlHandler;
	DBOfficeViewInfo officeView;


};

/**
* @brief Open the audio database file.
*
* @param path : the full path of the data base file.
*
* @return return an audio data base handler.
*/
extern DBOfficeManager *DBOfficeOpen(char *path);


/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
extern void DBOfficeClose(DBOfficeManager *handler);

/**
* @brief Get the information for a specific view such as Album,Artist,Genre etc.
*
* @param handler : the data base handler.
* @param viewType: the specific viewType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
extern int DBOfficeGetViewInfo(DBOfficeManager *handler);


/**
* @brief Sort the album view.
*
* @param handler : the data base handler.
* @param album : the album name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/

extern int DBOfficeDoViewSort(DBOfficeManager *handler,int sortType);


extern char * DBOfficeGetSortedViewItem(DBOfficeManager *handler,int index);


/**
* @brief Create the audio database.
*
* @param filesDir: the audio file base directory.
* @param dbFullPath: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
int DbofficeCreateDatabase(char *filesDir,char *dbFullPath,char *file_type);

/**
* @brief Delete the audio database.
*
* @param path: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbOfficeDeleteDatabase(char *path);

/**
* @brief Get the status for creating a database.
*
* @return Return the percentage for creating, e.g. 50 stands for 50% and 100 for done.
*/
extern int DbOfficeGetCreateStat();

/**
* @brief Stop creating the database.
*/
extern int DbOfficeStopCreate();
/**
* @brief select file from the database by filetype
*/

extern int DBOfficeSelectByFiletype(DBOfficeManager *handler,char *condition);

//#endif
