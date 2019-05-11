#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "swf_ext.h"
#include "db_photo.h"

#ifdef MODULE_CONFIG_SQLITE
#define _photodb_DEBUG_
#ifdef _photodb_DEBUG_
#define photodb_debug(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#endif
typedef struct _PdDbEngine PdDbEngine;
struct _PdDbEngine
{
	DBPhotoManager *dbManager;

};

static PdDbEngine PbEngineManager=
{
	.dbManager = NULL,
};

/**
* @brief Open the photo database engine.
* 
* @return 0 for success , others for fail.
*/
static int photo_db_open(void *handle)
{
	int rtn=0;
	char *dbPath;
	
	SWFEXT_FUNC_BEGIN(handle);

	dbPath = Swfext_GetString();
	photodb_debug("dbPath========%s\n",dbPath);

	if(access(dbPath,F_OK)==-1){
		fprintf(stderr,"The DB %s does not exists\n",dbPath);
		rtn = 1;
		goto AUDIO_DB_OPEN_OUT;
	}

	/** if already open a database, close first */
	if(PbEngineManager.dbManager){
		DBPhotoClose(PbEngineManager.dbManager);
		PbEngineManager.dbManager = NULL;
	}
	
	PbEngineManager.dbManager = DBPhotoOpen(dbPath);
	if(PbEngineManager.dbManager == NULL){
		fprintf(stderr,"Open %s error\n",dbPath);
		rtn = 1;
	}

	
AUDIO_DB_OPEN_OUT:	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int photo_db_close(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	if(PbEngineManager.dbManager){
		DBPhotoClose(PbEngineManager.dbManager);
		PbEngineManager.dbManager = NULL;
	}
	
	SWFEXT_FUNC_END();
}



	

/************************************************************
* For video.
************************************************************/
static int photo_db_get_number(void *handle)
{
	DBPhotoManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = PbEngineManager.dbManager;

		if(pDbHandle){
			rc = DBPhotoGetViewInfo(pDbHandle);
			if(rc == 0){
				total = pDbHandle->photoView.photos.totalphotos;
			}
			else{
				fprintf(stderr,"Get genre infor error\n");
			}
		}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}


static int photo_db_sort(void *handle)
{
	DBPhotoManager *pDbHandle;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	sortType = Swfext_GetNumber();
	pDbHandle = PbEngineManager.dbManager;

	rc = DBPhotoDoViewSort(pDbHandle,sortType);
	if(rc==0){
		Swfext_PutNumber(pDbHandle->photoView.photos.totalphotos);
	}
	else{
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();
}


static int photo_db_get_info_from_sorted(void *handle)
{
	DBPhotoManager *pDbHandle;
	char *info=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = PbEngineManager.dbManager;

	info = DBPhotoGetSortedViewItem(pDbHandle,index);
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

struct photo_db_create_arg
{
	char *file_dir;
	char *db_path;
};

pthread_t photo_db_thread;
struct photo_db_create_arg targ;

static void *photo_db_create_loop(void *arg)
{
	struct photo_db_create_arg *param = (struct photo_db_create_arg *)arg;
	
	DbphotoCreateDatabase(param->file_dir,param->db_path);
	/** should free the memory */
	free(param->file_dir);
	free(param->db_path);
	pthread_exit(NULL);
	return NULL;
}
static int photo_db_create(void *handle)
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

	memset(&targ,0,sizeof(struct photo_db_create_arg));
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
	
	ret = pthread_create(&photo_db_thread, &attr, photo_db_create_loop, (void *)&targ);
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

static int photo_db_get_create_stat(void *handle)
{
	int stat;
	
	SWFEXT_FUNC_BEGIN(handle);
	photodb_debug("video_db_get_create_stat\n");;
	stat = DbPhotoGetCreateStat();
	
	photodb_debug("stat====%d\n",stat);;
	Swfext_PutNumber(stat);

	SWFEXT_FUNC_END();
}

static int photo_db_stop_create(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	DbPhotoStopCreate();

	SWFEXT_FUNC_END();
}

static int photo_db_delete(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	DbPhotoDeleteDatabase(path);

	SWFEXT_FUNC_END();
}

static int photo_db_check_database(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	photodb_debug("path=====%s\n",path);;
	if(access(path, F_OK)==0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();
}

int swfext_photo_db_register(void)
{
	SWFEXT_REGISTER("photo_db_open__c", photo_db_open);
	SWFEXT_REGISTER("photo_db_close__c", photo_db_close);
	SWFEXT_REGISTER("video_db_get_photo_number__c", photo_db_get_number);
	SWFEXT_REGISTER("photo_db_sort__c", photo_db_sort);
	SWFEXT_REGISTER("photo_db_get_info_from_sorted__c", photo_db_get_info_from_sorted);

	/** create the database */
	SWFEXT_REGISTER("photo_db_create__c", photo_db_create);
	SWFEXT_REGISTER("photo_db_get_create_stat__c", photo_db_get_create_stat);
	SWFEXT_REGISTER("photo_db_stop_create__c", photo_db_stop_create);
	SWFEXT_REGISTER("photo_db_check_database__c", photo_db_check_database);
	SWFEXT_REGISTER("photo_db_delete__c", photo_db_delete);

	return 0;
	
}

#endif

