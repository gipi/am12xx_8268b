#include "swf_ext.h"
#include "mnavi_fop.h"
#include "filelist_engine.h"
#include <sys/stat.h>



#define fsapi_error(fmt,arg...)		//printf("ERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define fsapi_infor(fmt,arg...)		//printf("INFO[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

static int thread_sleep(unsigned int millsec){
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
	return 0;
}

static int _map_fops_status_to_asfserr(int status,int cmd)
{
	int asfs_err=ASFS_ERR_UNKOWN;
	switch(status){
		case MNAVI_WORK_IN_PROCESS:
		case MNAVI_WORK_FINISH_SUCCESS:
			asfs_err = ASFS_ERR_OK;
			break;
		case MNAVI_WORK_STOPED_FILE_EXSIT:
			asfs_err = ASFS_ERR_FileExist;
			break;
		case MNAVI_WORK_STOPED_RDONLY_FS:
			asfs_err = ASFS_ERR_ReadOnly;
			break;
		case MNAVI_WORK_STOPED_IO_ERROR:
			asfs_err = ASFS_ERR_DiskErr;
			break;
		case MNAVI_WORK_STOPED_INVALID_ARG:
			asfs_err = ASFS_ERR_UNKOWN;
			break;
		case MNAVI_WORK_STOPED_USER_CANCEL:
			switch(cmd){
				case MNAVI_CMD_FILE_DEL:
					asfs_err = ASFS_ERR_DELFILE_STOP;
					break;
				case MNAVI_CMD_DIR_DEL:
					asfs_err = ASFS_ERR_DELDIR_STOP;
					break;
				case MNAVI_CMD_FILE_COPY:
				case MNAVI_CMD_FILE_OVERLAP:
					asfs_err = ASFS_ERR_COPYFILE_STOP;
					break;
				case MNAVI_CMD_DIR_COPY:
				case MNAVI_CMD_DIR_OVERLAP:
					asfs_err = ASFS_ERR_COPYDIR_STOP;
					break;
				default:
					fsapi_error("Sorry Cmd=%d Not Support!",cmd);
			}
			break;
		default:
			fsapi_error("Sorry status=%d Not Support!",status);
			break;
	}
	return asfs_err;
}

/**
@brief call this function to del or copy file or dir
@param[in] cmd	: MNAVI_CMD_FILE_COPY ect
@parma[in] dest	: the destination where the current file is
@param[in] src	: where the file will be copy to, only used when the cmd==MNAVI_CMD_FILE_COPY.\
					MNAVI_CMD_DIR_COPY,MNAVI_CMD_FILE_OVERLAP,MNAVI_CMD_DIR_OVERLAP.
@return see ASFS_ERR_OK etc
**/
int file_opt(int cmd,char* dest,char *src){
	int rtn = 0;
	int process=0;
	if(file_ops_start_work(cmd,dest,src)!=0){
		rtn = ASFS_ERR_UNKOWN;
		goto FILE_OPT_END;
	}
	while(1){
		process = file_ops_get_process();
		if(process==-1||process==100 || process==-2){
			int status;
			status = file_ops_get_status();
			fsapi_infor("process=%d, cmd=%d: status=%d",process,cmd,status);
			rtn = _map_fops_status_to_asfserr(status,cmd);
			file_ops_stop_work();
			break;
		}
		else{
			thread_sleep(10);
		}
	}
FILE_OPT_END:
	return rtn;
}

static int fs_remove_file(void * handle){
	char* filename;
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	filename = Swfext_GetString();
	fsapi_infor("Remove file=%s",filename);
	rtn = file_opt(MNAVI_CMD_FILE_DEL,NULL,filename);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int  fs_copy_file(void * handle){
	char* filesrc;
	char* dirdest;
	char TmpBuf[MNAVI_MAX_NAME_LEN];
	int err=0;
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	filesrc = Swfext_GetString();
	dirdest = Swfext_GetString();
	fsapi_infor("COPY File: filesrc=%s,dirdest=%s",filesrc,dirdest);
	memset(TmpBuf,0,MNAVI_MAX_NAME_LEN);
	if(_get_dest_file(filesrc,dirdest,TmpBuf)==0){
		rtn = ASFS_ERR_UNKOWN;
		goto COPY_ERROR;
	}
	err = access(TmpBuf,F_OK);
	if(err == -1){
		rtn = file_opt(MNAVI_CMD_FILE_COPY,dirdest,filesrc);
	}
	else
		rtn = ASFS_ERR_FileExist;

COPY_ERROR:	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int fs_overlap_file(void *handle){
	char* filesrc;
	char* dirdest;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	filesrc = Swfext_GetString();
	dirdest = Swfext_GetString();
	fsapi_infor("OVER LAP: filesrc=%s,dirdest=%s",filesrc,dirdest);
	rtn = file_opt(MNAVI_CMD_FILE_OVERLAP,dirdest,filesrc);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int fs_mkdir(void *handle){
	char* dir_path;
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	dir_path = Swfext_GetString();
	fsapi_infor("MAKE DIR: path=%s",dir_path);
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			fsapi_error("Make Dir Failed=%s\n",dir_path);
		}
	}	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

int swfext_fs_register(void)
{
	SWFEXT_REGISTER("fs_removeFile",fs_remove_file);
	SWFEXT_REGISTER("fs_copyFile",fs_copy_file);
	SWFEXT_REGISTER("fs_overlapCopyFile",fs_overlap_file);
	SWFEXT_REGISTER("fs_mkdir",fs_mkdir);
	return 0;
}