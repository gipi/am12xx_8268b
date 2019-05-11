/**
* Database management for the video.
*/
#define __USE_LARGEFILE64
#define __FILE_OFFSET_BTS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_video.h"
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


/** export symbols for LSDK */
#ifdef WIN32
#define ATTR_VISIBLE
#else
#define ATTR_VISIBLE __attribute__((visibility("default")))
#endif

/**
* @brief Open the video database file.
*
* @param path : the full path of the data base file.
*
* @return return an video data base handler.
*/
ATTR_VISIBLE
DBVideoManager *DBVideoOpen(char *path)
{
	DBVideoManager *handler;
	int rc;

	if(path == NULL){
		return NULL;
	}

	handler = (DBVideoManager *)malloc(sizeof(DBVideoManager));
	if(handler == NULL){
		return NULL;
	}
	memset(handler,0,sizeof(DBVideoManager));

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






static void DBVideoFreeView(DBVideoManager *handler)
{
	DBVideoViewInfo *pVideo;

	if(handler == NULL){
		return;
	}

	handler->videoView.videos.totalVideos = 0;

	pVideo = &handler->videoView.videos;
	if(pVideo->videos.sortIndex){
		free(pVideo->videos.sortIndex);
		pVideo->videos.sortIndex = NULL;
	}
	
	return;
}

/**
* @brief Close an opened data base.
* 
* @param handler : the data base handler.
*/
ATTR_VISIBLE
void DBVideoClose(DBVideoManager *handler)
{
	if(handler == NULL){
		return;
	}

	if(handler->sqlHandler){
		sqlite3_close(handler->sqlHandler);
	}

	

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
int DBVideoGetViewInfo(DBVideoManager *handler)
{
	sqlite3_stmt *stmt;
	char *sql;
	int rc;
	const char *name;
	int i;


	if(handler == NULL){
		return 1;
	}

	DBVideoFreeView(handler);

		/** sql for getting song number */
	sql = "SELECT COUNT(id) FROM video_db";

	rc = sqlite3_prepare(handler->sqlHandler, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
			fprintf(stderr, "SQLite prepare error %d\n", rc);
			return rc;
	}

		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW) {
			handler->videoView.videos.totalVideos = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);


	return 0;
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
int DBVideoDoViewSort(DBVideoManager *handler,int sortType)
{
	sqlite3_stmt *stmt;
	DBVideoInfo *pInfo;
	int i;
	char sql[256];
	int rc;

	if(handler == NULL){
		return 1;
	}
	pInfo = &handler->videoView.videos;


	if(pInfo->totalVideos<=0){
		return 1;
	}

	if(pInfo->sortIndex){
		free(pInfo->sortIndex);
		pInfo->sortIndex = NULL;
	}
	pInfo->sortIndex = (int *)malloc(pInfo->totalVideos * sizeof(int));
	if(pInfo->sortIndex == NULL){
		return 1;
	}
	memset(pInfo->sortIndex,0,pInfo->totalVideos * sizeof(int));

	switch(sortType){
		case DBVIDEO_SORT_BY_NAME:
			sprintf(sql,"SELECT id,name FROM video_db ORDER BY name;");
			break;

		case DBVIDEO_SORT_BY_FILESIZE:
			sprintf(sql,"SELECT id,file_size FROM video_db ORDER BY file_size;");
			break;

		case DBVIDEO_SORT_BY_FILEEXTENSION:
			sprintf(sql,"SELECT id,file_extension FROM video_db ORDER BY file_extension;");
			break;

		case DBVIDEO_SORT_BY_TIME:
			sprintf(sql,"SELECT id,modify_time FROM video_db ORDER BY modify_time;");
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
		if(i>=pInfo->totalVideos){
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
char * DBVideoGetSortedViewItem(DBVideoManager *handler,int index)
{
	char *result=NULL;
	DBVideoInfo *pInfo;
	int id;
	char sql[128];
	int rc;
	sqlite3_stmt *stmt;
	char *dir;
	char *name;
	int fileSize;
	char *modifyTime;
	char *fileExt;
	

	if(handler == NULL){
		return NULL;
	}

	pInfo = &handler->videoView.videos;
	if(index >= pInfo->totalVideos){
		fprintf(stderr,"Index %d range error\n",index);
		return NULL;
	}

	if(pInfo->sortIndex == NULL){
		return NULL;
	}

	id = *(pInfo->sortIndex + index);
	sprintf(sql,"SELECT * FROM video_db WHERE id=%d;",id);

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


		result = (char *)malloc(256 + strlen(dir) + strlen(name) + strlen(modifyTime) +strlen(fileExt));

		if(result){
			sprintf(result,"<FullPath>=%s%s;<FileSize>=%d;<FileModifyTime>=%s;<FileExtension>=%s;",\
				dir,name,fileSize,modifyTime,fileExt);
		}
	}

	sqlite3_finalize(stmt);

	return result;
}


typedef struct _videoDbTable VideoDbTable;

struct _videoDbTable
{
	int id;
	char *dir;
	char *name;
	unsigned long long fileSize;
	char *modifyTime;
	char *ext;

};



static void __DbvideoFreeTable(VideoDbTable *table)
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

}





static int createProcess = 0;
int video_stopFlag=0;

ATTR_VISIBLE
int DbvideoCreateDatabase(char *filesDir,char *dbFullPath)
{
	UI_FILELIST filelist;
	int total;
	int i,j;
	char *file;
	VideoDbTable table;
	char *ptr,*ptr2,*ptr3;
	int len;
	struct stat64 fileInfo;
	struct tm mtime;
	void* fp;
	char *tmp;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int rc;
	char *sql;
	int ret = -1;

	

	createProcess = 0;
	video_stopFlag = 0;

	rc = sqlite3_open(dbFullPath, &db);
	if(rc) {
		fprintf(stderr, "Create database error: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return ret;
	}

	sql = (char *)malloc(512);
	if(sql == NULL){
		sqlite3_close(db);
		return ret;
	}

	/** create table */
	sprintf(sql,"CREATE TABLE video_db ( id integer primary key,directory text,name text,file_size integer,modify_time text,file_extension text );");
	rc = sqlite3_exec(db, sql,NULL, NULL,NULL);
	if(rc != SQLITE_OK){
		fprintf(stderr, "SQLite exec error %d\n", rc);
		sqlite3_close(db);
		free(sql);
		return ret;
	}

	/** prepare for insert items to the table */
	sprintf(sql,"INSERT INTO 'video_db' VALUES(:id_a,:dir_a,:name_a,:file_size_a,:mtime_a,:ext_a);");
	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	if(rc != SQLITE_OK){
		fprintf(stderr, "SQLite prepare error %d\n", rc);
		sqlite3_close(db);
		free(sql);
		remove(dbFullPath);
		return ret;
	}

	/** for get total time */

	filelist.fsel = NULL;
	ui_filelist_init(&filelist, filesDir, "avi mp4 3gp mov mpg mpeg mod vob ts tp m2t m2ts mts wmv mkv rm rmvb divx 3g2 m4v flv", 0, FLIST_MODE_BROWSE_FILE);
	total = ui_filelist_get_filetotal_num(&filelist);

	if(total < 1){
		fprintf(stderr,"No video Filse when creating dababase\n");
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		free(sql);
		remove(dbFullPath);
		createProcess=100;
		return ret;
	}

	ret = 0;
	if(video_stopFlag){
		goto _CREATE_OUT;
	}

	for(i=0;i<total;i++){

		/** update the creating process */
		createProcess = (i*100)/total;
		fprintf(stderr,"fucntion:%s,line:%d,createProcess======%d\n",__FUNCTION__,__LINE__,createProcess);
		if(video_stopFlag){
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
			if(stat64(file, &fileInfo)==0){
				table.fileSize =(unsigned long long)fileInfo.st_size;
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
		

			/** Total time */
	
			
			/** insert into table */
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":id_a"),table.id);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":dir_a"),table.dir,(int)strlen(table.dir),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":name_a"),table.name,(int)strlen(table.name),NULL);
			sqlite3_bind_int(stmt,sqlite3_bind_parameter_index(stmt, ":file_size_a"),table.fileSize);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":mtime_a"),table.modifyTime,(int)strlen(table.modifyTime),NULL);
			sqlite3_bind_text(stmt,sqlite3_bind_parameter_index(stmt, ":ext_a"),table.ext,(int)strlen(table.ext),NULL);

			sqlite3_step(stmt);
			rc = sqlite3_reset(stmt);
			if(rc != SQLITE_OK){
				fprintf(stderr,"SQLite reset error:%s\n",sqlite3_errmsg(db));
				__DbvideoFreeTable(&table);
				ret = -1;
				break;
			}
			/** free table memory */
			__DbvideoFreeTable(&table);
		}
	}

_CREATE_OUT:
	

	
	ui_filelist_exit(&filelist);
	
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(sql);
	
	createProcess = 100;
	
	return ret;
}


ATTR_VISIBLE
int DbVideoDeleteDatabase(char *path)
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
int DbVideoGetCreateStat()
{
	return createProcess;
}

ATTR_VISIBLE
int DbVideoStopCreate()
{
	video_stopFlag = 1;

	return 0;
}

