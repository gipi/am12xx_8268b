#ifndef _DB_AUDIO_H
#define _DB_AUDIO_H

#include "sqlite3.h"

enum DBAudioViewType{
	DBAUDIO_VIEW_ALBUM=0,
	DBAUDIO_VIEW_ARTIST,
	DBAUDIO_VIEW_GENRE,
	DBAUDIO_VIEW_SONG,
};

enum DBAudioSortType{
	DBAUDIO_SORT_BY_NAME=0,
	DBAUDIO_SORT_BY_FILESIZE,
	DBAUDIO_SORT_BY_FILEEXTENSION,
	DBAUDIO_SORT_BY_ALBUM,
	DBAUDIO_SORT_BY_GENRE,
	DBAUDIO_SORT_BY_YEAR,
	DBAUDIO_SORT_BY_ARTIST,
	DBAUDIO_SORT_BY_COMPOSER,
	DBAUDIO_SORT_BY_SINGER,
	DBAUDIO_SORT_BY_TRACK_NUMBER,
	DBAUDIO_SORT_BY_TOTAL_TIME,
};

/** 
* for album management 
*/
typedef struct _DBAudioAlbumInfo DBAudioAlbumInfo;
struct _DBAudioAlbumInfo
{
	char *albumName;
	int totalSongs;
	int *sortIndex;
	DBAudioAlbumInfo *next;
};

typedef struct _DBAudioAlbumViewInfo DBAudioAlbumViewInfo;
struct _DBAudioAlbumViewInfo
{
	int totalAlbums;
	DBAudioAlbumInfo *album;
};

/** 
* for artist management 
*/
typedef struct _DBAudioArtistInfo DBAudioArtistInfo;
struct _DBAudioArtistInfo
{
	char *artistName;
	int totalAlbums;
	int totalSongs;
	int *sortIndex;
	DBAudioArtistInfo *next;
};

typedef struct _DBAudioArtistViewInfo DBAudioArtistViewInfo;
struct _DBAudioArtistViewInfo
{
	int totalArtist;
	DBAudioArtistInfo *artist;
};

/** 
* for genre management 
*/
typedef struct _DBAudioGenreInfo DBAudioGenreInfo;
struct _DBAudioGenreInfo
{
	char *genreName;
	int totalAlbums;
	int totalSongs;
	int *sortIndex;
	DBAudioGenreInfo *next;
};


typedef struct _DBAudioGenreViewInfo DBAudioGenreViewInfo;
struct _DBAudioGenreViewInfo
{
	int totalGenre;
	DBAudioGenreInfo *genre;
};

/** 
* for songs view management.
*/
typedef struct _DBAudioSongInfo DBAudioSongInfo;
struct _DBAudioSongInfo
{
	int totalSongs;
	int *sortIndex;
};
typedef struct _DBAudioSongViewInfo DBAudioSongViewInfo;
struct _DBAudioSongViewInfo
{
	DBAudioSongInfo songs;
};


typedef struct _DBAudioManager DBAudioManager;
struct _DBAudioManager
{
	/// the SQLite handler.
	sqlite3 *sqlHandler;

	/// the album view information.
	DBAudioAlbumViewInfo albumView;

	/// the artist view information.
	DBAudioArtistViewInfo artistView;

	/// the genre view information.
	DBAudioGenreViewInfo genreView;

	/// the song view information.
	DBAudioSongViewInfo songView;
};

/**
* @brief Open the audio database file.
*
* @param path : the full path of the data base file.
*
* @return return an audio data base handler.
*/
extern DBAudioManager *DBAudioOpen(char *path);


/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
extern void DBAudioClose(DBAudioManager *handler);

/**
* @brief Get the information for a specific view such as Album,Artist,Genre etc.
*
* @param handler : the data base handler.
* @param viewType: the specific viewType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
extern int DBAudioGetViewInfo(DBAudioManager *handler,int viewType);


/**
* @brief Sort the album view.
*
* @param handler : the data base handler.
* @param album : the album name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
extern int DBAudioDoAlbumViewSort(DBAudioManager *handler,char *album,int sortType);

/**
* @brief Sort the artist view.
*
* @param handler : the data base handler.
* @param artist : the artist name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
extern int DBAudioDoArtistViewSort(DBAudioManager *handler,char *artist,int sortType);

/**
* @brief Sort the genre view.
*
* @param handler : the data base handler.
* @param genre : the genre name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
extern int DBAudioDoGenreViewSort(DBAudioManager *handler,char *genre,int sortType);

/**
* @brief Sort the songs view.
*
* @param handler : the data base handler.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
extern int DBAudioDoSongViewSort(DBAudioManager *handler,int sortType);

/**
* Get the item information after the album has been sorted by a specific 
* sort type such as by name,by file extension etc..
*
* @param handler: the db manager handle.
* @param album: the album name.
* @param index: the sorted index.
*
* @return This function will return a string containing the item info.
*     The format of the info will be like the following:
*     "<FullPath>=...;<FileSize>=...;<FileModifyTime>=..;<FileExtension>=...;
*     <Album>=...;<Genre>=...;<Year>=...;<Artist>=...;<Composer>=...;
*     <Singer>=...;<TrackNumber>=...;<TotalTime>=...;"
*
* @note Please free the return memory after using.
*/
extern char * DBAudioGetSortedAlbumViewItem(DBAudioManager *handler,char *album,int index);

/**
* Get the item information after the artist view has been sorted by a specific 
* sort type such as by name,by file extension etc..
*
* @param handler: the db manager handle.
* @param artist: the artist name.
* @param index: the sorted index.
*
* @return This function will return a string containing the item info.
*     The format of the info will be like the following:
*     "<FullPath>=...;<FileSize>=...;<FileModifyTime>=..;<FileExtension>=...;
*     <Album>=...;<Genre>=...;<Year>=...;<Artist>=...;<Composer>=...;
*     <Singer>=...;<TrackNumber>=...;<TotalTime>=...;"
*
* @note Please free the return memory after using.
*/
extern char * DBAudioGetSortedArtistViewItem(DBAudioManager *handler,char *artist,int index);

/**
* Get the item information after the genre view has been sorted by a specific 
* sort type such as by name,by file extension etc..
*
* @param handler: the db manager handle.
* @param genre: the genre name.
* @param index: the sorted index.
*
* @return This function will return a string containing the item info.
*     The format of the info will be like the following:
*     "<FullPath>=...;<FileSize>=...;<FileModifyTime>=..;<FileExtension>=...;
*     <Album>=...;<Genre>=...;<Year>=...;<Artist>=...;<Composer>=...;
*     <Singer>=...;<TrackNumber>=...;<TotalTime>=...;"
*
* @note Please free the return memory after using.
*/
extern char * DBAudioGetSortedGenreViewItem(DBAudioManager *handler,char *genre,int index);

/**
* Get the item information after the song view has been sorted by a specific 
* sort type such as by name,by file extension etc..
*
* @param handler: the db manager handle.
* @param index: the sorted index.
*
* @return This function will return a string containing the item info.
*     The format of the info will be like the following:
*     "<FullPath>=...;<FileSize>=...;<FileModifyTime>=..;<FileExtension>=...;
*     <Album>=...;<Genre>=...;<Year>=...;<Artist>=...;<Composer>=...;
*     <Singer>=...;<TrackNumber>=...;<TotalTime>=...;"
*
* @note Please free the return memory after using.
*/
extern char * DBAudioGetSortedSongViewItem(DBAudioManager *handler,int index);


/**
* @brief Create the audio database.
*
* @param filesDir: the audio file base directory.
* @param dbFullPath: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbAudioCreateDatabase(char *filesDir,char *dbFullPath);

/**
* @brief Delete the audio database.
*
* @param path: the full path of the database file, e.g. /mnt/udisk/Audio.db
*
* @return 0 for success or failed.
*/
extern int DbAudioDeleteDatabase(char *path);

/**
* @brief Get the status for creating a database.
*
* @return Return the percentage for creating, e.g. 50 stands for 50% and 100 for done.
*/
extern int DbAudioGetCreateStat();

/**
* @brief Stop creating the database.
*/
extern int DbAudioStopCreate();

#endif
