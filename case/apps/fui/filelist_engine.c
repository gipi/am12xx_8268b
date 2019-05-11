#define __USE_LARGEFILE64
#define __FILE_OFFSET_BTS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include "filelist_engine.h"
#include "mnavi_fop.h"
#include "file_list.h"
#include "system_res.h"
#include "ezcast_public.h"
#include <errno.h>
#include <wait.h>


#include <fcntl.h>


UI_FILELIST swf_ext_ui_filelist={
	.fsel = NULL,
}; 


#define TIME_STR_BUF 128
char time_string[TIME_STR_BUF];
struct mnavi_work_handle_t* workhandle=NULL;
struct mnavi_work_info workinfo;
extern char *disk_rootpath[];

static char TmpBuf[MNAVI_MAX_NAME_LEN];
static char pathBuf[512];

static char currentMedia_path[64];
static char currentMedia_type[512];


static UI_FILELIST *last_flist=NULL;
static void fe_change_work_path(UI_FILELIST *ui_flist)
{
#if 0
	if(last_flist!=ui_flist){
		if(ui_flist){
			// printf("%s,%d:change pSel=0x%x,WorkPath===%s,curPath=%s\n",__FILE__,__LINE__,ui_flist->fsel,ui_flist->work_path,ui_flist->fsel->cur_path);
			chdir(ui_flist->fsel->cur_search_path);
		}
		last_flist = ui_flist;
	}	
#endif
}

static void  fe_save_work_path(UI_FILELIST * ui_flist)
{
#if 0
	char curPath[MAX_NAME_LEN];
	if(ui_flist){
		if(getcwd(curPath,sizeof(curPath))!=NULL){
			strcpy(ui_flist->work_path,curPath);
			//printf("%s,%d:Save pSel=0x%x,WorkPath===%s\n",__FILE__,__LINE__,ui_flist->fsel,ui_flist->work_path);
		}
	}
#endif
}

static inline void fe_reset_last_flist(UI_FILELIST *ui_flist)
{
	if(ui_flist)
		last_flist=NULL;
}

static inline char __check_filelist_valid()
{
	return (swf_ext_ui_filelist.fsel==NULL)?0:1;
}

static int fe_get_total(void * handle)
{
	int totalnum=0;
	SWFEXT_FUNC_BEGIN(handle);

	if(__check_filelist_valid()){
		totalnum = ui_filelist_get_filetotal_num(&swf_ext_ui_filelist);
		//totalnum=mnavi_get_file_total(swf_ext_ui_filelist.fsel,MNAVI_FS_ALL);
		//printf("totalnum1=======%d\n",totalnum);
	}
	Swfext_PutNumber(totalnum);
	SWFEXT_FUNC_END();
}

static int fe_get_path(void * handle)
{
	char * path=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);

	if(__check_filelist_valid())
		path = ui_filelist_get_cur_workpath(&swf_ext_ui_filelist);
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

static int fe_set_path(void * handle)
{
	char * path;
	char * filetype;
	int ret=0;
	int n;
	filelist_mode_e ftype;
	int filesort=MNAVI_FS_ALL;

	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	if(n == 0){
		path = disk_rootpath[NAND_DISK];
	}
	else{
		path = Swfext_GetString();	
	}

	filetype = Swfext_GetString();
	if(__check_filelist_valid()){
		ui_filelist_exit(&swf_ext_ui_filelist);

		swf_ext_ui_filelist.fsel = NULL;
	}
	
	if(strcmp(filetype,"") ==0){
		ftype = FLIST_MODE_BROWSE_ALL;
	}
	else{
		ftype = FLIST_MODE_BROWSE_FILE;
	}
	
	fe_save_work_path(last_flist);
	ui_filelist_init(&swf_ext_ui_filelist, path, filetype, 0, ftype);
	//printf("go to 1\n");
	//ret=ui_filelist_dir_file_apart(&swf_ext_ui_filelist,filesort);
	printf("ret is %d\n",ret);
	fe_change_work_path(&swf_ext_ui_filelist);
	//Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int fe_release_path(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);	
	printf("%s,%d:Call Release Path !\n",__FILE__,__LINE__);
	if(__check_filelist_valid()){
		ui_filelist_exit(&swf_ext_ui_filelist);

		swf_ext_ui_filelist.fsel = NULL;
		fe_reset_last_flist(&swf_ext_ui_filelist);
	}
	SWFEXT_FUNC_END();
}

static int fe_get_filetype(void * handle)
{
	int index, type;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		//printf("Q20\n");
		type = ui_filelist_get_cur_filetype(&swf_ext_ui_filelist, index);
		//printf("type======%d\n",type);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	else
		type = -1;
	Swfext_PutNumber(type);
	
	SWFEXT_FUNC_END();
}

static int fe_get_filename(void * handle)
{
	char * file=NULL;
	int index;
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_longname(&swf_ext_ui_filelist, index);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	Swfext_PutString(file);
	
	SWFEXT_FUNC_END();	
}

static int fe_get_filepath(void * handle)
{
	static char * file=NULL;
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);

	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	Swfext_PutString(file);
	
	SWFEXT_FUNC_END();	
}


static int fe_get_dir_total_item(void * handle)
{
	char * file=NULL;
	int index;
	int num=0;
	DIR *pdir;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
		pdir = opendir(file);
		if(pdir){
			while(readdir(pdir)){
				num++;
			}
			num = num-2;	//exclude . and .. two dir
			closedir(pdir);
		}
	}
	else
		num = 0;
	printf("%s,%d:num is %d\n",__FILE__,__LINE__,num);
	Swfext_PutNumber(num);
	
	SWFEXT_FUNC_END();	
}


static int fe_get_filetime_modified(void * handle)
{
	char * file;
	int index;
	struct stat filestat;
	char *timestr;
	memset(time_string,0,TIME_STR_BUF);
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
		fe_save_work_path(&swf_ext_ui_filelist);
		if(stat(file,&filestat)==0){
			timestr = ctime(&filestat.st_mtime);
			memcpy(time_string,timestr,strlen(timestr));
			Swfext_PutString(time_string);
		}
		else{
			Swfext_PutString(time_string);
		}	
	}
	Swfext_PutString(time_string);
	SWFEXT_FUNC_END();	
}

static int fe_get_filesize(void * handle)
{
	char * file;
	int index;
	FILE *fp=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
		fe_save_work_path(&swf_ext_ui_filelist);
		fp = fopen(file,"rb");
		if(fp == NULL){
			Swfext_PutNumber(0);
		}
		else{
			if(fseek(fp, 0, SEEK_END)==0){
				Swfext_PutNumber(ftell(fp));
			}
			else{
				Swfext_PutNumber(0);
			}
			fclose(fp);
		}
	}
	SWFEXT_FUNC_END();	
}

static int fe_enter_dir(void * handle)
{
	int index;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();

	/** FIXME: [do enter works] */
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		//printf("fe_enter_dir_index====%d\n",index);
		if(ui_filelist_enter_dir(&swf_ext_ui_filelist,index)==0){
			printf("[ERROR]%s,%d:Enter Dir Error\n",__func__,__LINE__);
			Swfext_PutNumber(0);
		}
		else
			Swfext_PutNumber(1);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	else
		Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

static int fe_exit_dir(void * handle)
{
	int rtn;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [do exit dir works] */
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		rtn = ui_filelist_esc_dir(&swf_ext_ui_filelist);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	else
		rtn = 0;
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}


#if 1
int _get_dest_file(char *srcfile,char *destdir,char *destfile){
	int i=0;
	char *tmp=srcfile;
	int srcfilelen=strlen(srcfile);
	for(i=srcfilelen-1;i>=0;i--){
		if(tmp[i]=='/'){
			if(i==srcfilelen-1)
				continue;
			break;
		}
	}
	if(i>0){
		tmp =tmp+i+1;
	}
	else{
		printf("%s,%d:Srcfile Err=%s\n",__FILE__,__LINE__);
	}
	strcpy(destfile,destdir);
	strcat(destfile,tmp);
	printf("Get Dest File==%s\n",destfile);
	return 1;
}

int _change_file_name(char *srcfile,char* destfile){
	int srcfilelen=strlen(srcfile);
	int i=0;
	char *tmp=srcfile;
	while(*tmp!=0){
		if(*tmp==0x20){//it is a space 
			*(destfile+i)='\\';
			i++;
			*(destfile+i)=*tmp;
		}
		else
			*(destfile+i)=*tmp;
		i++;
		tmp++;
	}
	return 1;
}

 int  file_ops_start_work(int cmd,char*des,char*src){
	workinfo.cmd = cmd;
	workinfo.des = des;
	workinfo.src = src;
	int ret = 0;
	if(workhandle!=NULL){
		//printf("%s,%d:Please Call Stop work First\n",__FILE__,__LINE__);
		//return -1;
		mnavi_stop_work(workhandle);
		mnavi_release_work(workhandle);
		workhandle=NULL;
	}
	//printf("%s,%d:cmd=%d\n",__FILE__,__LINE__,workinfo.cmd);
	workhandle = mnavi_create_work(&workinfo);
	ret = mnavi_start_work(workhandle);
	return ret;
}

 int file_ops_stop_work(){
	if(workhandle!=NULL){
		mnavi_stop_work(workhandle);
		mnavi_release_work(workhandle);
		workhandle=NULL;
	}
	else
		printf("%s,%d:WorkHandle==NULL,Do nothing\n",__FILE__,__LINE__);
	return 0;
}

 int file_ops_get_process(){
	double process;
	int percent=0;
	if(workhandle!=NULL){
		process =  mnavi_get_process(workhandle);
		percent = (int)(process*100);
		if(percent==-100)//err,call file_ops_get_status to get the err no
			return -1;
		else
			return percent;
	}
	else
		return -2;
}

int file_ops_get_status(){
	int status=-1;
	if(workhandle!=NULL){
		status =  mnavi_get_status(workhandle);
	}
	else{
		printf("%s,%d:Work Handle==NULL\n",__FILE__,__LINE__);
	}
	return status;
}

static int fe_copy_file(void * handle)
{
	int index;
	int rtn=0;
	char * file,*p;
	int err=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	p = Swfext_GetString();
	
	fe_change_work_path(&swf_ext_ui_filelist);
	file=ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	fe_save_work_path(&swf_ext_ui_filelist);
	
	printf("Copy File file=%s,des=%s\n",file,p);
	/** do copy works */
	
	memset(TmpBuf,0,MNAVI_MAX_NAME_LEN);
	if(_get_dest_file(file,p,TmpBuf)==0){
		rtn = ASFS_ERR_UNKOWN;
		goto _DO_COPY_OUT_;
	}
	printf("%s,%d:Copy file des=%s,src=%s\n",__FILE__,__LINE__,p,file);
	err = access(TmpBuf,F_OK);
	//printf("%s,%d:access=%d\n",__func__,__LINE__,err);
	if(err == -1){
		if(file_ops_start_work(MNAVI_CMD_FILE_COPY,p,file)==0){
			rtn = ASFS_ERR_OK;
		}
		else
			rtn = ASFS_ERR_UNKOWN;
	}
	else
		rtn = ASFS_ERR_FileExist;

_DO_COPY_OUT_:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}



static int fe_delete_file(void * handle)
{
	int index;
	char * file;
	int ret=ASFS_ERR_UNKOWN,err;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();

	/*
	swf_ext_ui_filelist.list.active = index;
	err=ui_filelist_del_item(&swf_ext_ui_filelist,index);
	if(err==-1)
		ret = 0;
	else
		ret = 1;
	*/
	fe_change_work_path(&swf_ext_ui_filelist);
	//printf("ui_filelist_get_cur_filepath_index=======%d\n",index);
	file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	fe_save_work_path(&swf_ext_ui_filelist);
	
	printf("%s,%d:Call Del File=%s\n",__FILE__,__LINE__,file);
	if(file!=NULL&&file_ops_start_work(MNAVI_CMD_FILE_DEL,NULL,file)==0){
		ret = ASFS_ERR_OK;
	}
	else
		ret = ASFS_ERR_UNKOWN;
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int fe_overlap_copy(void * handle)
{
	int index;
	int err;
	char *p,*file;
	int rtn=0;
	int ret=ASFS_ERR_UNKOWN;
	int type=0;
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it] */
	index = Swfext_GetNumber();
	p = Swfext_GetString();
	
	fe_change_work_path(&swf_ext_ui_filelist);
	file=ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	type = ui_filelist_get_cur_filetype(&swf_ext_ui_filelist, index);
	fe_save_work_path(&swf_ext_ui_filelist);
	
	printf("%s,%d:srcFile=%s,des=%s, Type=%d\n",__FILE__,__LINE__,file,p,type);
	if(type==FT_DIR)
		rtn = file_ops_start_work(MNAVI_CMD_DIR_OVERLAP,p,file);
	else
		rtn = file_ops_start_work(MNAVI_CMD_FILE_OVERLAP,p,file);
	if(rtn ==0){
		rtn = ASFS_ERR_OK;
	}
	else
		rtn = ASFS_ERR_UNKOWN;
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


static int fe_copy_dir(void * handle)
{
	int index;
	char * dir,*p;
	int err;
	char ret=ASFS_ERR_UNKOWN;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	p= Swfext_GetString();

	fe_change_work_path(&swf_ext_ui_filelist);
	dir = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	fe_save_work_path(&swf_ext_ui_filelist);
	
	printf("Copy Dir dir=%s,des=%s\n",dir,p);
	if(strlen(p) > 118 || strlen(dir)>118){
		ret = ASFS_ERR_UNKOWN;
		printf("COPY DIR ERROR: PATH TOO LONG!!\n");
		goto _DO_COPY_DIR_OUT_;
	}
	memset(TmpBuf,0,MNAVI_MAX_NAME_LEN);
	if(_get_dest_file(dir,p,TmpBuf)==0){
		ret = ASFS_ERR_UNKOWN;
		goto _DO_COPY_DIR_OUT_;
	};
	printf("%s,%d:Copy Dir des=%s,src=%s\n",__FILE__,__LINE__,p,dir);
	err = access(TmpBuf,F_OK);
	if(err==-1){
		if(file_ops_start_work(MNAVI_CMD_DIR_COPY,p,dir)==0){
			ret = ASFS_ERR_OK;
		}
		else
			ret = ASFS_ERR_UNKOWN;
	}
	else
		ret = ASFS_ERR_FileExist;
	
_DO_COPY_DIR_OUT_:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int fe_delete_dir(void * handle)
{
	int index;
	char * dir;
	int err,ret=ASFS_ERR_UNKOWN;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	index = Swfext_GetNumber();
	
	fe_change_work_path(&swf_ext_ui_filelist);
	dir=ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	fe_save_work_path(&swf_ext_ui_filelist);
	
	printf("%s,%d:Call Del Dir=%s\n",__FILE__,__LINE__,dir);
	if(dir!=NULL&&(file_ops_start_work(MNAVI_CMD_DIR_DEL,NULL,dir)==0)){
		ret = ASFS_ERR_OK;
	}
	else
		ret = ASFS_ERR_UNKOWN;
		
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int fe_fresh_flist(void *handle){
	int index;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	/** FIXME: [finishi it]*/
	index = Swfext_GetNumber();
	//printf("Fresh Filelist:%s,%d:flhandle=0x%x,index=%d\n",__FILE__,__LINE__,fhandle,index);
	
	fe_change_work_path(&swf_ext_ui_filelist);
	ret=ui_filelist_refresh_list(&swf_ext_ui_filelist,index);//2011.09.16 add index para input
	fe_save_work_path(&swf_ext_ui_filelist);
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


static int fe_get_process(void * handle)
{
	int process =0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finishi it]*/
	process = file_ops_get_process();
	Swfext_PutNumber(process);
	
	SWFEXT_FUNC_END();
}

static int fe_get_status(void*handle){
	int status =0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finishi it]*/
	status = file_ops_get_status();
	//printf("%s,%d:Get Status status==%d\n",__FILE__,__LINE__,status);
	Swfext_PutNumber(status);
	
	SWFEXT_FUNC_END();
}

static int fe_stop_work(void*handle){
	SWFEXT_FUNC_BEGIN(handle);
	/** FIXME: [finishi it]*/
	file_ops_stop_work();
	//printf("%s,%d:Stop Work\n",__FILE__,__LINE__);
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static int fe_attach_sortview(void *handle)
{
	int ret=0;
	int filesort=MNAVI_FS_ALL;
	SWFEXT_FUNC_BEGIN(handle);
	//filesort = Swfext_GetNumber();
	ret = ui_filelist_creat_sortview(&swf_ext_ui_filelist,filesort);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int fe_detach_sortview(void *handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);
	ret = ui_filelist_delete_sortview(&swf_ext_ui_filelist);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


#endif
/**
* For file list search by task
*/

SWF_FILELIST multi_filelist_handle[SWF_MAX_FILELIST]; 

static void _fe_filelist_init()
{
	int i;
	for(i=0;i<SWF_MAX_FILELIST;i++){
		multi_filelist_handle[i].free=1;
	}
}

static int  _fe_get_free_list()
{
	int i;
	for(i=0;i<SWF_MAX_FILELIST;i++){
		if(multi_filelist_handle[i].free){
			multi_filelist_handle[i].free=0;
			break;
		}
	}

	if(i<SWF_MAX_FILELIST){
		return i;
	}
	else{
		return -1;
	}
}

/**
* prototype should be "setPathByMode(char *path,char *extension,int mode,int scan)"
* @return:
*	sucess: a valid handle>=0
*	fail: -1
*/

static int fe_set_path_new(void * handle)
{
	char * path;
	char * filetype;
	int n;
	int fileHandle=-1;
	int mode;
	int scan;
	int list_rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	
	if(n!=4){
		printf("invalid prototype %s\n",__FUNCTION__);
		goto _SET_PATH_OUT;
	}
	
	fileHandle=_fe_get_free_list();
	if(fileHandle==-1){
		goto _SET_PATH_OUT;
	}

	path=Swfext_GetString();
	filetype = Swfext_GetString();
	mode=Swfext_GetNumber();
	scan=Swfext_GetNumber();

	fe_save_work_path(last_flist);
	list_rtn = ui_filelist_init(&multi_filelist_handle[fileHandle].filelist, path, filetype, scan,mode);
//	list_rtn = ui_filelist_init(&multi_filelist_handle[fileHandle].filelist, path, filetype,mode,scan);
	fe_change_work_path(&multi_filelist_handle[fileHandle].filelist);
	
	if(list_rtn==-1){
		multi_filelist_handle[fileHandle].free=1;
		fileHandle = -1;
	}


	
_SET_PATH_OUT:
	Swfext_PutNumber(fileHandle);
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "setPathByMode(char *path,char *extension,int mode,int scan)"
* @return:
*	sucess: a valid handle>=0
*	fail: -1
*/

static int fe_set_path_new_deamon(void * handle)
{
	char * path;
	char * filetype;
	int n;
	int fileHandle=-1;
	int mode;
	int scan;
	int list_rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	
	if(n!=4){
		printf("invalid prototype %s\n",__FUNCTION__);
		goto _SET_PATH_OUT;
	}
	
	fileHandle=_fe_get_free_list();
	if(fileHandle==-1){
		goto _SET_PATH_OUT;
	}

	path=Swfext_GetString();
	filetype = Swfext_GetString();
	mode=Swfext_GetNumber();
	scan=Swfext_GetNumber();

	fe_save_work_path(last_flist);
	list_rtn = ui_filelist_init_deamon(&multi_filelist_handle[fileHandle].filelist, path, filetype, scan,mode);
//	list_rtn = ui_filelist_init(&multi_filelist_handle[fileHandle].filelist, path, filetype,mode,scan);
	fe_change_work_path(&multi_filelist_handle[fileHandle].filelist);
	
	if(list_rtn==-1){
		multi_filelist_handle[fileHandle].free=1;
		fileHandle = -1;
	}


	
_SET_PATH_OUT:
	Swfext_PutNumber(fileHandle);
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getScanTaskStatNew(int handle)"
*/
static int fe_get_scan_task_stat_new(void * handle)
{
	int stat=NO_SENSE;
	int n;
	int listhandle;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	
	if(n==0){
		goto _GET_STAT_OUT;
	}

	listhandle=Swfext_GetNumber();
	if(listhandle<0 || listhandle>=SWF_MAX_FILELIST){
		goto _GET_STAT_OUT;
	}
	if(multi_filelist_handle[listhandle].free){
		goto _GET_STAT_OUT;
	}

	stat = ui_filelist_check_scan_task(&multi_filelist_handle[listhandle].filelist);

_GET_STAT_OUT:
	Swfext_PutNumber(stat);
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getPathNew(int handle,int index,int buf)"
*/

static int fe_get_path_new(void * handle)
{
	static char * path=NULL;
	int index,flhandle;
	int n;
	char buffer[256*3];
	int bufen=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	path=NULL;
	n=Swfext_GetParamNum();

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_PATH_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _GET_PATH_NEW_OUT;
	}

	index=Swfext_GetNumber();

	if(n==3){
		bufen=Swfext_GetNumber();
	}

	if(n==2){
		path = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	}
	else{
		if(bufen){
			//printf("use buffer\n");
			path = ui_filelist_get_cur_filepath_buf(&multi_filelist_handle[flhandle].filelist,index,buffer);
		}
		else{
			path = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
		}
	}

_GET_PATH_NEW_OUT:
	
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "releasePathNew(int handle)"
*/
static int fe_release_path_new(void * handle)
{
	int flhandle;
	
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _RELEASE_PATH_NEW_OUT;
	}

	if(multi_filelist_handle[flhandle].free){
		if(multi_filelist_handle[flhandle].filelist.fsel!=NULL){
			ui_filelist_exit(&multi_filelist_handle[flhandle].filelist);

			fe_reset_last_flist(&multi_filelist_handle[flhandle].filelist);
			
		}
		goto _RELEASE_PATH_NEW_OUT;
	}
	else{
		if(multi_filelist_handle[flhandle].filelist.fsel!=NULL){
			ui_filelist_exit(&multi_filelist_handle[flhandle].filelist);

			fe_reset_last_flist(&multi_filelist_handle[flhandle].filelist);
			
		}
		multi_filelist_handle[flhandle].free=1;
	}

_RELEASE_PATH_NEW_OUT:

	SWFEXT_FUNC_END();
}

/**
* prototype should be "getTotalNew(int handle)"
*/

static int fe_get_total_new(void * handle)
{
	int total=0,flhandle;
	
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_TOTAL_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _GET_TOTAL_NEW_OUT;
	}
	//total = multi_filelist_handle[flhandle].filelist.list.total;
	
	total = ui_filelist_get_filetotal_num(&multi_filelist_handle[flhandle].filelist);
	
_GET_TOTAL_NEW_OUT:
	
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();
}


/**
* prototype should be "getLongnameNew(int handle,int index)"
*/

static int fe_get_longname_new(void * handle)
{
	static char * path=NULL;
	int index,flhandle;

	SWFEXT_FUNC_BEGIN(handle);
	path=NULL;
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_PATH_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _GET_PATH_NEW_OUT;
	}

	index=Swfext_GetNumber();
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	path = ui_filelist_get_longname(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
_GET_PATH_NEW_OUT:
	
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getFileSizeNew(int handle,int index)"
*/
unsigned long long get_file_size(char *file_name){
	struct stat64 buf;
	if(stat64(file_name,&buf)<0){
		//fprintf(stderr,"function:%s line:%d strerror:%s\n",__FUNCTION__,__LINE__,strerror(errno));
		return 0;
	}
	//printf("buf.st_size===========%ld\n",buf.st_size);
	return (unsigned long long)buf.st_size;
}

static int fe_get_file_size_new(void * handle)
{
	char * file;
	int index;
	int flhandle;
	double size=0;
	FILE *fp;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_FILESIZE_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _GET_FILESIZE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	file = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	
	//printf("file is %s\n",file);
	if(file){
		size=get_file_size(file)/1024;
		if(size<1)
			size=1;
		//printf("the size is %d\n",size);
	}
		/*int fd=open(file,O_RDONLY|O_LARGEFILE);
		
		if(fd==-1)
		{
			printf("dio_open, open error\n");
			fprintf(stderr,"function:%s line:%d strerror:%s\n",__FUNCTION__,__LINE__,strerror(errno));
		}
		else{	
			size = lseek64(fd, 0, SEEK_END);
		
			printf("the size1111======%d\n",size);
			size=size/1024;
			close(fd);
		}
	}
		fp = fopen(file,"r");
		printf("open file!\n");
		if(fp != NULL){
			
			printf("open file OK!\n");
			if(fseek(fp, 0, SEEK_END)==0){
				size = ftell(fp);
				printf("the size is %d\n",size);
			}
			fclose(fp);
		}
		else{
			fprintf(stderr,"function:%s line:%d strerror:%s\n",__FUNCTION__,__LINE__,strerror(errno));
		}
	}
	*/
	
_GET_FILESIZE_NEW_OUT:
	Swfext_PutNumber(size);
	SWFEXT_FUNC_END();	
}

/**
* prototype should be "getFileTimeNew(int handle,int index)"
*/
static int fe_get_file_time_new(void * handle)
{
	char * file;
	int index;
	int flhandle;
	struct stat filestat;
	char *timestr;
	struct stat fileInfo;
	struct tm mtime;
	SWFEXT_FUNC_BEGIN(handle);
	memset(time_string,0,TIME_STR_BUF);
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto fe_get_file_time_new_out;
	}
	if(multi_filelist_handle[flhandle].free){
		goto fe_get_file_time_new_out;
	}
	
	index = Swfext_GetNumber();
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	file = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	if(file){
	/*	if(stat(file,&filestat)==0){
			timestr = ctime(&filestat.st_mtime);
			memcpy(time_string,timestr,strlen(timestr));
		}
	*/
		
		if(stat(file, &fileInfo)==0){
		if(gmtime_r(&fileInfo.st_mtime,&mtime)){
				sprintf(time_string,"%d-%02d-%02d %02d:%02d",mtime.tm_year+1900,mtime.tm_mon+1,\
						mtime.tm_mday,mtime.tm_hour,mtime.tm_min);
			}
		else{
			strncpy(time_string ,strdup("1900-01-01 00:00"),32);
		}
			
		}		
	}
fe_get_file_time_new_out:
	//printf("%s,%d:time is %s\n",__FILE__,__LINE__,time_string);
	Swfext_PutString(time_string);
	SWFEXT_FUNC_END();	
}

/**
* prototype should be "copyFileNew(int handle,int index[,char *dest])"
*/
static int fe_copy_file_new(void * handle)
{
	char *dest;
	int index;
	int flhandle;
	int err=ASFS_ERR_UNKOWN,err1;
	int n;
	char *file;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _COPY_FILE_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _COPY_FILE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	dest=Swfext_GetString();
	strcpy(pathBuf, dest);//add by charles
	dest = pathBuf;
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	file = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	memset(TmpBuf,0,MNAVI_MAX_NAME_LEN);
	if(_get_dest_file(file,dest,TmpBuf)==0){
		err = ASFS_ERR_UNKOWN;
		goto _COPY_FILE_NEW_OUT;
	}
	/**
	err = access(TmpBuf,F_OK);
	if(err == -1){
		err = file_opt(MNAVI_CMD_FILE_COPY,dest,file);
	}
	else
		err = ASFS_ERR_FileExist;
	**/
	err = access(TmpBuf,F_OK);
	if(err == -1){
		if(file_ops_start_work(MNAVI_CMD_FILE_COPY,dest,file)==0){
			err = ASFS_ERR_OK;
		}
		else
			err = ASFS_ERR_UNKOWN;
	}
	else
		err= ASFS_ERR_FileExist;
_COPY_FILE_NEW_OUT:
	
	Swfext_PutNumber(err);
	SWFEXT_FUNC_END();	
}

static int fe_overlap_copy_new(void *handle)
{
	//SWFEXT_FUNC_BEGIN(handle);//delete by charles, If has it will get parm error

	int index;
	int err;
	char *file=NULL;
	int rtn=ASFS_ERR_UNKOWN;
	int flhandle;
	char *dest=NULL;
	int ret=ASFS_ERR_UNKOWN;
	int type=0;
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it] */
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _OVERLAP_COPY_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _OVERLAP_COPY_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	dest=Swfext_GetString();
	if(NULL != dest){
		strcpy(TmpBuf, dest);//add by charles
		dest = TmpBuf;
	}else{
		printf("[%s] dest is null\n", __func__);
		rtn = ASFS_ERR_UNKOWN;
		goto _OVERLAP_COPY_NEW_OUT;
	}
	
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	file = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	type = ui_filelist_get_cur_filetype(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	
	printf("%s,%d:srcFile=%s,des=%s, Type=%d\n",__FILE__,__LINE__,file,dest,type);
	if(NULL == file){
		rtn = ASFS_ERR_UNKOWN;
		goto _OVERLAP_COPY_NEW_OUT;
	}
	if(type==FT_DIR)
		rtn = file_ops_start_work(MNAVI_CMD_DIR_OVERLAP,dest,file);
	else
		rtn = file_ops_start_work(MNAVI_CMD_FILE_OVERLAP,dest,file);
	if(rtn ==0){
		rtn = ASFS_ERR_OK;
	}
	else
		rtn = ASFS_ERR_UNKOWN;

_OVERLAP_COPY_NEW_OUT:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();

	//Swfext_PutNumber(err);//delete by charles
	//SWFEXT_FUNC_END();	
}

static int checkFileHandle(int flhandle)
{
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		return -1;
	}
	if(multi_filelist_handle[flhandle].free){
		return -1;
	}
	else
		return 0;
}

static int fe_enter_dir_new(void* handle)
{
	int flhandle=-1,index;
	int rtn=-1;
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(checkFileHandle(flhandle)==-1)
		goto ENTER_DIR_OUT;
	
	index = Swfext_GetNumber();

	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	if(ui_filelist_enter_dir(&multi_filelist_handle[flhandle].filelist,index)==0){
		printf("[ERROR]%s,%d:Enter Dir Error\n",__func__,__LINE__);
		rtn = -1;
	}
	else
		rtn =0;
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
ENTER_DIR_OUT:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}

static int fe_exit_dir_new(void*handle)
{
	int rtn=-1,flhandle=-1;
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(checkFileHandle(flhandle)==-1)
		goto EXIT_DIR_OUT;

	printf("%s,%d:flhandle=0x%x\n",__FILE__,__LINE__,flhandle);
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	rtn = ui_filelist_esc_dir(&multi_filelist_handle[flhandle].filelist);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	
EXIT_DIR_OUT:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}


static int fe_get_filetype_new(void *handle)
{
	int rtn=-1,flhandle=-1,index,type=-1;
	SWFEXT_FUNC_BEGIN(handle);	

	flhandle=Swfext_GetNumber();
	if(checkFileHandle(flhandle)==-1)
		goto GET_FILETYPE_OUT;
	
	index = Swfext_GetNumber();
	//printf("%s,%d:flhandle=0x%x,index=%d\n",__FILE__,__LINE__,flhandle,index);
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	type = ui_filelist_get_cur_filetype(&multi_filelist_handle[flhandle].filelist, index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
	
GET_FILETYPE_OUT:
	Swfext_PutNumber(type);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "deleteFileNew(int handle,int index)"
*/
static int fe_delete_file_new(void * handle)
{
	int index;
	int flhandle;
	char* file;
	int err,ret=ASFS_ERR_UNKOWN;
	mnavi_listinfo_para_t listinfo;
	
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _DELETE_FILE_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _DELETE_FILE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	file = ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	//printf("[idx=%d]filename del=%s\n",index,file);
	listinfo.file_idx= mnavi_viewidx_2_fileidx(multi_filelist_handle[flhandle].filelist.fsel,index);
	//listinfo.file_sort = file_sort; //需要上层传入 此处写为默认值
	listinfo.file_sort = MNAVI_FS_ALL; //需要上层传入 此处写为默认值

	if(mnavi_get_file_info(multi_filelist_handle[flhandle].filelist.fsel,&listinfo)==0){
		printf("delete fileidx=%d,filepath=%s\n",index,listinfo.file_path);
		ret = file_opt(MNAVI_CMD_FILE_DEL,NULL,listinfo.file_path);
		//printf("%s,%d:Call Del File=%s\n",__FILE__,__LINE__,file);
		mnavi_delete_file(multi_filelist_handle[flhandle].filelist.fsel,&listinfo);
	}
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
_DELETE_FILE_NEW_OUT:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


static int fe_del_dir_new(void*handle)
{
	int ret=ASFS_ERR_UNKOWN;
	int flhandle,index;
	char *dir;
	SWFEXT_FUNC_BEGIN(handle);
	#if 0
	flhandle=Swfext_GetNumber();
	index = Swfext_GetNumber();
	
	dir=ui_filelist_get_cur_filepath(&multi_filelist_handle[flhandle].filelist,index);
	printf("%s,%d:Call Del Dir=%s\n",__FILE__,__LINE__,dir);
	if(file_opt(MNAVI_CMD_DIR_DEL,NULL,dir)==0){
		ret = ASFS_ERR_OK;
	}
	else
		ret = ASFS_ERR_UNKOWN;
	if(ret==ASFS_ERR_OK){
		ui_filelist_refresh_list(&multi_filelist_handle[flhandle].filelist);
	}
	#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int fe_copy_dir_new(void *handle)
{
	int ret=ASFS_ERR_UNKOWN;
	int flhandle,index;
	SWFEXT_FUNC_BEGIN(handle);
	#if 0
	flhandle=Swfext_GetNumber();
	index = Swfext_GetNumber();

	
	#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int fe_fresh_filelist_new(void *handle)
{
	int ret=ASFS_ERR_UNKOWN;
	int flhandle;
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	flhandle=Swfext_GetNumber();
	index = Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _FRESH_LIST_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _FRESH_LIST_NEW_OUT;
	}
	fe_change_work_path(&multi_filelist_handle[flhandle].filelist);
	ui_filelist_refresh_list(&multi_filelist_handle[flhandle].filelist,index);
	fe_save_work_path(&multi_filelist_handle[flhandle].filelist);
_FRESH_LIST_NEW_OUT:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


static int fe_attach_sortview_new(void *handle)
{
	int flhandle;
	int filesort=MNAVI_FS_ALL;
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _ATTACH_SORTVIEW_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _ATTACH_SORTVIEW_NEW_OUT;
	}
	//filesort = Swfext_GetNumber();
	ret = ui_filelist_creat_sortview(&multi_filelist_handle[flhandle].filelist,filesort);
_ATTACH_SORTVIEW_NEW_OUT:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}


static int fe_attach_sortview_new_by_keyword(void *handle)
{
	int flhandle;
	int filesort=MNAVI_FS_ALL;
	int ret = 0;
	int keyword=0;
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	keyword=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _ATTACH_SORTVIEW_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _ATTACH_SORTVIEW_NEW_OUT;
	}
	ret = ui_filelist_creat_sortview_by_keyword(&multi_filelist_handle[flhandle].filelist,filesort,keyword);
_ATTACH_SORTVIEW_NEW_OUT:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int fe_detach_sortview_new(void *handle)
{
	int flhandle;
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);

	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _DETACH_SORTVIEW_NEW_OUT;
	}
	if(multi_filelist_handle[flhandle].free){
		goto _DETACH_SORTVIEW_NEW_OUT;
	}
	ret = ui_filelist_delete_sortview(&multi_filelist_handle[flhandle].filelist);
_DETACH_SORTVIEW_NEW_OUT:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int fe_put_search_flag(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	
	int search_flag=Swfext_GetNumber();
	ui_filelist_search_flag(search_flag);
		
	SWFEXT_FUNC_END();	
}

static int fe_stop_deamon_search_task(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	int flhandle;
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
			goto _DETACH_SORTVIEW_NEW_OUT;
		}
	if(multi_filelist_handle[flhandle].free){
			goto _DETACH_SORTVIEW_NEW_OUT;
		}
	//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
	ui_stop_search_task(&multi_filelist_handle[flhandle].filelist);
_DETACH_SORTVIEW_NEW_OUT:
	
	SWFEXT_FUNC_END();	
}

void fe_exception_release()
{
	int i;
	
	if(swf_ext_ui_filelist.fsel!=NULL){
		ui_filelist_exit(&swf_ext_ui_filelist);
		fe_reset_last_flist(&swf_ext_ui_filelist);
	}

	for(i=0;i<SWF_MAX_FILELIST;i++){
		multi_filelist_handle[i].free=1;
		if(multi_filelist_handle[i].filelist.fsel!=NULL){
			ui_filelist_exit(&multi_filelist_handle[i].filelist);
			fe_reset_last_flist(&multi_filelist_handle[i].filelist);
		}
	}
	
}

/*file list api for fui  */
 int ezcast_fui_get_total()
{
	int totalnum=0;
	if(__check_filelist_valid()){
		totalnum = ui_filelist_get_filetotal_num(&swf_ext_ui_filelist);
		//totalnum=mnavi_get_file_total(swf_ext_ui_filelist.fsel,MNAVI_FS_ALL);
		//printf("totalnum1=======%d\n",totalnum);
	}
	return totalnum;
}
int ezcast_fui_release_path()
{
	   
	 printf("%s,%d:Call Release Path !\n",__FILE__,__LINE__);
	 if(__check_filelist_valid()){
		 ui_filelist_exit(&swf_ext_ui_filelist);
 
		 swf_ext_ui_filelist.fsel = NULL;
		 fe_reset_last_flist(&swf_ext_ui_filelist);
	 }
 }

 int ezcast_fui_set_path(char *file_path,char *file_tpye)
{

	
	char * path;
	char * filetype;
	int ret=0;
	int n;
	filelist_mode_e ftype;
	int filesort=MNAVI_FS_ALL;
	_fe_filelist_init();
	path = file_path;	

	filetype =file_tpye;
	if(__check_filelist_valid()){
		ui_filelist_exit(&swf_ext_ui_filelist);

		swf_ext_ui_filelist.fsel = NULL;
	}
	
	if(strcmp(filetype,"") ==0){
		ftype = FLIST_MODE_BROWSE_ALL;
	}
	else{
		ftype = FLIST_MODE_BROWSE_FILE;
	}
	
	fe_save_work_path(last_flist);
	ui_filelist_init(&swf_ext_ui_filelist, path, filetype, 0, ftype);
	//printf("go to 1\n");
	//ret=ui_filelist_dir_file_apart(&swf_ext_ui_filelist,filesort);
	printf("ret is %d\n",ret);
	fe_change_work_path(&swf_ext_ui_filelist);
	//Swfext_PutNumber(ret);
}

int ezcast_fui_get_filepath(int file_index,char *filepath)
{
	static char * file=NULL;
	int index;
	index = file_index;
	if(__check_filelist_valid()){
		fe_change_work_path(&swf_ext_ui_filelist);
		file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
		fe_save_work_path(&swf_ext_ui_filelist);
	}
	strcpy(filepath,file);
}
//#if EZMUSIC_ENABLE
#if EZMUSIC_ENABLE || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
int  filelist_close(int searchDone,int fileHandle)
{
    
	if(searchDone==0)
	{
		printf("stopDeamonTask");
		if(fileHandle<0 || fileHandle>=SWF_MAX_FILELIST)
		{
			searchDone=1;
			return 0;
		}
		if(multi_filelist_handle[fileHandle].free)
		{
			searchDone=1;
			return 0;
		}
		ui_stop_search_task(&multi_filelist_handle[fileHandle].filelist);

	}
	if(fileHandle<0 || fileHandle>=SWF_MAX_FILELIST)
	{
					return 0;
	}

		if(multi_filelist_handle[fileHandle].free)
		{
			if(multi_filelist_handle[fileHandle].filelist.fsel!=NULL)
			{
				ui_filelist_exit(&multi_filelist_handle[fileHandle].filelist);

				fe_reset_last_flist(&multi_filelist_handle[fileHandle].filelist);
			
			}
			return 0;
		}
		else
		{
			if(multi_filelist_handle[fileHandle].filelist.fsel!=NULL)
			{
				ui_filelist_exit(&multi_filelist_handle[fileHandle].filelist);

				fe_reset_last_flist(&multi_filelist_handle[fileHandle].filelist);
			
			}
		     multi_filelist_handle[fileHandle].free=1;
	     }
			
	searchDone=1;
	return 0;

}
static void*  __search_thread(void * arg)
{

	printf("[%s] start!pid = %d\n", __func__, pthread_self());
	int i=0;
	int fileHandle=-1;
	int mode=0;
	int scan;
	int list_rtn=0;
	int searchDone=0;
	int stat=NO_SENSE;
	int total=0;
	int output_total=0;
	char *path_url;
	char path[128]="";
	fileHandle=_fe_get_free_list();
	if(fileHandle==-1){
		goto out;
	}
	mode=FLIST_MODE_BROWSE_FILE;//Swfext_GetNumber();
	scan=1;//Swfext_GetNumber();
	fe_save_work_path(last_flist);
	list_rtn = ui_filelist_init_deamon(&multi_filelist_handle[fileHandle].filelist,  currentMedia_path, currentMedia_type, scan,mode);
	fe_change_work_path(&multi_filelist_handle[fileHandle].filelist);
	if(list_rtn==-1)
	{
		multi_filelist_handle[fileHandle].free=1;
		fileHandle = -1;
	}

	while(1)
	{
		
		if(fileHandle<0 || fileHandle>=SWF_MAX_FILELIST){
			break;
		}
		if(multi_filelist_handle[fileHandle].free){
			break;
		}

		stat = ui_filelist_check_scan_task(&multi_filelist_handle[fileHandle].filelist);
		if(stat==FIND_NOFILE)
		{
			output_total=0;
			searchDone=1;
			filelist_close(searchDone,fileHandle);
			printf("stat=%d\n",stat);		
			break;
		}
		else
		{
			if(fileHandle<0 || fileHandle>=SWF_MAX_FILELIST)
			{
				break;
			}
			if(multi_filelist_handle[fileHandle].free){
				break;
			}	
			total = ui_filelist_get_filetotal_num(&multi_filelist_handle[fileHandle].filelist);
	
			if(total!=0)
			{
				output_total=total;
			}
		}
		if(stat==FIND_COMPLETE)
		{  // the scanning done
			output_total=total;
			printf("output_total=%d\n",output_total);
			if(output_total==0)
				filelist_close(searchDone,fileHandle);
			 searchDone=1;
			 
			break;


		}
	
	}
	for(i=0;i<output_total;i++)
	{
		path_url= ui_filelist_get_cur_filepath(&multi_filelist_handle[fileHandle].filelist,i);
		if(strlen(path_url)>23)
		{
			char *locate=NULL;
			locate=path_url+23;
/* Mos: This url only affect wifi_ezdisplay local request,
 * Web airdisk.html will replace absolute path to relarive path, so would not affect 
 * so it'should be fine with 127.0.0.1
 * to Fix Pro lan plus, Airdisk local auto play function failed issue
 * cause Pro lan plus does not have soft AP to bind ip 192.168.168.1 */

            sprintf(path, "%s", "http://127.0.0.1/");
			strcat(path,locate);
			Write_MusicURL_Array(i,path);


		}
		/*
		if(i>500)
		{
			output_total=i;
			break;
		}*/
		
	}
	if(output_total)
	{
		 Write_MusicURL_TOJson(output_total);
		 filelist_close(searchDone,fileHandle);

	}
	searchDone=0;

out:
	pthread_exit(NULL);
	return NULL;
	
}
int fe_search_file_thread()
{
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, __search_thread, NULL);	
}
int fe_set_serch_path_type(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char *path=NULL;
	char *type=NULL;
	path=Swfext_GetString();
	type=Swfext_GetString();
	printf("path=%s type=%s\n",type,path);
	sprintf(currentMedia_path,"%s",path);
	sprintf(currentMedia_type,"%s",type);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
int fe_creat_search_file_thread(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	fe_search_file_thread();
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}


int fe_creat_default_info_file(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Write_default_info_TOJson();
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

#endif
int swfext_filelist_register(void)
{
	_fe_filelist_init();
	
	SWFEXT_REGISTER("fl_setPath", fe_set_path);
	SWFEXT_REGISTER("fl_releasePath", fe_release_path);
	SWFEXT_REGISTER("fl_getPath", fe_get_path);
	SWFEXT_REGISTER("fl_getTotal", fe_get_total);
	SWFEXT_REGISTER("fl_getFileName", fe_get_filename);
	SWFEXT_REGISTER("fl_getFileType", fe_get_filetype);
	SWFEXT_REGISTER("fl_getFilePath", fe_get_filepath);
	SWFEXT_REGISTER("fl_getFileTime", fe_get_filetime_modified);
	SWFEXT_REGISTER("fl_getFileSize", fe_get_filesize);
	SWFEXT_REGISTER("fl_getSubDirTotal", fe_get_dir_total_item);
	SWFEXT_REGISTER("fl_enterDir", fe_enter_dir);
	SWFEXT_REGISTER("fl_exitDir", fe_exit_dir);
	SWFEXT_REGISTER("fl_copyFile", fe_copy_file);
	SWFEXT_REGISTER("fl_deleteFile", fe_delete_file);
	SWFEXT_REGISTER("fl_copyDir",fe_copy_dir);
	SWFEXT_REGISTER("fl_deleteDir",fe_delete_dir);
	SWFEXT_REGISTER("fl_overlapCopy",fe_overlap_copy);
	SWFEXT_REGISTER("fl_freshFileList",fe_fresh_flist);
	SWFEXT_REGISTER("fl_attachSortview",fe_attach_sortview);
	SWFEXT_REGISTER("fl_detachSortview",fe_detach_sortview);

	/**
		newly added for file search
	*/
	SWFEXT_REGISTER("fl_setPathNew", fe_set_path_new);
	SWFEXT_REGISTER("fl_getScanTaskStatNew", fe_get_scan_task_stat_new);
	SWFEXT_REGISTER("fl_getPathNew", fe_get_path_new);
	SWFEXT_REGISTER("fl_releasePathNew", fe_release_path_new);
	SWFEXT_REGISTER("fl_getTotalNew", fe_get_total_new);
	SWFEXT_REGISTER("fl_getLongnameNew", fe_get_longname_new);
	SWFEXT_REGISTER("fl_getFileSizeNew", fe_get_file_size_new);
	SWFEXT_REGISTER("fl_getFileTimeNew", fe_get_file_time_new);
	SWFEXT_REGISTER("fl_copyFileNew", fe_copy_file_new);
	SWFEXT_REGISTER("fl_overlapCopyNew",fe_overlap_copy_new);
	SWFEXT_REGISTER("fl_deleteFileNew", fe_delete_file_new);
	SWFEXT_REGISTER("fl_enterDirNew", fe_enter_dir_new);
	SWFEXT_REGISTER("fl_eixtDirNew", fe_exit_dir_new);
	SWFEXT_REGISTER("fl_getFileTypeNew", fe_get_filetype_new);
	
	SWFEXT_REGISTER("fl_get_process",fe_get_process);
	SWFEXT_REGISTER("fl_get_status",fe_get_status);
	SWFEXT_REGISTER("fl_stop_work",fe_stop_work);
	SWFEXT_REGISTER("fl_attachSortviewNew",fe_attach_sortview_new);
	SWFEXT_REGISTER("fl_detachSortviewNew",fe_detach_sortview_new);
	SWFEXT_REGISTER("fl_freshFileListNew",fe_fresh_filelist_new);
	SWFEXT_REGISTER("fl_putsearchflag",fe_put_search_flag);

	
	SWFEXT_REGISTER("fl_attachSortviewNewbykey",fe_attach_sortview_new_by_keyword);
	
	SWFEXT_REGISTER("fl_setPathNewdeamon",fe_set_path_new_deamon);
	
	SWFEXT_REGISTER("fl_stoptaskdeamon",fe_stop_deamon_search_task);
	//#if EZMUSIC_ENABLE
	#if EZMUSIC_ENABLE || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	SWFEXT_REGISTER("fl_set_serch_path_type",fe_set_serch_path_type);

	SWFEXT_REGISTER("fl_creat_search_file_thread",fe_creat_search_file_thread);
	SWFEXT_REGISTER("fl_creat_default_info_file",fe_creat_default_info_file);
	#endif
	
	return 0;
}

