

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include "fui_common.h"
#include "swf_ext.h"
//#include "semaphore.h"
#include "fui_common.h"

#ifdef  MODULE_CONFIG_DLNA  //module config
#include  "dlna_dmp.h"

//#include "ILibParsers.h"
//#include "MediaServerCP_ControlPoint.h"
//#include "MediaServer_MicroStack.h"
//#include "ILibWebServer.h"
//#include "ILibAsyncSocket.h"

//#include "ILibThreadPool.h"
//#include <pthread.h>
//#include "FilteringBrowser.h"
//#include "CdsObject.h"
//#include "CdsDidlSerializer.h"
//#include "DLNAProtocolInfo.h"
//#include "DownloadController.h"

//#include "DmsIntegration.h"
//#include "stream_api.h"



static int am7x_dlna_start_work(void* handle)
{
	int ret= -1;
	SWFEXT_FUNC_BEGIN(handle);
	
	ret=dlna_start_work(NULL);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();		

}

#if EZMUSIC_ENABLE
int ezmusic_am7x_dlna_start_work(void)
{
	dlna_start_work(NULL);
}
#endif
static int am7x_dlna_stop_work(void*handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	dlna_stop_work(NULL);
	SWFEXT_FUNC_END();	
	
}
#if EZMUSIC_ENABLE
void ezmusic_am7x_dlna_stop_work(void)
{
	dlna_stop_work(NULL);
}
#endif
static int am7x_dlna_dmp_get_status(void *handle)
{
	int ret=-1;
	SWFEXT_FUNC_BEGIN(handle);
	ret=dlna_dmp_get_status(NULL);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	

}


static int am7x_dlna_dmp_get_total(void * handle)
{
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	ret=dlna_dmp_get_total(NULL);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int am7x_dlna_dmp_get_name(void * handle)
{
	char *ret=NULL;
	int index = -1;
	//void *llnode;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ret=dlna_dmp_get_name(index);
	Swfext_PutString(ret);
	
	SWFEXT_FUNC_END();	
}


static int am7x_dlna_dmp_get_uri(void * handle)
{

	char *ret=NULL;
	int index = -1;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ret=dlna_dmp_get_uri(index);
	Swfext_PutString(ret);
	SWFEXT_FUNC_END();	
}

static int am7x_dlna_dmp_enter(void *handle){

	int index = -1;
	int ret= -1;

	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ret=dlna_dmp_enter(index);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
	
}


static int am7x_dlna_dmp_escape(void *handle){
	int index = -1;
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ret=dlna_dmp_escape(NULL);
	Swfext_PutNumber(ret);
	
	SWFEXT_FUNC_END();	

}

static int am7x_dlna_dmp_get_obj_type(void*handle){
	int file_type = -1;
	int index = -1;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	file_type=dlna_dmp_get_obj_type(index);
	Swfext_PutNumber(file_type);
	SWFEXT_FUNC_END();	

}


static int am7x_dlna_download_file(void*handle)
{
//	dlna_info("");
	int index = -1;
	int ret = -1;
	char * save_path = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	save_path = Swfext_GetString();
	ret=dlna_download_file(index,save_path);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	

}



static int am7x_dlna_download_GetTotalBytesExpected(void*handle)
{
	int fsize = 0;
	SWFEXT_FUNC_BEGIN(handle);
	fsize=dlna_download_GetTotalBytesExpected(NULL);
	Swfext_PutNumber(fsize);

	SWFEXT_FUNC_END();	


}

static int am7x_dlna_download_GetBytesReceived(void*handle)
{
	int bytes_received = 0;
	SWFEXT_FUNC_BEGIN(handle);
	bytes_received=dlna_download_GetBytesReceived(NULL);
	Swfext_PutNumber(bytes_received);

	SWFEXT_FUNC_END();	

}


static int am7x_dlna_get_download_status(void*handle)
{
	int index = -1;
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	ret=dlna_get_download_status(NULL);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	

	return 0;
}


static int am7x_dlna_start_DMS(void*handle){
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	dlna_start_DMS(NULL);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	

}
#if EZMUSIC_ENABLE
int ezmusic_am7x_dlna_start_DMS(void)
{
	dlna_start_DMS(NULL);
}
#endif
static int am7x_dlna_stop_DMS(void*handle){
	
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	

}





int swfext_dlna_engine_register(void)
{
	SWFEXT_REGISTER("dlna_StartWork", am7x_dlna_start_work);
	SWFEXT_REGISTER("dlna_StopWork",am7x_dlna_stop_work);
	SWFEXT_REGISTER("dlna_DMPGetStatus",am7x_dlna_dmp_get_status);
	SWFEXT_REGISTER("dlna_DMPGetTotal", am7x_dlna_dmp_get_total);
	SWFEXT_REGISTER("dlna_DMPGetName", am7x_dlna_dmp_get_name);
	SWFEXT_REGISTER("dlna_DMPGetUri", am7x_dlna_dmp_get_uri);
	SWFEXT_REGISTER("dlna_DMPEnter", am7x_dlna_dmp_enter);
	SWFEXT_REGISTER("dlna_DMPEsc", am7x_dlna_dmp_escape);
	SWFEXT_REGISTER("dlna_DMPGetMediaType",am7x_dlna_dmp_get_obj_type);
	SWFEXT_REGISTER("dlna_MDMDDownloadFile",am7x_dlna_download_file);
	SWFEXT_REGISTER("dlna_MDMDGetTotalBytesExpected",am7x_dlna_download_GetTotalBytesExpected);
	SWFEXT_REGISTER("dlna_MDMDGetBytesReceived",am7x_dlna_download_GetBytesReceived);
	SWFEXT_REGISTER("dlna_MDMDGetDownloadStatus",am7x_dlna_get_download_status);
	SWFEXT_REGISTER("dlna_StartDMS",am7x_dlna_start_DMS);
	SWFEXT_REGISTER("dlna_StopDMS",am7x_dlna_stop_DMS);
	return 0;
}

#endif /** MODULE_CONFIG_DLNA */



