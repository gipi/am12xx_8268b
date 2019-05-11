#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "db_audio.h"

//#define TEST_ALBUM_VIEW
//#define TEST_ARTIST_VIEW
//#define TEST_GENRE_VIEW
//#define TEST_SONG_VIEW

int main(int argc, char **argv)
{
	DBAudioManager *handle;
	int i;
	DBAudioAlbumInfo *palbum;
	DBAudioArtistInfo *partist;
	DBAudioGenreInfo *pgenre;
	char *sRet;

	handle = DBAudioOpen("/mnt/udisk/AudioInfoDB.db");
	if(handle == NULL){
		printf("db open error\n");
	}

#ifdef TEST_ALBUM_VIEW
	DBAudioGetViewInfo(handle,DBAUDIO_VIEW_ALBUM);
#endif

#ifdef TEST_ARTIST_VIEW
	DBAudioGetViewInfo(handle,DBAUDIO_VIEW_ARTIST);
#endif

#ifdef TEST_GENRE_VIEW
	DBAudioGetViewInfo(handle,DBAUDIO_VIEW_GENRE);
#endif

#ifdef TEST_SONG_VIEW
	DBAudioGetViewInfo(handle,DBAUDIO_VIEW_SONG);
#endif

#ifdef TEST_ALBUM_VIEW
	if(handle->albumView.totalAlbums > 0){
		palbum = handle->albumView.album;
		while(palbum){
			//printf("album:%s\n",palbum->albumName);
			DBAudioDoAlbumViewSort(handle,palbum->albumName,DBAUDIO_SORT_BY_TOTAL_TIME);
			palbum = palbum->next;
			//printf("\n");
		}

		palbum = handle->albumView.album;
		if(palbum){
			for(i=0;i<palbum->totalSongs;i++){
				sRet = DBAudioGetSortedAlbumViewItem(handle,palbum->albumName,i);
				if(sRet){
					printf("%s\n\n",sRet);
					free(sRet);
				}
			}
		}

	}
	
#endif

#ifdef TEST_ARTIST_VIEW
	if(handle->artistView.totalArtist > 0){
		partist = handle->artistView.artist;
		while(partist){
			//printf("artist:%s\n",partist->artistName);
			DBAudioDoArtistViewSort(handle,partist->artistName,DBAUDIO_SORT_BY_NAME);
			partist = partist->next;
			//printf("\n");
		}

		partist = handle->artistView.artist;
		if(partist){
			for(i=0;i<partist->totalSongs;i++){
				sRet = DBAudioGetSortedArtistViewItem(handle,partist->artistName,i);
				if(sRet){
					printf("%s\n\n",sRet);
					free(sRet);
				}
			}
		}
	}
#endif

#ifdef TEST_GENRE_VIEW
	if(handle->genreView.totalGenre > 0){
		pgenre = handle->genreView.genre;
		while(pgenre){
			printf("genre:%s\n",pgenre->genreName);
			DBAudioDoGenreViewSort(handle,pgenre->genreName,DBAUDIO_SORT_BY_NAME);
			pgenre = pgenre->next;
			printf("\n");
		}

		pgenre = handle->genreView.genre;
		if(pgenre){
			for(i=0;i<pgenre->totalSongs;i++){
				sRet = DBAudioGetSortedGenreViewItem(handle,pgenre->genreName,i);
				if(sRet){
					printf("%s\n\n",sRet);
					free(sRet);
				}
			}
		}
	}
#endif

#ifdef TEST_SONG_VIEW
	DBAudioDoSongViewSort(handle,DBAUDIO_SORT_BY_NAME);
	for(i=0;i<handle->songView.songs.totalSongs;i++){
		sRet = DBAudioGetSortedSongViewItem(handle,i);
		if(sRet){
			printf("%s\n\n",sRet);
			free(sRet);
		}
	}
#endif

	DBAudioClose(handle);

	DbAudioCreateDatabase("/mnt/udisk/","/mnt/udisk/test_audio.db");

    return 0;
}
