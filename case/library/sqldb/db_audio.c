/**
* Database management for the audio.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_audio.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "mnavi_fop.h"
#include "file_list.h"
#include "id3.h"
#include "osapi.h"
#include "audio_midware.h"


/** export symbols for LSDK */
#ifdef WIN32
#define ATTR_VISIBLE
#else
#define ATTR_VISIBLE __attribute__((visibility("default")))
#endif

#define DbAudioCreateDatabase_dbg(fmt, arg...) //printf("DbAudioCreateDatabase_dbg[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

/**
* @brief Open the audio database file.
*
* @param path : the full path of the data base file.
*
* @return return an audio data base handler.
*/
ATTR_VISIBLE
DBAudioManager *DBAudioOpen(char *path)
{
	DBAudioManager *handler;
	int rc;

	if(path == NULL){
		return NULL;
	}

	handler = (DBAudioManager *)malloc(sizeof(DBAudioManager));
	if(handler == NULL){
		return NULL;
	}
	memset(handler,0,sizeof(DBAudioManager));

	/** open the sqlite handler. */
	rc = sqlite3_open(path, &handler->sqlHandler);
	if(rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(handler->sqlHandler));
		sqlite3_close(handler->sqlHandler);
		free(handler);
		return NULL;
	}

	return (void *)handler;
}

static void DBAudioFreeAlbumView(DBAudioManager *handler)
{
	DBAudioAlbumInfo *pAlbum,*prevAlbum;

	if(handler == NULL){
		return;
	}

	handler->albumView.totalAlbums = 0;

	pAlbum = handler->albumView.album;
	while(pAlbum){
		prevAlbum = pAlbum;
		pAlbum = pAlbum->next;
		if(prevAlbum->albumName){
			free(prevAlbum->albumName);
		}
		if(prevAlbum->sortIndex){
			free(prevAlbum->sortIndex);
		}
		free(prevAlbum);
	}

	handler->albumView.album = NULL;

	return;
}

static void DBAudioFreeArtistView(DBAudioManager *handler)
{
	DBAudioArtistInfo *pArtist,*prevArtist;

	if(handler == NULL){
		return;
	}

	handler->artistView.totalArtist = 0;

	pArtist = handler->artistView.artist;
	while(pArtist){
		prevArtist = pArtist;
		pArtist = pArtist->next;
		if(prevArtist->artistName){
			free(prevArtist->artistName);
		}
		if(prevArtist->sortIndex){
			free(prevArtist->sortIndex);
		}
		free(prevArtist);
	}

	handler->artistView.artist = NULL;

	return;
}

static void DBAudioFreeGenreView(DBAudioManager *handler)
{
	DBAudioGenreInfo *pGenre,*prevGenre;

	if(handler == NULL){
		return;
	}

	handler->genreView.totalGenre = 0;

	pGenre = handler->genreView.genre;
	while(pGenre){
		prevGenre = pGenre;
		pGenre = pGenre->next;
		if(prevGenre->genreName){
			free(prevGenre->genreName);
		}
		if(prevGenre->sortIndex){
			free(prevGenre->sortIndex);
		}
		free(prevGenre);
	}

	handler->genreView.genre = NULL;

	return;
}

static void DBAudioFreeSongView(DBAudioManager *handler)
{
	DBAudioSongInfo *pSong;

	if(handler == NULL){
		return;
	}

	handler->songView.songs.totalSongs = 0;

	pSong = &handler->songView.songs;
	if(pSong->sortIndex){
		free(pSong->sortIndex);
		pSong->sortIndex = NULL;
	}
	
	return;
}

/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
ATTR_VISIBLE
void DBAudioClose(DBAudioManager *handler)
{
	if(handler == NULL){
		return;
	}

	if(handler->sqlHandler){
		sqlite3_close(handler->sqlHandler);
	}

	DBAudioFreeAlbumView(handler);
	DBAudioFreeArtistView(handler);
	DBAudioFreeGenreView(handler);
	DBAudioFreeSongView(handler);

	free(handler);

	return;
}

/**
* @brief Get the information for a specific view such as Album,Artist,Genre etc.
*
* @param handler : the data base handler.
* @param viewType: the specific viewType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
ATTR_VISIBLE
int DBAudioGetViewInfo(DBAudioManager *handler,int viewType)
{
	sqlite3_stmt *stmt;
	char *sql;
	int rc;
	const char *name;
	int i;
	DBAudioAlbumInfo *oneAlbum,*pAlbum;
	DBAudioArtistInfo *oneArtist,*pArtis;
	DBAudioGenreInfo *oneGenre,*pGenre;

	if(handler == NULL){
		return 1;
	}

	/********************************************************************/
	/*                     Album View Process                           */
	/********************************************************************/
	if(viewType == DBAUDIO_VIEW_ALBUM){
		/** free the old album view */
		DBAudioFreeAlbumView(handler);
		pAlbum = handler->albumView.album;

		/** sql for getting ALBUM number */
		sql = "SELECT album FROM audio_db GROUP BY album";

		rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
		if(rc != SQLITE_OK){
			fprintf(stderr, "SQLite prepare error %d\n", rc);
			return rc;
		}

		rc = sqlite3_step(stmt);
		while(rc == SQLITE_ROW) {
			name = sqlite3_column_text(stmt, 0);
			if(name != NULL){
				oneAlbum = (DBAudioAlbumInfo *)malloc(sizeof(DBAudioAlbumInfo));
				if(oneAlbum){
					memset(oneAlbum,0,sizeof(DBAudioAlbumInfo));
					oneAlbum->albumName = strdup(name);
					/** add to album list */
					if(pAlbum == NULL){
						handler->albumView.album = oneAlbum;
						pAlbum = handler->albumView.album;
					}
					else{
						pAlbum->next = oneAlbum;
						pAlbum = pAlbum->next;
					}
					handler->albumView.totalAlbums++;
				}
			} else {
				/* Field is NULL */
			} 
			rc = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		/** get total songs for each album */
		pAlbum = handler->albumView.album;
		for(i=0;i<handler->albumView.totalAlbums;i++){
			sql = (char *)malloc(1024);
			if(sql && pAlbum && pAlbum->albumName){
				snprintf(sql,1024,"SELECT COUNT(album) FROM audio_db WHERE album=\'%s\'",pAlbum->albumName);
			}
			else{
				if(pAlbum){
					pAlbum = pAlbum->next;
				}
				if(sql){
					free(sql);
				}
				continue;
			}

			rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
			if(rc != SQLITE_OK){
				sqlite3_finalize(stmt);
				pAlbum = pAlbum->next;
				free(sql);
				continue;
			}
			
			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW){
				pAlbum->totalSongs = sqlite3_column_int(stmt, 0);
				rc = sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			pAlbum = pAlbum->next;
			free(sql);
		}
	}
	
	/********************************************************************/
	/*                     Artist View Process                          */
	/********************************************************************/
	else if(viewType == DBAUDIO_VIEW_ARTIST){
		/** free the old artist view */
		DBAudioFreeArtistView(handler);
		pArtis = handler->artistView.artist;

		/** sql for getting artist number */
		sql = "SELECT artist FROM audio_db GROUP BY artist";
		

		rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
		if(rc != SQLITE_OK){
			fprintf(stderr, "SQLite prepare error %d\n", rc);
			return rc;
		}

		rc = sqlite3_step(stmt);
		while(rc == SQLITE_ROW) {
			name = sqlite3_column_text(stmt, 0);
			if(name != NULL){
				oneArtist = (DBAudioArtistInfo *)malloc(sizeof(DBAudioArtistInfo));
				if(oneArtist){
					memset(oneArtist,0,sizeof(DBAudioArtistInfo));
					oneArtist->artistName = strdup(name);
					/** add to artist list */
					if(pArtis == NULL){
						handler->artistView.artist = oneArtist;
						pArtis = handler->artistView.artist;
					}
					else{
						pArtis->next = oneArtist;
						pArtis = pArtis->next;
					}
					handler->artistView.totalArtist++;
				}
			} else {
				/* Field is NULL */
			} 
			rc = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		/** get total songs for each artist */
		pArtis = handler->artistView.artist;
		for(i=0;i<handler->artistView.totalArtist;i++){
			sql = (char *)malloc(1024);
			if(sql && pArtis && pArtis->artistName){
				snprintf(sql,1024,"SELECT COUNT(artist) FROM audio_db WHERE artist=\'%s\';",pArtis->artistName);
			}
			else{
				if(pArtis){
					pArtis = pArtis->next;
				}
				if(sql){
					free(sql);
				}
				continue;
			}

			rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
			if(rc != SQLITE_OK){
				sqlite3_finalize(stmt);
				pArtis = pArtis->next;
				free(sql);
				continue;
			}

			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW){
				pArtis->totalSongs = sqlite3_column_int(stmt, 0);
				rc = sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			pArtis = pArtis->next;
			free(sql);
		}

		/** get total albums for each artist */
		pArtis = handler->artistView.artist;
		for(i=0;i<handler->artistView.totalArtist;i++){
			sql = (char *)malloc(1024);
			if(sql && pArtis && pArtis->artistName){
				snprintf(sql,1024,"SELECT COUNT(myalbum) FROM (SELECT album AS myalbum FROM audio_db WHERE artist=\'%s\' GROUP BY album);",pArtis->artistName);
			}
			else{
				if(pArtis){
					pArtis = pArtis->next;
				}
				if(sql){
					free(sql);
				}
				continue;
			}

			rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
			if(rc != SQLITE_OK){
				sqlite3_finalize(stmt);
				pArtis = pArtis->next;
				free(sql);
				continue;
			}

			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW){
				pArtis->totalAlbums = sqlite3_column_int(stmt, 0);
				rc = sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			pArtis = pArtis->next;
			free(sql);
		}
	}

	/********************************************************************/
	/*                     Genre View Process                           */
	/********************************************************************/
	else if(viewType == DBAUDIO_VIEW_GENRE){
		/** free the old genre view */
		DBAudioFreeGenreView(handler);
		pGenre = handler->genreView.genre;

		/** sql for getting genre number */
		sql = "SELECT genre FROM audio_db GROUP BY genre";

		rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
		if(rc != SQLITE_OK){
			fprintf(stderr, "SQLite prepare error %d\n", rc);
			return rc;
		}

		rc = sqlite3_step(stmt);
		while(rc == SQLITE_ROW) {
			name = sqlite3_column_text(stmt, 0);
			if(name != NULL){
				oneGenre = (DBAudioGenreInfo *)malloc(sizeof(DBAudioGenreInfo));
				if(oneGenre){
					memset(oneGenre,0,sizeof(DBAudioGenreInfo));
					oneGenre->genreName = strdup(name);
					/** add to genre list */
					if(pGenre == NULL){
						handler->genreView.genre = oneGenre;
						pGenre = handler->genreView.genre;
					}
					else{
						pGenre->next = oneGenre;
						pGenre = pGenre->next;
					}
					handler->genreView.totalGenre++;
				}
			} else {
				/* Field is NULL */
			} 
			rc = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		/** get total songs for each genre */
		pGenre = handler->genreView.genre;
		for(i=0;i<handler->genreView.totalGenre;i++){
			sql = (char *)malloc(1024);
			if(sql && pGenre && pGenre->genreName){
				snprintf(sql,1024,"SELECT COUNT(genre) FROM audio_db WHERE genre=\'%s\';",pGenre->genreName);
			}
			else{
				if(pGenre){
					pGenre = pGenre->next;
				}
				if(sql){
					free(sql);
				}
				continue;
			}

			rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
			if(rc != SQLITE_OK){
				sqlite3_finalize(stmt);
				pGenre = pGenre->next;
				free(sql);
				continue;
			}

			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW){
				pGenre->totalSongs = sqlite3_column_int(stmt, 0);
				rc = sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			pGenre = pGenre->next;
			free(sql);
		}

		/** get total albums for each genre */
		pGenre = handler->genreView.genre;
		for(i=0;i<handler->genreView.totalGenre;i++){
			sql = (char *)malloc(1024);
			if(sql && pGenre && pGenre->genreName){
				snprintf(sql,1024,"SELECT COUNT(myalbum) FROM (SELECT album AS myalbum FROM audio_db WHERE genre=\'%s\' GROUP BY album);",pGenre->genreName);
			}
			else{
				if(pGenre){
					pGenre = pGenre->next;
				}
				if(sql){
					free(sql);
				}
				continue;
			}

			rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
			if(rc != SQLITE_OK){
				sqlite3_finalize(stmt);
				pGenre = pGenre->next;
				free(sql);
				continue;
			}

			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW){
				pGenre->totalAlbums = sqlite3_column_int(stmt, 0);
				rc = sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			pGenre = pGenre->next;
			free(sql);
		}
	}

	/********************************************************************/
	/*                     Songs View Process                           */
	/********************************************************************/
	else if(viewType == DBAUDIO_VIEW_SONG){
		/** free the old album view */
		DBAudioFreeSongView(handler);

		/** sql for getting song number */
		sql = "SELECT COUNT(id) FROM audio_db";

		rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
		if(rc != SQLITE_OK){
			fprintf(stderr, "SQLite prepare error %d\n", rc);
			return rc;
		}

		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW) {
			handler->songView.songs.totalSongs = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);
	}
	else{
		fprintf(stderr, "View type(%d) error\n", viewType);
		return 1;
	}

	return 0;
}


/**
* @brief Sort the album view.
*
* @param handler : the data base handler.
* @param album : the album name.
* @param sortType: the specific sortType. See DBAudioViewType.
*
* @return 0 for success; others for failed.
*/
ATTR_VISIBLE
int DBAudioDoAlbumViewSort(DBAudioManager *handler,char *album,int sortType)
{
	sqlite3_stmt *stmt;
	DBAudioAlbumInfo *pAlbum;
	int i;
	char sql[1024];
	int rc;

	if(handler == NULL || album == NULL){
		return 1;
	}

	pAlbum = handler->albumView.album;
	if(pAlbum == NULL){
		return 1;
	}

	/** find the album */
	while(pAlbum){
		if(strcmp(pAlbum->albumName,album)==0){
			break;
		}
		pAlbum = pAlbum->next;
	}

	if(pAlbum == NULL){
		return 1;
	}

	if(pAlbum->totalSongs<=0){
		return 1;
	}

	if(pAlbum->sortIndex){
		free(pAlbum->sortIndex);
		pAlbum->sortIndex = NULL;
	}
	pAlbum->sortIndex = (int *)malloc(pAlbum->totalSongs * sizeof(int));
	if(pAlbum->sortIndex == NULL){
		return 1;
	}
	memset(pAlbum->sortIndex,0,pAlbum->totalSongs * sizeof(int));
	memset(sql,0,1024);
	switch(sortType){
		case DBAUDIO_SORT_BY_NAME:
			snprintf(sql,1024,"SELECT id,name FROM audio_db WHERE album=\'%s\' ORDER BY name;",album);
			break;

		case DBAUDIO_SORT_BY_FILESIZE:
			snprintf(sql,1024,"SELECT id,file_size FROM audio_db WHERE album=\'%s\' ORDER BY file_size;",album);
			break;

		case DBAUDIO_SORT_BY_FILEEXTENSION:
			snprintf(sql,1024,"SELECT id,file_extension FROM audio_db WHERE album=\'%s\' ORDER BY file_extension;",album);
			break;

		case DBAUDIO_SORT_BY_GENRE:
			snprintf(sql,1024,"SELECT id,genre FROM audio_db WHERE album=\'%s\' ORDER BY genre;",album);
			break;

		case DBAUDIO_SORT_BY_YEAR:
			snprintf(sql,1024,"SELECT id,year_generation FROM audio_db WHERE album=\'%s\' ORDER BY year_generation;",album);
			break;

		case DBAUDIO_SORT_BY_ARTIST:
			snprintf(sql,1024,"SELECT id,artist FROM audio_db WHERE album=\'%s\' ORDER BY artist;",album);
			break;

		case DBAUDIO_SORT_BY_COMPOSER:
			snprintf(sql,1024,"SELECT id,composer FROM audio_db WHERE album=\'%s\' ORDER BY composer;",album);
			break;

		case DBAUDIO_SORT_BY_SINGER:
			snprintf(sql,1024,"SELECT id,singer FROM audio_db WHERE album=\'%s\' ORDER BY singer;",album);
			break;

		case DBAUDIO_SORT_BY_TRACK_NUMBER:
			snprintf(sql,1024,"SELECT id,track_nr FROM audio_db WHERE album=\'%s\' ORDER BY track_nr;",album);
			break;

		case DBAUDIO_SORT_BY_TOTAL_TIME:
			snprintf(sql,1024,"SELECT id,total_time FROM audio_db WHERE album=\'%s\' ORDER BY total_time;",album);
			break;

		default:
			goto ERROR1;
			break;
	}

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		goto ERROR1;
	}

	i=0;
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW){
		*(pAlbum->sortIndex+i) = sqlite3_column_int(stmt, 0);
		if(i>=pAlbum->totalSongs){
			fprintf(stderr,"Total songs error when sort album\n");
			break;
		}
		i++;
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	return 0;

ERROR1:
	if(pAlbum->sortIndex){
		free(pAlbum->sortIndex);
		pAlbum->sortIndex = NULL;
	}

	return 1;
}



/**
* @brief Sort the artist view.
*
* @param handler : the data base handler.
* @param artist : the artist name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
ATTR_VISIBLE
int DBAudioDoArtistViewSort(DBAudioManager *handler,char *artist,int sortType)
{
	sqlite3_stmt *stmt;
	DBAudioArtistInfo *pInfo;
	int i;
	char sql[1024];
	int rc;

	if(handler == NULL || artist == NULL){
		return 1;
	}

	pInfo = handler->artistView.artist;
	if(pInfo == NULL){
		return 1;
	}

	/** find the album */
	while(pInfo){
		if(strcmp(pInfo->artistName,artist)==0){
			break;
		}
		pInfo = pInfo->next;
	}

	if(pInfo == NULL){
		return 1;
	}

	if(pInfo->totalSongs<=0){
		return 1;
	}

	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}
	pInfo->sortIndex = (int *)malloc(pInfo->totalSongs * sizeof(int));
	if(pInfo->sortIndex == NULL){
		return 1;
	}
	memset(pInfo->sortIndex,0,pInfo->totalSongs * sizeof(int));
	memset(sql,0,1024);
	switch(sortType){
		case DBAUDIO_SORT_BY_NAME:
			snprintf(sql,1024,"SELECT id,name FROM audio_db WHERE artist=\'%s\' ORDER BY name;",artist);
			break;

		case DBAUDIO_SORT_BY_FILESIZE:
			snprintf(sql,1024,"SELECT id,file_size FROM audio_db WHERE artist=\'%s\' ORDER BY file_size;",artist);
			break;

		case DBAUDIO_SORT_BY_FILEEXTENSION:
			snprintf(sql,1024,"SELECT id,file_extension FROM audio_db WHERE artist=\'%s\' ORDER BY file_extension;",artist);
			break;

		case DBAUDIO_SORT_BY_ALBUM:
			snprintf(sql,1024,"SELECT id,album FROM audio_db WHERE artist=\'%s\' ORDER BY album;",artist);
			break;

		case DBAUDIO_SORT_BY_GENRE:
			snprintf(sql,1024,"SELECT id,genre FROM audio_db WHERE artist=\'%s\' ORDER BY genre;",artist);
			break;

		case DBAUDIO_SORT_BY_YEAR:
			snprintf(sql,1024,"SELECT id,year_generation FROM audio_db WHERE artist=\'%s\' ORDER BY year_generation;",artist);
			break;

		case DBAUDIO_SORT_BY_COMPOSER:
			snprintf(sql,1024,"SELECT id,composer FROM audio_db WHERE artist=\'%s\' ORDER BY composer;",artist);
			break;

		case DBAUDIO_SORT_BY_SINGER:
			snprintf(sql,1024,"SELECT id,singer FROM audio_db WHERE artist=\'%s\' ORDER BY singer;",artist);
			break;

		case DBAUDIO_SORT_BY_TRACK_NUMBER:
			snprintf(sql,1024,"SELECT id,track_nr FROM audio_db WHERE artist=\'%s\' ORDER BY track_nr;",artist);
			break;

		case DBAUDIO_SORT_BY_TOTAL_TIME:
			snprintf(sql,1024,"SELECT id,total_time FROM audio_db WHERE artist=\'%s\' ORDER BY total_time;",artist);
			break;

		default:
			goto ERROR1;
			break;
	}

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		goto ERROR1;
	}

	i=0;
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW){
		*(pInfo->sortIndex+i) = sqlite3_column_int(stmt, 0);
		if(i>=pInfo->totalSongs){
			fprintf(stderr,"Total songs error when sort artist\n");
			break;
		}
		i++;
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	return 0;

ERROR1:
	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}

	return 1;
}

/**
* @brief Sort the genre view.
*
* @param handler : the data base handler.
* @param genre : the genre name.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
ATTR_VISIBLE
int DBAudioDoGenreViewSort(DBAudioManager *handler,char *genre,int sortType)
{
	sqlite3_stmt *stmt;
	DBAudioGenreInfo *pInfo;
	int i;
	char sql[1024];
	int rc;

	if(handler == NULL || genre == NULL){
		return 1;
	}

	pInfo = handler->genreView.genre;
	if(pInfo == NULL){
		return 1;
	}

	/** find the genre */
	while(pInfo){
		if(strcmp(pInfo->genreName,genre)==0){
			break;
		}
		pInfo = pInfo->next;
	}

	if(pInfo == NULL){
		return 1;
	}

	if(pInfo->totalSongs<=0){
		return 1;
	}

	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}
	pInfo->sortIndex = (int *)malloc(pInfo->totalSongs * sizeof(int));
	if(pInfo->sortIndex == NULL){
		return 1;
	}
	memset(pInfo->sortIndex,0,pInfo->totalSongs * sizeof(int));
	memset(sql,0,1024);

	switch(sortType){
		case DBAUDIO_SORT_BY_NAME:
			snprintf(sql,1024,"SELECT id,name FROM audio_db WHERE genre=\'%s\' ORDER BY name;",genre);
			break;

		case DBAUDIO_SORT_BY_FILESIZE:
			snprintf(sql,1024,"SELECT id,file_size FROM audio_db WHERE genre=\'%s\' ORDER BY file_size;",genre);
			break;

		case DBAUDIO_SORT_BY_FILEEXTENSION:
			snprintf(sql,1024,"SELECT id,file_extension FROM audio_db WHERE genre=\'%s\' ORDER BY file_extension;",genre);
			break;

		case DBAUDIO_SORT_BY_ALBUM:
			snprintf(sql,1024,"SELECT id,album FROM audio_db WHERE genre=\'%s\' ORDER BY album;",genre);
			break;

		case DBAUDIO_SORT_BY_ARTIST:
			snprintf(sql,1024,"SELECT id,artist FROM audio_db WHERE genre=\'%s\' ORDER BY genre;",genre);
			break;

		case DBAUDIO_SORT_BY_YEAR:
			snprintf(sql,1024,"SELECT id,year_generation FROM audio_db WHERE genre=\'%s\' ORDER BY year_generation;",genre);
			break;

		case DBAUDIO_SORT_BY_COMPOSER:
			snprintf(sql,1024,"SELECT id,composer FROM audio_db WHERE genre=\'%s\' ORDER BY composer;",genre);
			break;

		case DBAUDIO_SORT_BY_SINGER:
			snprintf(sql,1024,"SELECT id,singer FROM audio_db WHERE genre=\'%s\' ORDER BY singer;",genre);
			break;

		case DBAUDIO_SORT_BY_TRACK_NUMBER:
			snprintf(sql,1024,"SELECT id,track_nr FROM audio_db WHERE genre=\'%s\' ORDER BY track_nr;",genre);
			break;

		case DBAUDIO_SORT_BY_TOTAL_TIME:
			snprintf(sql,1024,"SELECT id,total_time FROM audio_db WHERE genre=\'%s\' ORDER BY total_time;",genre);
			break;

		default:
			goto ERROR1;
			break;
	}
	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		goto ERROR1;
	}
	i=0;
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW){
		*(pInfo->sortIndex+i) = sqlite3_column_int(stmt, 0);
		if(i>=pInfo->totalSongs){
			fprintf(stderr,"Total songs error when sort genre\n");
			break;
		}
		i++;
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	return 0;

ERROR1:
	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}

	return 1;
}

/**
* @brief Sort the songs view.
*
* @param handler : the data base handler.
* @param sortType: the specific sortType. See DBAudioSortType.
*
* @return 0 for success; others for failed.
*/
ATTR_VISIBLE
int DBAudioDoSongViewSort(DBAudioManager *handler,int sortType)
{
	sqlite3_stmt *stmt;
	DBAudioSongInfo *pInfo;
	int i;
	char sql[1024];
	int rc;

	if(handler == NULL){
		return 1;
	}

	pInfo = &handler->songView.songs;

	if(pInfo->totalSongs<=0){
		return 1;
	}

	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}
	pInfo->sortIndex = (int *)malloc(pInfo->totalSongs * sizeof(int));
	if(pInfo->sortIndex == NULL){
		return 1;
	}
	memset(pInfo->sortIndex,0,pInfo->totalSongs * sizeof(int));
	memset(sql,0,1024);
	switch(sortType){
		case DBAUDIO_SORT_BY_NAME:
			snprintf(sql,1024,"SELECT id,name FROM audio_db ORDER BY name;");
			break;

		case DBAUDIO_SORT_BY_FILESIZE:
			snprintf(sql,1024,"SELECT id,file_size FROM audio_db ORDER BY file_size;");
			break;

		case DBAUDIO_SORT_BY_FILEEXTENSION:
			snprintf(sql,1024,"SELECT id,file_extension FROM audio_db ORDER BY file_extension;");
			break;

		case DBAUDIO_SORT_BY_ALBUM:
			snprintf(sql,1024,"SELECT id,album FROM audio_db ORDER BY album;");
			break;

		case DBAUDIO_SORT_BY_ARTIST:
			snprintf(sql,1024,"SELECT id,artist FROM audio_db ORDER BY artist;");
			break;

		case DBAUDIO_SORT_BY_GENRE:
			snprintf(sql,1024,"SELECT id,genre FROM audio_db ORDER BY genre;");
			break;

		case DBAUDIO_SORT_BY_YEAR:
			snprintf(sql,1024,"SELECT id,year_generation FROM audio_db ORDER BY year_generation;");
			break;

		case DBAUDIO_SORT_BY_COMPOSER:
			snprintf(sql,1024,"SELECT id,composer FROM audio_db ORDER BY composer;");
			break;

		case DBAUDIO_SORT_BY_SINGER:
			snprintf(sql,1024,"SELECT id,singer FROM audio_db ORDER BY singer;");
			break;

		case DBAUDIO_SORT_BY_TRACK_NUMBER:
			snprintf(sql,1024,"SELECT id,track_nr FROM audio_db ORDER BY track_nr;");
			break;

		case DBAUDIO_SORT_BY_TOTAL_TIME:
			snprintf(sql,1024,"SELECT id,total_time FROM audio_db ORDER BY total_time;");
			break;

		default:
			goto ERROR1;
			break;
	}

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		goto ERROR1;
	}

	i=0;
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW){
		*(pInfo->sortIndex+i) = sqlite3_column_int(stmt, 0);
		if(i>=pInfo->totalSongs){
			fprintf(stderr,"Total songs error when sort artist\n");
			break;
		}
		i++;
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	return 0;

ERROR1:
	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}

	return 1;
}


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
ATTR_VISIBLE
char * DBAudioGetSortedAlbumViewItem(DBAudioManager *handler,char *album,int index)
{
	char *result=NULL;
	DBAudioAlbumInfo *pInfo;
	int id;
	char sql[1024];
	int rc;
	sqlite3_stmt *stmt;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *fileExt;
	char *albumName;
	char *genreName;
	char *yearGeneration;
	char *artistName;
	char *composerName;
	char *singerName;
	int trackNr;
	int totalTime;
	int result_init_len = 0;
	if(handler == NULL || album == NULL){
		return NULL;
	}

	pInfo = handler->albumView.album;
	while(pInfo){
		if(strcmp(pInfo->albumName,album)==0){
			break;
		}
		pInfo = pInfo->next;
	}
	if(pInfo == NULL){
		fprintf(stderr,"Album %s not exist\n",album);
		return NULL;
	}

	if(index >= pInfo->totalSongs){
		fprintf(stderr,"Index %d range error\n",index);
		return NULL;
	}

	if(pInfo->sortIndex == NULL){
		return NULL;
	}
	
	id = *(pInfo->sortIndex + index);
	memset(sql,0,1024);
	snprintf(sql,1024,"SELECT * FROM audio_db WHERE id=%d;",id);

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		return NULL;
	}

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		dir = sqlite3_column_text(stmt, 1);
		name = sqlite3_column_text(stmt, 2);
		fileSize = sqlite3_column_int(stmt,3);
		modifyTime = sqlite3_column_text(stmt, 4);
		fileExt = sqlite3_column_text(stmt, 5);
		albumName = sqlite3_column_text(stmt, 6);
		genreName = sqlite3_column_text(stmt, 7);
		yearGeneration = sqlite3_column_text(stmt, 8);
		artistName = sqlite3_column_text(stmt, 9);
		composerName = sqlite3_column_text(stmt, 10);
		singerName = sqlite3_column_text(stmt, 11);
		trackNr = sqlite3_column_int(stmt,12);
		totalTime = sqlite3_column_int(stmt,13);
		result_init_len = 256 + strlen(dir) + strlen(name) + strlen(modifyTime) +\
			strlen(fileExt) + strlen(albumName) + strlen(genreName) + strlen(yearGeneration) + \
			strlen(artistName) + strlen(composerName) + strlen(singerName);
		result = (char *)malloc(result_init_len);

		if(result){
			memset(result,0,result_init_len);
			snprintf(result,result_init_len,"<FullPath>=%s%s;<FileSize>=%d;<FileModifyTime>=%s;<FileExtension>=%s;<Album>=%s;<Genre>=%s;<Year>=%s;<Artist>=%s;<Composer>=%s;<Singer>=%s;<TrackNumber>=%d;<TotalTime>=%d;",\
				dir,name,fileSize,modifyTime,fileExt,albumName,genreName,yearGeneration,artistName,composerName,singerName,trackNr,totalTime);
		}
	}

	sqlite3_finalize(stmt);

	return result;
}

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
ATTR_VISIBLE
char * DBAudioGetSortedArtistViewItem(DBAudioManager *handler,char *artist,int index)
{
	char *result=NULL;
	DBAudioArtistInfo *pInfo;
	int id;
	char sql[1024];
	int rc;
	sqlite3_stmt *stmt;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *fileExt;
	char *albumName;
	char *genreName;
	char *yearGeneration;
	char *artistName;
	char *composerName;
	char *singerName;
	int trackNr;
	int totalTime;
	int result_init_len = 0;

	if(handler == NULL || artist == NULL){
		return NULL;
	}

	pInfo = handler->artistView.artist;
	while(pInfo){
		if(strcmp(pInfo->artistName,artist)==0){
			break;
		}
		pInfo = pInfo->next;
	}
	if(pInfo == NULL){
		fprintf(stderr,"Artist %s not exist\n",artist);
		return NULL;
	}

	if(index >= pInfo->totalSongs){
		fprintf(stderr,"Index %d range error\n",index);
		return NULL;
	}

	if(pInfo->sortIndex == NULL){
		return NULL;
	}

	id = *(pInfo->sortIndex + index);
	memset(sql,0,1024);
	snprintf(sql,1024,"SELECT * FROM audio_db WHERE id=%d;",id);

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		return NULL;
	}

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		dir = sqlite3_column_text(stmt, 1);
		name = sqlite3_column_text(stmt, 2);
		fileSize = sqlite3_column_int(stmt,3);
		modifyTime = sqlite3_column_text(stmt, 4);
		fileExt = sqlite3_column_text(stmt, 5);
		albumName = sqlite3_column_text(stmt, 6);
		genreName = sqlite3_column_text(stmt, 7);
		yearGeneration = sqlite3_column_text(stmt, 8);
		artistName = sqlite3_column_text(stmt, 9);
		composerName = sqlite3_column_text(stmt, 10);
		singerName = sqlite3_column_text(stmt, 11);
		trackNr = sqlite3_column_int(stmt,12);
		totalTime = sqlite3_column_int(stmt,13);
		result_init_len = 256 + strlen(dir) + strlen(name) + strlen(modifyTime) +\
			strlen(fileExt) + strlen(albumName) + strlen(genreName) + strlen(yearGeneration) + \
			strlen(artistName) + strlen(composerName) + strlen(singerName);
		result = (char *)malloc(result_init_len);

		if(result){
			memset(result,0,result_init_len);
			snprintf(result,result_init_len,"<FullPath>=%s%s;<FileSize>=%d;<FileModifyTime>=%s;<FileExtension>=%s;<Album>=%s;<Genre>=%s;<Year>=%s;<Artist>=%s;<Composer>=%s;<Singer>=%s;<TrackNumber>=%d;<TotalTime>=%d;",\
				dir,name,fileSize,modifyTime,fileExt,albumName,genreName,yearGeneration,artistName,composerName,singerName,trackNr,totalTime);
		}
	}

	sqlite3_finalize(stmt);

	return result;
}

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
ATTR_VISIBLE
char * DBAudioGetSortedGenreViewItem(DBAudioManager *handler,char *genre,int index)
{
	char *result=NULL;
	DBAudioGenreInfo *pInfo;
	int id;
	char sql[1024];
	int rc;
	sqlite3_stmt *stmt;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *fileExt;
	char *albumName;
	char *genreName;
	char *yearGeneration;
	char *artistName;
	char *composerName;
	char *singerName;
	int trackNr;
	int totalTime;
	int result_init_len = 0;

	if(handler == NULL || genre == NULL){
		return NULL;
	}

	pInfo = handler->genreView.genre;
	while(pInfo){
		if(strcmp(pInfo->genreName,genre)==0){
			break;
		}
		pInfo = pInfo->next;
	}
	if(pInfo == NULL){
		fprintf(stderr,"Genre %s not exist\n",genre);
		return NULL;
	}

	if(index >= pInfo->totalSongs){
		fprintf(stderr,"Index %d range error\n",index);
		return NULL;
	}

	if(pInfo->sortIndex == NULL){
		return NULL;
	}

	id = *(pInfo->sortIndex + index);
	memset(sql,0,1024);
	snprintf(sql,1024,"SELECT * FROM audio_db WHERE id=%d;",id);

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		return NULL;
	}

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		dir = sqlite3_column_text(stmt, 1);
		name = sqlite3_column_text(stmt, 2);
		fileSize = sqlite3_column_int(stmt,3);
		modifyTime = sqlite3_column_text(stmt, 4);
		fileExt = sqlite3_column_text(stmt, 5);
		albumName = sqlite3_column_text(stmt, 6);
		genreName = sqlite3_column_text(stmt, 7);
		yearGeneration = sqlite3_column_text(stmt, 8);
		artistName = sqlite3_column_text(stmt, 9);
		composerName = sqlite3_column_text(stmt, 10);
		singerName = sqlite3_column_text(stmt, 11);
		trackNr = sqlite3_column_int(stmt,12);
		totalTime = sqlite3_column_int(stmt,13);
		result_init_len = 256 + strlen(dir) + strlen(name) + strlen(modifyTime) +\
			strlen(fileExt) + strlen(albumName) + strlen(genreName) + strlen(yearGeneration) + \
			strlen(artistName) + strlen(composerName) + strlen(singerName);
		result = (char *)malloc(result_init_len);

		if(result){
			memset(result,0,result_init_len);
			snprintf(result,result_init_len,"<FullPath>=%s%s;<FileSize>=%d;<FileModifyTime>=%s;<FileExtension>=%s;<Album>=%s;<Genre>=%s;<Year>=%s;<Artist>=%s;<Composer>=%s;<Singer>=%s;<TrackNumber>=%d;<TotalTime>=%d;",\
				dir,name,fileSize,modifyTime,fileExt,albumName,genreName,yearGeneration,artistName,composerName,singerName,trackNr,totalTime);
		}
	}

	sqlite3_finalize(stmt);

	return result;
}

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
ATTR_VISIBLE
char * DBAudioGetSortedSongViewItem(DBAudioManager *handler,int index)
{
	char *result=NULL;
	DBAudioSongInfo *pInfo;
	int id;
	char sql[1024];
	int rc;
	sqlite3_stmt *stmt;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *fileExt;
	char *albumName;
	char *genreName;
	char *yearGeneration;
	char *artistName;
	char *composerName;
	char *singerName;
	int trackNr;
	int totalTime;
	int result_init_len = 0;
	if(handler == NULL){
		return NULL;
	}

	pInfo = &handler->songView.songs;
	if(index >= pInfo->totalSongs){
		fprintf(stderr,"Index %d range error\n",index);
		return NULL;
	}

	if(pInfo->sortIndex == NULL){
		return NULL;
	}

	id = *(pInfo->sortIndex + index);
	memset(sql,0,1024);
	snprintf(sql,1024,"SELECT * FROM audio_db WHERE id=%d;",id);

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(stmt);
		return NULL;
	}

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		dir = sqlite3_column_text(stmt, 1);
		name = sqlite3_column_text(stmt, 2);
		fileSize = sqlite3_column_int(stmt,3);
		modifyTime = sqlite3_column_text(stmt, 4);
		fileExt = sqlite3_column_text(stmt, 5);
		albumName = sqlite3_column_text(stmt, 6);
		genreName = sqlite3_column_text(stmt, 7);
		yearGeneration = sqlite3_column_text(stmt, 8);
		artistName = sqlite3_column_text(stmt, 9);
		composerName = sqlite3_column_text(stmt, 10);
		singerName = sqlite3_column_text(stmt, 11);
		trackNr = sqlite3_column_int(stmt,12);
		totalTime = sqlite3_column_int(stmt,13);
		
		result_init_len = 256 + strlen(dir) + strlen(name) + strlen(modifyTime) +\
			strlen(fileExt) + strlen(albumName) + strlen(genreName) + strlen(yearGeneration) + \
			strlen(artistName) + strlen(composerName) + strlen(singerName);
			
		result = (char *)malloc(result_init_len);
		
		if(result){
			memset(result,0,result_init_len);
			snprintf(result,result_init_len,"<FullPath>=%s%s;<FileSize>=%d;<FileModifyTime>=%s;<FileExtension>=%s;<Album>=%s;<Genre>=%s;<Year>=%s;<Artist>=%s;<Composer>=%s;<Singer>=%s;<TrackNumber>=%d;<TotalTime>=%d;",\
				dir,name,fileSize,modifyTime,fileExt,albumName,genreName,yearGeneration,artistName,composerName,singerName,trackNr,totalTime);
		}
	}

	sqlite3_finalize(stmt);

	return result;
}


typedef struct _AudioDbTable AudioDbTable;

struct _AudioDbTable
{
	int id;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *ext;
	char *album;
	char *genre;
	char *year;
	char *artist;
	char *composer;
	char *singer;
	int trackNumber;
	int totalTime;
};

static void __DbAudioSetUtf16(char * str, int * index, unsigned short u16c)
{
	int i = *index;
	
	if(u16c >= 0x800)
	{
		// 3 byte
		*index = i + 3;
		str[i] = (u16c >> 12) | 0xe0;
		str[i+1] = ((u16c >> 6) & 0x3f) | 0x80;
		str[i+2] = (u16c & 0x3f) | 0x80;
	}
	else if(u16c >= 0x80)
	{
		// 2 byte
		*index = i + 2;
		str[i] = ((u16c >> 6) & 0x1f) | 0xc0;
		str[i+1] = (u16c & 0x3f) | 0x80;
	}
	else
	{
		// 1 byte
		*index = i + 1;
		str[i] = (unsigned char)u16c;
	}
}

/**
* @note 1. keep u8str long enough.
*       2. memset u8str to zero.
*/
static int __DbAudioUtf16ToUtf8(unsigned short * u16str, char * u8str,int len)
{
	int i, j;
	int u16len = len/2;
	
	i = 0, j = 0;
	
	do {
		__DbAudioSetUtf16(u8str, &i, u16str[j]);
		if(u16str[j]==0){
			break;
		}
		j++;
	} while (j<u16len);
	
	return i;
}

static void __DbAudioFreeTable(AudioDbTable *table)
{
	if(table == NULL){
		return;
	}

	if(table->dir){
		free(table->dir);
	}

	if(table->name){
		free(table->name);
	}

	if(table->modifyTime){
		free(table->modifyTime);
	}

	if(table->ext){
		free(table->ext);
	}

	if(table->album){
		free(table->album);
	}

	if(table->genre){
		free(table->genre);
	}

	if(table->year){
		free(table->year);
	}

	if(table->artist){
		free(table->artist);
	}

	if(table->composer){
		free(table->composer);
	}

	if(table->singer){
		free(table->singer);
	}
}

static void *_audio_fopen(char *path, char *mode)
{
	return (void*)fopen(path, mode);
}

static int _audio_fclose(void *fp)
{
	return fclose((FILE*)fp);
}

static long  _audio_read_packet(void *opaque,void *buf,int buf_size)
{
	return (int)fread(buf, sizeof(unsigned char), buf_size, (FILE*)opaque);
}

static int _audio_write_packet(void *buf, int size, int buf_size,void *opaque)
{
	return (int)fwrite(buf, size, buf_size, (FILE*)opaque);
}

static long _audio_file_seek_set(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_SET);
}

static long _audio_file_seek_cur(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_CUR);
}

static long _audio_file_seek_end(void *opaque, long offset)
{
	return fseek((FILE*)opaque, offset, SEEK_END);
}

static long _audio_get_pos(void *opaque)
{
	return ftell((FILE*)opaque);
}

static int __AudioDbSetMusicPara(file_iocontext_s *p_fio,char*filename)
{
	p_fio->file_name = strdup(filename);
	p_fio->handle = _audio_fopen(filename,"rb");
	p_fio->read = (void*)_audio_read_packet;
	p_fio->seek_cur = _audio_file_seek_cur;
	p_fio->seek_end = _audio_file_seek_end;
	p_fio->seek_set = _audio_file_seek_set;
	p_fio->tell = _audio_get_pos;
	p_fio->get_file_length = NULL;
	
	return 0;
}

static int __AudioDbReleaseMusicPara(file_iocontext_s *p_fio)
{
	if(p_fio->file_name){
		free(p_fio->file_name);
		p_fio->file_name = NULL;
	}

	if(p_fio->handle){
		_audio_fclose(p_fio->handle);
		p_fio->handle = NULL;
	}

	return 0;
}


static void* dbAudioSetFileSem;
static int musicSetFileResult = 0;
static void db_audio_get_player_msg(audio_notify_msg_e msg)
{	
	switch(msg){
		case AUDIO_NO_SUPPORT_MSG:
			printf("db audio file is not support\n");
			musicSetFileResult = -1;
			OSSemPost(dbAudioSetFileSem);
			break;
		case AUDIO_FILE_INFO_READY_MSG:
			musicSetFileResult = 0;
			OSSemPost(dbAudioSetFileSem);
			break;
		default:
			break;
	}
}

static int createProcess = 0;
int stopFlag=0;

ATTR_VISIBLE
int DbAudioCreateDatabase(char *filesDir,char *dbFullPath)
{
	UI_FILELIST filelist;
	int total;
	int i,j;
	char *file;
	AudioDbTable table;
	char *ptr,*ptr2,*ptr3;
	int len;
	struct stat fileInfo;
	struct tm mtime;
	MUSIC_INFO id3Info;
	void* fp;
	char *tmp;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int rc;
	char *sql;
	int ret = -1;

	void *musicHandle=NULL;
	file_iocontext_s file_iocontext;
	int musicErr;
	music_file_info_t media_info;
	a_set_file_s aSetfile;

	

	createProcess = 0;
	stopFlag = 0;

	rc = sqlite3_open(dbFullPath, &db);
	if(rc) {
		fprintf(stderr, "Create database error: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return ret;
	}

	sql = (char *)malloc(1024);
	if(sql == NULL){
		sqlite3_close(db);
		return ret;
	}

	/** create table */
	memset(sql,0,1024);
	snprintf(sql,1024,"CREATE TABLE audio_db ( id integer primary key,directory text,name text,file_size integer,modify_time text,file_extension text,album text,genre text,year_generation text,artist text,composer text,singer text,track_nr integer,total_time integer );");
	rc = sqlite3_exec(db, sql,NULL, NULL,NULL);
	if(rc != SQLITE_OK){
		fprintf(stderr, "SQLite exec error %d\n", rc);
		sqlite3_close(db);
		free(sql);
		return ret;
	}

	/** prepare for insert items to the table */
	
	memset(sql,0,1024);
	snprintf(sql,1024,"INSERT INTO 'audio_db' VALUES(:id_a,:dir_a,:name_a,:file_size_a,:mtime_a,:ext_a,:album_a,:genre_a,:year_a,:artist_a,:composer_a,:singer_a,:track_a,:total_a);");
	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		fprintf(stderr, "SQLite prepare error %d\n", rc);
		sqlite3_close(db);
		free(sql);
		remove(dbFullPath);
		return ret;
	}

	/** for get total time */
	musicHandle = audioDecOpen(NULL);
	if(musicHandle){
		dbAudioSetFileSem = OSSemCreate(0);
		if(NULL==dbAudioSetFileSem){
			printf("create dbAudioSetFileSem error!\n");
		}
		audio_play_cmd(musicHandle,SET_NOTIFY_FUNC,(unsigned int)(&db_audio_get_player_msg));
	}

	filelist.fsel = NULL;
	ui_filelist_init(&filelist, filesDir, "mp3 wma ogg wav aac flac cook ape ac3 dts ra rm amr", 0, FLIST_MODE_BROWSE_FILE);
	total = ui_filelist_get_filetotal_num(&filelist);

	if(total < 1){
		fprintf(stderr,"No Audio Filse when creating dababase\n");
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		free(sql);
		remove(dbFullPath);
		createProcess=100;
		return ret;
	}

	ret = 0;
	if(stopFlag){
		goto _CREATE_OUT;
	}

	for(i=0;i<total;i++){

		/** update the creating process */
		createProcess = (i*100)/total;
		fprintf(stderr,"fucntion:%s,line:%d,createProcess======%d\n",__FUNCTION__,__LINE__,createProcess);
		if(stopFlag){
			break;
		}
		
		file = ui_filelist_get_cur_filepath(&filelist,i);
		
		fprintf(stderr,"fucntion:%s,line:%d,file======%s\n",__FUNCTION__,__LINE__,file);
		if(file){
			/** id */
			table.id = i;

			/** directory and name */
			ptr = file;
			ptr2 = ptr + strlen(ptr)-1;
			while((ptr2!=ptr) && (*ptr2 != '/')) ptr2--;
			ptr2++;
			len = ptr2 -ptr + 1;
			table.dir = (char *)malloc(len);
			if(table.dir){
				memset(table.dir,0,len);
				ptr3 = table.dir;
				j=0;
				while(j<len-1){
					*(ptr3+j) = *(ptr+j);
					j++;
				}
				
			}
			table.name = strdup(ptr2);

			/** file extension */
			ptr2 = ptr + strlen(ptr)-1;
			while((ptr2!=ptr) && (*ptr2 != '.')) ptr2--;
			ptr2++;
			table.ext = strdup(ptr2);

			/** FS information */
			if(stat(file, &fileInfo)==0){
				table.fileSize = fileInfo.st_size;
				if(gmtime_r(&fileInfo.st_mtime,&mtime)){
					table.modifyTime = (char *)malloc(32);
					if(table.modifyTime){
						sprintf(table.modifyTime,"%d-%02d-%02d %02d:%02d",mtime.tm_year+1900,mtime.tm_mon+1,\
							mtime.tm_mday,mtime.tm_hour,mtime.tm_min);
					}
					else{
						table.modifyTime = strdup("1900-01-01 00:00");
					}
				}
				else{
					table.modifyTime = strdup("1900-01-01 00:00");
				}
			}
			else{
				table.fileSize = 0;
				table.modifyTime = strdup("1900-01-01 00:00");
			}
			
			//fprintf(stderr,"file:%s,dir=%s,name=%s,ext=%s,time=%s,size=%d\n",file,table.dir,table.name,table.ext,table.modifyTime,table.fileSize);
			fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);


			/** ID3 information */
			fp=OSfopen(file,"rb");
			
			fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
			if(fp!=NULL)
				get_audio_info(fp,&id3Info,0xfff);
			
			fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
			if(id3Info.album.length){
				//printf("album-length=%d-->",id3Info.album.length);
				table.album = (char *)malloc(id3Info.album.length*2);
				//fprintf(stderr,"function:%s,line:%d,table.album==%d\n",__FUNCTION__,__LINE__,table.album);
				if(table.album){
					memset(table.album,0,id3Info.album.length*2);
					//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.album.content,table.album,id3Info.album.length);
					
					//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
				}
				else{
					table.album = strdup("unknown");
					
					//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
				}
				//printf("album = %s\n",table.album);
			}
			else{
				table.album = strdup("unknown");
			}

			if(id3Info.genre.length){
				printf("genre-length=%d-->",id3Info.genre.length);
				table.genre = (char *)malloc(id3Info.genre.length*2);
				if(table.genre){
					memset(table.genre,0,id3Info.genre.length*2);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.genre.content,table.genre,id3Info.genre.length);
				}
				else{
					table.genre = strdup("unknown");
				}
				printf("genre = %s\n",table.genre);
			}
			else{
				table.genre = strdup("unknown");
			}

			if(id3Info.year.length){
				printf("year-length=%d-->",id3Info.year.length);
				table.year = (char *)malloc(id3Info.year.length*2);
				if(table.year){
					memset(table.year,0,id3Info.year.length*2);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.year.content,table.year,id3Info.year.length);
				}
				else{
					table.year = strdup("unknown");
				}
				printf("year = %s\n",table.year);
			}
			else{
				table.year = strdup("unknown");
			}

			if(id3Info.author.length){
				printf("artist-length=%d-->",id3Info.author.length);
				table.artist = (char *)malloc(id3Info.author.length*2);
				if(table.artist){
					memset(table.artist,0,id3Info.author.length*2);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.author.content,table.artist,id3Info.author.length);
				}
				else{
					table.artist = strdup("unknown");
				}
				printf("artist = %s\n",table.artist);
				
			}
			else{
				table.artist = strdup("unknown");
			}

			if(id3Info.composer.length){
				printf("composer-length=%d\n",id3Info.composer.length);
				table.composer = (char *)malloc(id3Info.composer.length*2);
				if(table.composer){
					memset(table.composer,0,id3Info.composer.length*2);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.composer.content,table.composer,id3Info.composer.length);
				}
				else{
					table.composer = strdup("unknown");
				}
				printf("composer = %s\n",table.composer);
				
			}
			else{
				table.composer = strdup("unknown");
			}

			
			table.singer= strdup("unknown");

			if(id3Info.track.length){
				printf("track-length=%d\n",id3Info.track.length);
				tmp = (char *)malloc(id3Info.track.length*2);
				if(tmp){
					memset(tmp,0,id3Info.track.length*2);
					__DbAudioUtf16ToUtf8((unsigned short *)id3Info.track.content,tmp,id3Info.track.length);
					table.trackNumber = atoi(tmp);
					free(tmp);
				}
				else{
					table.trackNumber = 0;
				}
				printf("track = %d\n",table.trackNumber);
				
			}
			else{
				table.trackNumber = 0;
			}

			id3_free(&id3Info);
			OSfclose(fp);

			/** Total time */
			if(musicHandle){
				__AudioDbSetMusicPara(&file_iocontext,file);
				aSetfile.f_io = &file_iocontext;
				aSetfile.pInfo = &media_info;
				musicSetFileResult = -1;
				musicErr = audioDecCmd(musicHandle,(unsigned int)SET_FILE,(unsigned int)&aSetfile);
				if(musicErr == 0 ){
					OSSemPend(dbAudioSetFileSem, 10000);
					if(0 == musicSetFileResult){
						musicErr = audioDecCmd(musicHandle,GET_MEDIA_INFO,(unsigned int)&media_info);
						if(musicErr == 0){
							table.totalTime = media_info.total_time;
						}
						else{
							table.totalTime = 0;
						}
					}else{
						table.totalTime = 0;
					}
				}
				else{
					table.totalTime = 0;
				}
				__AudioDbReleaseMusicPara(&file_iocontext);
			}
			else{
				table.totalTime = 0;
			}
			
			/** insert into table */
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":id_a"),table.id);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":dir_a"),table.dir,(int)strlen(table.dir),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":name_a"),table.name,(int)strlen(table.name),NULL);
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":file_size_a"),table.fileSize);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":mtime_a"),table.modifyTime,(int)strlen(table.modifyTime),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":ext_a"),table.ext,(int)strlen(table.ext),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":album_a"),table.album,(int)strlen(table.album),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":genre_a"),table.genre,(int)strlen(table.genre),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":year_a"),table.year,(int)strlen(table.year),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":artist_a"),table.artist,(int)strlen(table.artist),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":composer_a"),table.composer,(int)strlen(table.composer),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":singer_a"),table.singer,(int)strlen(table.singer),NULL);
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":track_a"),table.trackNumber);
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":total_a"),table.totalTime);
			sqlite3_step(stmt);
			rc = sqlite3_reset(stmt);
			if(rc != SQLITE_OK){
				fprintf(stderr,"SQLite reset error:%s\n",sqlite3_errmsg(db));
				__DbAudioFreeTable(&table);
				ret = -1;
				break;
			}
			/** free table memory */
			__DbAudioFreeTable(&table);
		}
	}

_CREATE_OUT:
	
	if(musicHandle){
		audio_play_cmd(musicHandle,TERMINAL,0);
		audioDecClose(musicHandle);
	}

	if(dbAudioSetFileSem){
		OSSemClose(dbAudioSetFileSem);
		dbAudioSetFileSem = NULL;
	}
	
	ui_filelist_exit(&filelist);
	
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(sql);
	
	createProcess = 100;
	
	return ret;
}


ATTR_VISIBLE
int DbAudioDeleteDatabase(char *path)
{
	if(path == NULL){
		return -1;
	}
	return remove(path);
}

/**
* @brief Get the status for creating a database.
*
* @return Return the percentage for creating, e.g. 50 stands for 50% and 100 for done.
*/
ATTR_VISIBLE
int DbAudioGetCreateStat()
{
	return createProcess;
}

ATTR_VISIBLE
int DbAudioStopCreate()
{
	stopFlag = 1;

	return 0;
}

