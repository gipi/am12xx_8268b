#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "swf_ext.h"
#include "db_audio.h"

#ifdef MODULE_CONFIG_SQLITE

typedef struct _AdDbEngine AdDbEngine;
struct _AdDbEngine
{
	DBAudioManager *dbManager;
	int albumViewInited;
	int artistViewInited;
	int genreViewInited;
	int songViewInited;
};

static AdDbEngine dbEngineManager=
{
	.dbManager = NULL,
};

/**
* @brief Open the audio database engine.
* 
* @return 0 for success , others for fail.
*/
static int audio_db_open(void *handle)
{
	int rtn=0;
	char *dbPath;
	
	SWFEXT_FUNC_BEGIN(handle);

	dbPath = Swfext_GetString();

	if(access(dbPath,F_OK)==-1){
		fprintf(stderr,"The DB %s does not exists\n",dbPath);
		rtn = 1;
		goto AUDIO_DB_OPEN_OUT;
	}

	/** if already open a database, close first */
	if(dbEngineManager.dbManager){
		DBAudioClose(dbEngineManager.dbManager);
		dbEngineManager.dbManager = NULL;
		dbEngineManager.albumViewInited = 0;
		dbEngineManager.artistViewInited = 0;
		dbEngineManager.genreViewInited = 0;
		dbEngineManager.songViewInited = 0;
	}
	
	dbEngineManager.dbManager = DBAudioOpen(dbPath);
	if(dbEngineManager.dbManager == NULL){
		fprintf(stderr,"Open %s error\n",dbPath);
		rtn = 1;
	}
	dbEngineManager.albumViewInited = 0;
	dbEngineManager.artistViewInited = 0;
	dbEngineManager.genreViewInited = 0;
	dbEngineManager.songViewInited = 0;
	
AUDIO_DB_OPEN_OUT:	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int audio_db_close(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	if(dbEngineManager.dbManager){
		DBAudioClose(dbEngineManager.dbManager);
		dbEngineManager.dbManager = NULL;
		dbEngineManager.albumViewInited = 0;
		dbEngineManager.artistViewInited = 0;
		dbEngineManager.genreViewInited = 0;
		dbEngineManager.songViewInited = 0;
	}
	
	SWFEXT_FUNC_END();
}


/************************************************************
* For albums.
************************************************************/
static int audio_db_get_album_number(void *handle)
{
	DBAudioManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = dbEngineManager.dbManager;
	if(dbEngineManager.albumViewInited){
		if(pDbHandle){
			total = pDbHandle->albumView.totalAlbums;
		}
	}
	else{
		if(pDbHandle){
			rc = DBAudioGetViewInfo(pDbHandle,DBAUDIO_VIEW_ALBUM);
			if(rc == 0){
				total = pDbHandle->albumView.totalAlbums;
				dbEngineManager.albumViewInited = 1;
			}
			else{
				fprintf(stderr,"Get album infor error\n");
			}
		}
	}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}

static int audio_db_get_album_info(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioAlbumInfo *pInfo;
	int index;
	char *result;
	int i,resultSize;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;
	
	if(dbEngineManager.albumViewInited){
		if(pDbHandle){
			if(index>=pDbHandle->albumView.totalAlbums || index <0){
				goto GET_ALBUM_INFO_ERROR;
			}
			pInfo = pDbHandle->albumView.album;
			i=0;
			while(i!=index && pInfo){
				i++;
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				goto GET_ALBUM_INFO_ERROR;
			}

			resultSize = 128 + strlen(pInfo->albumName);
			result = (char *)malloc(resultSize);
			if(result == NULL){
				goto GET_ALBUM_INFO_ERROR;
			}
			memset(result,0,resultSize);
			snprintf(result,resultSize,"<AlbumName>=%s;<TotalSongs>=%d;",pInfo->albumName,pInfo->totalSongs);
			Swfext_PutString(result);
			free(result);
			goto GET_ALBUM_INFO_OK;
			
		}
		else{
			goto GET_ALBUM_INFO_ERROR;
		}
	}
	else{
		fprintf(stderr,"Call audio_db_get_album_number() first please!\n");
		goto GET_ALBUM_INFO_ERROR;
	}

GET_ALBUM_INFO_ERROR:
	Swfext_PutString("unknown");
GET_ALBUM_INFO_OK:
	
	SWFEXT_FUNC_END();
}

static int audio_db_sort_album(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioAlbumInfo *pInfo;
	char *album;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	album = Swfext_GetString();
	sortType = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	rc = DBAudioDoAlbumViewSort(pDbHandle,album,sortType);
	if(rc==0){
		pInfo = pDbHandle->albumView.album;
		if(pInfo == NULL){
			Swfext_PutNumber(0);
		}
		else{
			while(pInfo){
				if(strcmp(pInfo->albumName,album)==0){
					break;
				}
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				Swfext_PutNumber(0);
			}
			else{
				Swfext_PutNumber(pInfo->totalSongs);
			}
		}
		
	}
	else{
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();
}


static int audio_db_get_info_from_sorted_album(void *handle)
{
	DBAudioManager *pDbHandle;
	char *info=NULL,*album=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	album = Swfext_GetString();
	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	info = DBAudioGetSortedAlbumViewItem(pDbHandle,album,index);
	if(info){
		Swfext_PutString(info);
		free(info);
	}
	else{
		Swfext_PutString("unknown");
	}

	SWFEXT_FUNC_END();
}


/************************************************************
* For artist.
************************************************************/
static int audio_db_get_artist_number(void *handle)
{
	DBAudioManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = dbEngineManager.dbManager;
	if(dbEngineManager.artistViewInited){
		if(pDbHandle){
			total = pDbHandle->artistView.totalArtist;
		}
	}
	else{
		if(pDbHandle){
			rc = DBAudioGetViewInfo(pDbHandle,DBAUDIO_VIEW_ARTIST);
			if(rc == 0){
				total = pDbHandle->artistView.totalArtist;
				dbEngineManager.artistViewInited = 1;
			}
			else{
				fprintf(stderr,"Get artist infor error\n");
			}
		}
	}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}

static int audio_db_get_artist_info(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioArtistInfo *pInfo;
	int index;
	char *result;
	int i,resultSize;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;
	
	if(dbEngineManager.artistViewInited){
		if(pDbHandle){
			if(index>=pDbHandle->artistView.totalArtist || index <0){
				goto GET_ARTIST_INFO_ERROR;
			}
			pInfo = pDbHandle->artistView.artist;
			i=0;
			while(i!=index && pInfo){
				i++;
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				goto GET_ARTIST_INFO_ERROR;
			}

			resultSize = 128 + strlen(pInfo->artistName);
			result = (char *)malloc(resultSize);
			if(result == NULL){
				goto GET_ARTIST_INFO_ERROR;
			}
			memset(result,0,resultSize);
			snprintf(result,resultSize,"<ArtistName>=%s;<TotalSongs>=%d;<TotalAlbums>=%d;",pInfo->artistName,pInfo->totalSongs,pInfo->totalAlbums);
			Swfext_PutString(result);
			free(result);
			goto GET_ARTIST_INFO_OK;
			
		}
		else{
			goto GET_ARTIST_INFO_ERROR;
		}
	}
	else{
		fprintf(stderr,"Call audio_db_get_artist_number() first please!\n");
		goto GET_ARTIST_INFO_ERROR;
	}

GET_ARTIST_INFO_ERROR:
	Swfext_PutString("unknown");
GET_ARTIST_INFO_OK:
	
	SWFEXT_FUNC_END();
}

static int audio_db_sort_artist(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioArtistInfo *pInfo;
	char *artist;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	artist = Swfext_GetString();
	sortType = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	rc = DBAudioDoArtistViewSort(pDbHandle,artist,sortType);
	if(rc==0){
		pInfo = pDbHandle->artistView.artist;
		if(pInfo == NULL){
			Swfext_PutNumber(0);
		}
		else{
			while(pInfo){
				if(strcmp(pInfo->artistName,artist)==0){
					break;
				}
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				Swfext_PutNumber(0);
			}
			else{
				Swfext_PutNumber(pInfo->totalSongs);
			}
		}
		
	}
	else{
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();
}


static int audio_db_get_info_from_sorted_artist(void *handle)
{
	DBAudioManager *pDbHandle;
	char *info=NULL,*artist=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	artist = Swfext_GetString();
	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	info = DBAudioGetSortedArtistViewItem(pDbHandle,artist,index);
	if(info){
		Swfext_PutString(info);
		free(info);
	}
	else{
		Swfext_PutString("unknown");
	}

	SWFEXT_FUNC_END();
}	

/************************************************************
* For genre.
************************************************************/
static int audio_db_get_genre_number(void *handle)
{
	DBAudioManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = dbEngineManager.dbManager;
	if(dbEngineManager.genreViewInited){
		if(pDbHandle){
			total = pDbHandle->genreView.totalGenre;
		}
	}
	else{
		if(pDbHandle){
			rc = DBAudioGetViewInfo(pDbHandle,DBAUDIO_VIEW_GENRE);
			if(rc == 0){
				total = pDbHandle->genreView.totalGenre;
				dbEngineManager.genreViewInited = 1;
			}
			else{
				fprintf(stderr,"Get genre infor error\n");
			}
		}
	}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}

static int audio_db_get_genre_info(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioGenreInfo *pInfo;
	int index;
	char *result;
	int i,resultSize;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;
	
	if(dbEngineManager.genreViewInited){
		if(pDbHandle){
			if(index>=pDbHandle->genreView.totalGenre || index <0){
				goto GET_GENRE_INFO_ERROR;
			}
			pInfo = pDbHandle->genreView.genre;
			i=0;
			while(i!=index && pInfo){
				i++;
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				goto GET_GENRE_INFO_ERROR;
			}

			resultSize = 128 + strlen(pInfo->genreName);
			result = (char *)malloc(resultSize);
			if(result == NULL){
				goto GET_GENRE_INFO_ERROR;
			}
			memset(result,0,resultSize);
			snprintf(result,resultSize,"<GenreName>=%s;<TotalSongs>=%d;<TotalAlbums>=%d;",pInfo->genreName,pInfo->totalSongs,pInfo->totalAlbums);
			Swfext_PutString(result);
			free(result);
			goto GET_GENRE_INFO_OK;
			
		}
		else{
			goto GET_GENRE_INFO_ERROR;
		}
	}
	else{
		fprintf(stderr,"Call audio_db_get_genre_number() first please!\n");
		goto GET_GENRE_INFO_ERROR;
	}

GET_GENRE_INFO_ERROR:
	Swfext_PutString("unknown");
GET_GENRE_INFO_OK:
	
	SWFEXT_FUNC_END();
}

static int audio_db_sort_genre(void *handle)
{
	DBAudioManager *pDbHandle;
	DBAudioGenreInfo *pInfo;
	char *genre;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	genre = Swfext_GetString();
	sortType = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;
	rc = DBAudioDoGenreViewSort(pDbHandle,genre,sortType);
	if(rc==0){
		pInfo = pDbHandle->genreView.genre;
		if(pInfo == NULL){
			Swfext_PutNumber(0);
		}
		else{
			while(pInfo){
				if(strcmp(pInfo->genreName,genre)==0){
					break;
				}
				pInfo = pInfo->next;
			}
			if(pInfo == NULL){
				Swfext_PutNumber(0);
			}
			else{
				Swfext_PutNumber(pInfo->totalSongs);
			}
		}
		
	}
	else{
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();
}


static int audio_db_get_info_from_sorted_genre(void *handle)
{
	DBAudioManager *pDbHandle;
	char *info=NULL,*genre=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	genre = Swfext_GetString();
	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;
	info = DBAudioGetSortedGenreViewItem(pDbHandle,genre,index);
	if(info){
		Swfext_PutString(info);
		free(info);
	}
	else{
		Swfext_PutString("unknown");
	}

	SWFEXT_FUNC_END();
}	

/************************************************************
* For song.
************************************************************/
static int audio_db_get_song_number(void *handle)
{
	DBAudioManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = dbEngineManager.dbManager;
	if(dbEngineManager.songViewInited){
		if(pDbHandle){
			total = pDbHandle->songView.songs.totalSongs;
		}
	}
	else{
		if(pDbHandle){
			rc = DBAudioGetViewInfo(pDbHandle,DBAUDIO_VIEW_SONG);
			if(rc == 0){
				total = pDbHandle->songView.songs.totalSongs;
				dbEngineManager.songViewInited = 1;
			}
			else{
				fprintf(stderr,"Get genre infor error\n");
			}
		}
	}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}


static int audio_db_sort_song(void *handle)
{
	DBAudioManager *pDbHandle;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	sortType = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	rc = DBAudioDoSongViewSort(pDbHandle,sortType);
	if(rc==0){
		Swfext_PutNumber(pDbHandle->songView.songs.totalSongs);
	}
	else{
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();
}


static int audio_db_get_info_from_sorted_song(void *handle)
{
	DBAudioManager *pDbHandle;
	char *info=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = dbEngineManager.dbManager;

	info = DBAudioGetSortedSongViewItem(pDbHandle,index);
	if(info){
		Swfext_PutString(info);
		free(info);
	}
	else{
		Swfext_PutString("unknown");
	}

	SWFEXT_FUNC_END();
}	

/**
************************************************************************
* For Creating Audio Database.
************************************************************************
*/

struct audio_db_create_arg
{
	char *file_dir;
	char *db_path;
};

pthread_t audio_db_thread;
struct audio_db_create_arg targ;

static void *audio_db_create_loop(void *arg)
{
	struct audio_db_create_arg *param = (struct audio_db_create_arg *)arg;
	
	DbAudioCreateDatabase(param->file_dir,param->db_path);
	/** should free the memory */
	free(param->file_dir);
	free(param->db_path);
	pthread_exit(NULL);
	return NULL;
}
static int audio_db_create(void *handle)
{
	int ret;
	pthread_attr_t attr;
	int rc=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = pthread_attr_init(&attr);
	if(ret)
	{
		printf("pthread_attr_init: errno:%d, %s\n", ret, strerror(ret));
		rc = 1;
		goto _CREATE_FAILED2;
	}
	
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(ret)
	{
		printf("pthread_attr_setdetachstate: errno:%d, %s\n", ret, strerror(ret));
		rc = 1;
		goto _CREATE_FAILED;
	}

	memset(&targ,0,sizeof(struct audio_db_create_arg));
	targ.file_dir = strdup(Swfext_GetString());
	if(targ.file_dir == NULL){
		rc = 1;
		goto _CREATE_FAILED;
	}
	targ.db_path = strdup(Swfext_GetString());
	if(targ.db_path == NULL){
		rc = 1;
		free(targ.file_dir);
		goto _CREATE_FAILED;
	}
	
	ret = pthread_create(&audio_db_thread, &attr, audio_db_create_loop, (void *)&targ);
	if(ret)
	{
		printf("pthread_create: errno:%d, %s\n", ret, strerror(ret));
		rc = 1;
		free(targ.file_dir);
		free(targ.db_path);
		goto _CREATE_FAILED;
	}
	
_CREATE_FAILED:
	pthread_attr_destroy(&attr);
	
_CREATE_FAILED2:
	Swfext_PutNumber(rc);
	SWFEXT_FUNC_END();
}	

static int audio_db_get_create_stat(void *handle)
{
	int stat;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	stat = DbAudioGetCreateStat();
	Swfext_PutNumber(stat);

	SWFEXT_FUNC_END();
}

static int audio_db_stop_create(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	DbAudioStopCreate();

	SWFEXT_FUNC_END();
}

static int audio_db_delete(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	DbAudioDeleteDatabase(path);

	SWFEXT_FUNC_END();
}

static int audio_db_check_database(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	if(access(path, F_OK)==0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();
}

int swfext_audio_db_register(void)
{
	SWFEXT_REGISTER("audio_db_open__c", audio_db_open);
	SWFEXT_REGISTER("audio_db_close__c", audio_db_close);
	SWFEXT_REGISTER("audio_db_get_album_number__c", audio_db_get_album_number);
	SWFEXT_REGISTER("audio_db_get_album_info__c", audio_db_get_album_info);
	SWFEXT_REGISTER("audio_db_sort_album__c", audio_db_sort_album);
	SWFEXT_REGISTER("audio_db_get_info_from_sorted_album__c", audio_db_get_info_from_sorted_album);
	SWFEXT_REGISTER("audio_db_get_artist_number__c", audio_db_get_artist_number);
	SWFEXT_REGISTER("audio_db_get_artist_info__c", audio_db_get_artist_info);
	SWFEXT_REGISTER("audio_db_sort_artist__c", audio_db_sort_artist);
	SWFEXT_REGISTER("audio_db_get_info_from_sorted_artist__c", audio_db_get_info_from_sorted_artist);
	SWFEXT_REGISTER("audio_db_get_genre_number__c", audio_db_get_genre_number);
	SWFEXT_REGISTER("audio_db_get_genre_info__c", audio_db_get_genre_info);
	SWFEXT_REGISTER("audio_db_sort_genre__c", audio_db_sort_genre);
	SWFEXT_REGISTER("audio_db_get_info_from_sorted_genre__c", audio_db_get_info_from_sorted_genre);
	SWFEXT_REGISTER("audio_db_get_song_number__c", audio_db_get_song_number);
	SWFEXT_REGISTER("audio_db_sort_song__c", audio_db_sort_song);
	SWFEXT_REGISTER("audio_db_get_info_from_sorted_song__c", audio_db_get_info_from_sorted_song);

	/** create the database */
	SWFEXT_REGISTER("audio_db_create__c", audio_db_create);
	SWFEXT_REGISTER("audio_db_get_create_stat__c", audio_db_get_create_stat);
	SWFEXT_REGISTER("audio_db_stop_create__c", audio_db_stop_create);
	SWFEXT_REGISTER("audio_db_check_database__c", audio_db_check_database);
	SWFEXT_REGISTER("audio_db_delete__c", audio_db_delete);

	return 0;
	
}

#endif

