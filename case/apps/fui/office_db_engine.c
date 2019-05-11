#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "swf_ext.h"
#include "db_office.h"

#ifdef MODULE_CONFIG_SQLITE
//#define _officedb_DEBUG_
#ifdef _officedb_DEBUG_
#define officedb_debug(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define officedb_debug(fmt,arg...)do {} while (0)

#endif
typedef struct _OdDbEngine OdDbEngine;
struct _OdDbEngine
{
	DBOfficeManager *dbManager;

};

static OdDbEngine ObEngineManager=
{
	.dbManager = NULL,
};

/**
* @brief Open the audio database engine.
* 
* @return 0 for success , others for fail.
*/
static int office_db_open(void *handle)
{
	int rtn=0;
	char *dbPath;
	
	SWFEXT_FUNC_BEGIN(handle);

	dbPath = Swfext_GetString();
	officedb_debug("dbPath========%s\n",dbPath);

	if(access(dbPath,F_OK)==-1){
		fprintf(stderr,"The DB %s does not exists\n",dbPath);
		rtn = 1;
		goto AUDIO_DB_OPEN_OUT;
	}

	/** if already open a database, close first */
	if(ObEngineManager.dbManager){
		DBOfficeClose(ObEngineManager.dbManager);
		ObEngineManager.dbManager = NULL;
	}
	
	ObEngineManager.dbManager = DBOfficeOpen(dbPath);
	if(ObEngineManager.dbManager == NULL){
		fprintf(stderr,"Open %s error\n",dbPath);
		rtn = 1;
	}

	
AUDIO_DB_OPEN_OUT:	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int office_db_close(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	if(ObEngineManager.dbManager){
		DBOfficeClose(ObEngineManager.dbManager);
		ObEngineManager.dbManager = NULL;
	}
	
	SWFEXT_FUNC_END();
}



	

/************************************************************
* For office.
************************************************************/
static int office_db_get_number(void *handle)
{
	DBOfficeManager *pDbHandle;
	int total=0;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	pDbHandle = ObEngineManager.dbManager;

		if(pDbHandle){
			rc = DBOfficeGetViewInfo(pDbHandle);
			if(rc == 0){
				total = pDbHandle->officeView.offices.totalOffices;
			}
			else{
				fprintf(stderr,"Get genre infor error\n");
			}
		}

	Swfext_PutNumber(total);
	
	SWFEXT_FUNC_END();
}


static int office_db_sort(void *handle)
{
	DBOfficeManager *pDbHandle;
	int sortType;
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);

	sortType = Swfext_GetNumber();
	pDbHandle = ObEngineManager.dbManager;

	rc = DBOfficeDoViewSort(pDbHandle,sortType);
	if(rc==0){
		Swfext_PutNumber(pDbHandle->officeView.offices.totalOffices);
	}
	else{
		Swfext_PutNumber(0);
	}
	

	SWFEXT_FUNC_END();
}


static int office_db_select_by_filetype(void *handle)
{
	DBOfficeManager *pDbHandle;
	char FileType[256];
	int rc;
	
	SWFEXT_FUNC_BEGIN(handle);
	memset(FileType,0,256);
	memcpy(FileType, Swfext_GetString(),256);
	pDbHandle = ObEngineManager.dbManager;

	rc = DBOfficeSelectByFiletype(pDbHandle,FileType);
	if(rc==0){
		Swfext_PutNumber(pDbHandle->officeView.offices.totalOffices);
	}
	else{
		Swfext_PutNumber(0);
	}
	officedb_debug("pDbHandle->officeView.offices.totalOffices===%d\n",pDbHandle->officeView.offices.totalOffices);
	SWFEXT_FUNC_END();
}


static int office_db_get_info_from_sorted(void *handle)
{
	DBOfficeManager *pDbHandle;
	char *info=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	pDbHandle = ObEngineManager.dbManager;

	info = DBOfficeGetSortedViewItem(pDbHandle,index);
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

struct office_db_create_arg
{
	char *file_dir;
	char *db_path;
	char *db_type;
};

pthread_t office_db_thread;
struct office_db_create_arg targ;

static void *office_db_create_loop(void *arg)
{
	struct office_db_create_arg *param = (struct office_db_create_arg *)arg;
	
	DbofficeCreateDatabase(param->file_dir,param->db_path,param->db_type);
	/** should free the memory */
	free(param->file_dir);
	free(param->db_path);
	pthread_exit(NULL);
	return NULL;
}
static int office_db_create(void *handle)
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

	memset(&targ,0,sizeof(struct office_db_create_arg));
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

	targ.db_type = strdup(Swfext_GetString());
	if(targ.db_path == NULL){
		rc = 1;
		free(targ.file_dir);
		goto _CREATE_FAILED;
	}
	
	ret = pthread_create(&office_db_thread, &attr, office_db_create_loop, (void *)&targ);
	if(ret)
	{
		printf("pthread_create: errno:%d, %s\n", ret, strerror(ret));
		rc = 1;
		free(targ.file_dir);
		free(targ.db_path);
		
		free(targ.db_type);
		goto _CREATE_FAILED;
	}
	
_CREATE_FAILED:
	pthread_attr_destroy(&attr);
	
_CREATE_FAILED2:
	Swfext_PutNumber(rc);
	SWFEXT_FUNC_END();
}	

static int office_db_get_create_stat(void *handle)
{
	int stat;
	
	SWFEXT_FUNC_BEGIN(handle);
	officedb_debug("office_db_get_create_stat\n");;
	stat = DbOfficeGetCreateStat();
	
	officedb_debug("stat====%d\n",stat);;
	Swfext_PutNumber(stat);

	SWFEXT_FUNC_END();
}

static int office_db_stop_create(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	DbOfficeStopCreate();

	SWFEXT_FUNC_END();
}

static int office_db_delete(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	DbOfficeDeleteDatabase(path);

	SWFEXT_FUNC_END();
}

static int office_db_check_database(void *handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);

	path = Swfext_GetString();
	officedb_debug("path=====%s\n",path);;
	if(access(path, F_OK)==0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();
}

int swfext_office_db_register(void)
{
	SWFEXT_REGISTER("office_db_open__c", office_db_open);
	SWFEXT_REGISTER("office_db_close__c", office_db_close);
	SWFEXT_REGISTER("office_db_get_office_number__c", office_db_get_number);
	SWFEXT_REGISTER("office_db_sort__c", office_db_sort);
	SWFEXT_REGISTER("office_db_get_info_from_sorted__c", office_db_get_info_from_sorted);

	/** create the database */
	SWFEXT_REGISTER("office_db_create__c", office_db_create);
	SWFEXT_REGISTER("office_db_get_create_stat__c", office_db_get_create_stat);
	SWFEXT_REGISTER("office_db_stop_create__c", office_db_stop_create);
	SWFEXT_REGISTER("office_db_check_database__c", office_db_check_database);
	SWFEXT_REGISTER("office_db_delete__c", office_db_delete);

	
	SWFEXT_REGISTER("office_db_select_by_filetype__c", office_db_select_by_filetype);
	return 0;
	
}

#endif

