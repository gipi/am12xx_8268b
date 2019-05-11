#include "file_list.h"
#include "am_types.h"
#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_conf.h"
#include <semaphore.h>
//#define _FILELIST_DEBUG

#ifdef _FILELIST_DEBUG
#define flist_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define flist_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define flist_info(fmt,arg...) do{}while(0);
#define flist_err(fmt,arg...) do{}while(0);
#endif

static char filemgr_ext[] = FILEMGR_FILE_TYPE;
static char ebook_ext[] = EBOOK_FILE_TYPE;
static char music_ext[] = MUSIC_FILE_TYPE;
static char video_ext[] = VIDEO_FILE_TYPE;
static char photo_ext[] = IMAGE_EXT;



enum{
	FILELIST_GET_LONGNAME=0,
	FILELIST_GET_FULLPATH=1,
	FILELIST_GET_FILETYPE=2,
	FILELIST_GET_SHORTNAME=3,
};

sem_t *big_filelist_semaphore = NULL;
sem_t *small_filelist_semaphore = NULL;

#if 1
static int thread_sleep(unsigned int millsec){
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
	return 0;
}

static int check_filelist_vaild(UI_FILELIST * filelist)
{
	if(filelist==NULL){
		printf("%s,%d:Crazy FileList Handle=NULL\n",__FILE__,__LINE__);
		return 0;
	}
	else
		return 1;
}

static void _flist_sem_wait(sem_t * sem)
{
	int err;

__PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto __PEND_REWAIT;	
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return;
	
}
static void _flist_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}


static char*  unicode_strcpy(char* dest,const char* src)
{
	char *tmp = dest;

	while(1){
		*dest = *src;
		*(dest+1) = *(src+1);
		if((*dest == 0)&&(*(dest+1) == 0)){
			break;
		}
		dest+=2;
		src+=2;
	}
	return tmp;
}

static unsigned int clear_list_node(void)
{
	return 0;
}

static int get_ext_name(char * file_name,char* ext_name,int ext_name_size)
{
	int ret=0;
	char *ptemp = NULL;
	if((strlen(file_name))<=0)
		return -1;
	if(strrchr(file_name,'.')){
		ptemp = (strrchr(file_name,'.')+1);
		strcpy(ext_name,ptemp);
		ret = (strlen(file_name)-strlen(ext_name));
		flist_info("find ext name ====%s,return%d",ext_name,ret);
	}
	return ret;
}


char  * get_cur_ext_name(char  * needle,char  *hay)
{
	int  i=0,j=0;

	if(hay==NULL)
	{
		needle[0]='\0';
		return NULL;
	}
	else
	{
		while(*hay == ' ')
		{
			hay++;
		}
	}
	if(*hay == '\0')
	{
		strcpy(needle,"   ");
		return hay;
	}
	while(1)
	{
		needle[j] = hay[i++];
		if(needle[j] == ' ')
		{
			needle[j]='\0';
			return hay;
		}else if(needle[j] == '\0')
		{
			return hay;
		} 
		if(j<MNAVI_EXTENTION_LEN)
		{
			j++;
		}
	}
}

char  * get_next_ext_name(char  * needle,char  *hay)
{
		int  i=0,j=0;
        //printf("%s\n",hay);
        if(hay==NULL){
               needle[0]='\0';
               return NULL;
        }else{
               while(*hay == ' '){
                     hay++;
               }
       }

	 while((*hay !=' ')&&(*hay != '\0')){
                 hay++;
        }
        while(*hay == ' '){
                 hay++;
       }
		 //printf("%s\n",hay);
       if(*hay=='\0'){
              return NULL;
       }
       
      while(1){
	   
              needle[j] = hay[i++];
			  //printf("%c\n",needle[j]);
              if(needle[j] == ' '){
                     needle[j]='\0';
                     return hay;
               }else if(needle[j] == '\0'){
                     return hay;
               }
               if(j<MNAVI_EXTENTION_LEN){
                    j++;
               }
      }

}

static int  get_filetype(char *filename)
{
	int rtn=FT_FILE;
	char ext[MNAVI_EXTENTION_LEN+1];
	
	/*find ext of the file*/
	if(filename == NULL)
			goto out;
	if( get_ext_name(filename,ext,MNAVI_EXTENTION_LEN)<=1){
		rtn = FT_FILE;
		goto out;
	}
	if(find_ext(video_ext,ext)){
		rtn = FT_VIDEO;
		goto out;
	}

	if(find_ext(music_ext,ext)){
		rtn = FT_MUSIC;
		goto out;
	}

	if(find_ext(photo_ext,ext)){
		rtn = FT_IMAGE;
		goto out;
	}

out:

	return rtn;

}



int  find_ext(char  * exts,	char  *ext )
{
	char  * p_cur_ext;
	char cur_ext[MNAVI_EXTENTION_LEN+1];

	p_cur_ext = get_cur_ext_name(cur_ext,exts);
	if(!strcmp(cur_ext,"   "))
	{
		return TRUE;
	}
	while(1)
	{
		if(!strncasecmp(ext,cur_ext,MNAVI_EXTENTION_LEN))
		{
			return TRUE;
		}
		p_cur_ext = get_next_ext_name(cur_ext,p_cur_ext);
		if(p_cur_ext==NULL)
		{
			return FALSE;
		}	
	}
}


#endif

#if 1
/**
@brief filelist initialization
@param[out] ui_filelist 	: a pointer to the UI_FILELIST returned from mnavi where the list information be stored 
@param[in] disk		: not used 
@param[in] base_path 	: which dir will be scanned as the root path, such as /mnt/udisk/ or /mnt/udisk/a/
@param[in] file_type	: the extension of the file which will be scanned, if the list_type="   "(3 space), it will list all the files
@param[in] scan		: tell the mnavi whether the scan process need be created
@param[in] list_type	: tell the mnavi which property of scanning see FILST_TYPE_t
@return
	- -1	: failed
	- 0	: succeed
@see FILST_TYPE_t
**/
EXPORT_SYMBOL
int ui_filelist_init(UI_FILELIST * ui_filelist,char * base_path,char * file_type,char scan,filelist_mode_e  list_type)
{										
	memset(ui_filelist,0,sizeof(UI_FILELIST));
	ui_filelist->show_dir = list_type;	
	printf("base_path=====%s\n",base_path);
	printf("scan========%d\n",scan);
	
	if( FLIST_MODE_BROWSE_ALL == ui_filelist->show_dir ){
		mnavi_attr_t attr;
		memset(&attr,0,sizeof(mnavi_attr_t));
		attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_SEARCH_CHILD|MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("MNAVI_ATTR_SEARCH_CHILD=====%d\n",MNAVI_ATTR_SEARCH_CHILD);
		
		//printf("MNAVI_ATTR_EXCLUDE_EMPTY_DIR=====%d\n",MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("attr.mnavi_prop=====%d\n",attr.mnavi_prop);
		attr.mnvai_lsmode = MNAVI_LM_BROWSE_ALL;
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, &attr);
		
		//ui_filelist->fsel->dir_info.dir_search_attr = 0;
	}
	else{
		flist_info("use default attr to open fsel\n");
		char pathbuf[512];
		strcpy(pathbuf,base_path);
		flist_info("pathbuf===%s   basepath===%s\n",pathbuf,base_path);
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, NULL);
		//ui_filelist->fsel->dir_info.dir_search_attr = 1;
	}

	if(ui_filelist->fsel ==NULL){
		flist_err("open fsel failed");
		return -1;
	}

	if(ui_filelist->fsel){
		mnavi_start_scan_dir(ui_filelist->fsel);
		flist_info("scan completed");
	}
	flist_info("filelist initialized");
	return 0;
}
/**
@brief filelist initialization
@param[out] ui_filelist 	: a pointer to the UI_FILELIST returned from mnavi where the list information be stored 
@param[in] disk		: not used 
@param[in] base_path 	: which dir will be scanned as the root path, such as /mnt/udisk/ or /mnt/udisk/a/
@param[in] file_type	: the extension of the file which will be scanned, if the list_type="   "(3 space), it will list all the files
@param[in] scan		: tell the mnavi whether the scan process need be created
@param[in] list_type	: tell the mnavi which property of scanning see FILST_TYPE_t
@return
	- -1	: failed
	- 0	: succeed
@see FILST_TYPE_t
**/
EXPORT_SYMBOL
int ui_filelist_init_deamon(UI_FILELIST * ui_filelist,char * base_path,char * file_type,char scan,filelist_mode_e  list_type)
{										
	memset(ui_filelist,0,sizeof(UI_FILELIST));
	ui_filelist->show_dir = list_type;	
	printf("base_path=====%s\n",base_path);
	printf("scan========%d\n",scan);
	
	if( FLIST_MODE_BROWSE_ALL == ui_filelist->show_dir ){
		mnavi_attr_t attr;
		memset(&attr,0,sizeof(mnavi_attr_t));
		attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_SEARCH_CHILD|MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("MNAVI_ATTR_SEARCH_CHILD=====%d\n",MNAVI_ATTR_SEARCH_CHILD);
		
		//printf("MNAVI_ATTR_EXCLUDE_EMPTY_DIR=====%d\n",MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("attr.mnavi_prop=====%d\n",attr.mnavi_prop);
		attr.mnvai_lsmode = MNAVI_LM_BROWSE_ALL;
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, &attr);
		
		//ui_filelist->fsel->dir_info.dir_search_attr = 0;
	}
	else{
		flist_info("use default attr to open fsel\n");
		char pathbuf[512];
		strcpy(pathbuf,base_path);
		flist_info("pathbuf===%s   basepath===%s\n",pathbuf,base_path);
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, NULL);
		//ui_filelist->fsel->dir_info.dir_search_attr = 1;
	}

	if(ui_filelist->fsel ==NULL){
		flist_err("open fsel failed");
		return -1;
	}

	if(ui_filelist->fsel){
		mnavi_start_scan_dir_deamon(ui_filelist->fsel);
		flist_info("scan completed");
	}
	flist_info("filelist initialized");
	return 0;
}

/*
EXPORT_SYMBOL
int ui_filelist_init_for_wifi(UI_FILELIST * ui_filelist,char * base_path,char * file_type,char scan,filelist_mode_e  list_type)
{										
	memset(ui_filelist,0,sizeof(UI_FILELIST));
	ui_filelist->show_dir = list_type;	
	printf("base_path=====%s\n",base_path);
	printf("scan========%d\n",scan);
	
	if( FLIST_MODE_BROWSE_ALL == ui_filelist->show_dir ){
		mnavi_attr_t attr;
		memset(&attr,0,sizeof(mnavi_attr_t));
		attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_SEARCH_CHILD|MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//attr.mnavi_prop = (unsigned char)(MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("MNAVI_ATTR_SEARCH_CHILD=====%d\n",MNAVI_ATTR_SEARCH_CHILD);
		
		//printf("MNAVI_ATTR_EXCLUDE_EMPTY_DIR=====%d\n",MNAVI_ATTR_EXCLUDE_EMPTY_DIR);
		//printf("attr.mnavi_prop=====%d\n",attr.mnavi_prop);
		
		attr.mnvai_lsmode = MNAVI_LM_BROWSE_ALL;
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, &attr);
	}
	else{
		flist_info("use default attr to open fsel\n");
		char pathbuf[512];
		strcpy(pathbuf,base_path);
		flist_info("pathbuf===%s   basepath===%s\n",pathbuf,base_path);
		ui_filelist->fsel = mnavi_fsel_open(base_path, file_type, NULL);
	}

	if(ui_filelist->fsel ==NULL){
		flist_err("open fsel failed");
		return -1;
	}

	if(ui_filelist->fsel){
		mnavi_start_scan_dir_for_wifi(ui_filelist->fsel);
		flist_info("scan completed");
	}
	flist_info("filelist initialized");
	return 0;
}
*/
/**************************************************************************
* name : ui_filelist_exit
* descp: 关闭flist
* intput:  
*          
* output: 0-
* by:     zd
* note:   
**************************************************************************/
EXPORT_SYMBOL
int ui_filelist_exit(UI_FILELIST * ui_filelist)
{

	if(check_filelist_vaild(ui_filelist)==0)
		return -1;
	if(ui_filelist->fsel){
		mnavi_fsel_exit(ui_filelist->fsel);
		ui_filelist->fsel =NULL;
	}
	return 0;
}


/**************************************************************************
* name : ui_filelist_check_scan_task
* descp: 检测后台扫描任务的状态
* intput:  
*           
* output: 0-
* by:     zd
* note:   1   若扫描结束，会删除扫描任务。
*        2   浏览全盘文件的模式下，如果目录信息还没有包含指定文件
*            则将浏览模式暂时设定为只浏览当前文件下的文件，
*            若函数filelist_check_scan_task 检测到已经包含当前文件，会将浏览模式
*            重新设置为全盘浏览。
**************************************************************************/
EXPORT_SYMBOL
scan_status_t ui_filelist_check_scan_task(UI_FILELIST * filelist)
{
	int task_staus;
	
	if(check_filelist_vaild(filelist)==0)
	return NO_SENSE;

	task_staus = mnavi_get_task_staus(filelist->fsel);
	//fprintf(stderr,"function:%s,task_status====%d\n",__FUNCTION__,task_staus);
	if( MNAVI_TASK_COMPLETED == task_staus )
		return  FIND_COMPLETE;
	else{
		return FIND_FIRST_FILE;
	} 
}


/**
 * ui_filelist_refresh_list - refresh filelist after file or dir deleted.
 * @filelist: current filelist
 * @req: index of the file or dir
 *
 * ui_filelist_refresh_list( )-after file or dir deleted,use this functions to 
 * refresh  filelist.
 */
EXPORT_SYMBOL
int ui_filelist_refresh_list(UI_FILELIST * filelist,int idx_del)
{
	static UI_FILELIST *filelist_temp;
	static int init_filelist;
	mnavi_listinfo_para_t listinfo;
	listinfo.file_sort = MNAVI_FS_ALL;
	listinfo.file_idx= mnavi_viewidx_2_fileidx(filelist->fsel,idx_del);
	int ret1=-1;
	int ret2=-1;
	int ret3=-1;
	int ret=-1;
	
	if(mnavi_get_file_info(filelist->fsel,&listinfo)==0){
		//printf("mnavi_get_file_info OK!\n");
		mnavi_delete_file(filelist->fsel,&listinfo);
		//printf("mnavi_delete_file OK!\n");
		ret = 0;
	}
	//printf("mnavi_delete_file OK!\n");
	/*
	if((init_filelist==1)&&(filelist_temp->fsel!=NULL))
	{
		//printf("init_filelist OK!\n");
		UI_FILELIST *filelist_temp_tmf;
		filelist_temp_tmf=filelist_temp;
		mnavi_detach_sortview(filelist_temp_tmf->fsel);
		//printf("mnavi_detach_sortview OK\n");
		ret2=0;
	}
	if(filelist->fsel!=NULL){
		
		//printf("mnavi_attach_sortview ready!\n");
		 mnavi_attach_sortview(filelist->fsel, MNAVI_FS_ALL);
		//printf("mnavi_attach_sortview OK!\n");
		filelist_temp=filelist;
		//printf("filelist_temp OK!\n");
		init_filelist=1;
		ret3=0;
	}
	if(ret1==0&&ret2==0&&ret3==0)
		ret=0;
	else
		ret=-1;
	*/
	return ret;
}


/**************************************************************************
* name : ui_filelist_enter_dir
* descp: 进入到下一级目录
* intput:  
*           
* output: 0-
* by:     zd
* note:   
**************************************************************************/
EXPORT_SYMBOL
int  ui_filelist_enter_dir(UI_FILELIST * filelist,int dir_index)
{
	char  * name;
	mnavi_listinfo_para_t listinfo;
	
	if(check_filelist_vaild(filelist)==0){
		return 0;
	}
	//printf("dir_index======%d\n",dir_index);
	listinfo.file_idx= mnavi_viewidx_2_fileidx(filelist->fsel,dir_index);
	//listinfo.file_idx=dir_index;
	//printf("listinfo.file_idx=====%d\n",listinfo.file_idx);
	if(mnavi_get_file_info(filelist->fsel,&listinfo)==0){
			//printf("$$$$$$$$$$$$\n");
			//printf("listinfo.file_type=======%d",listinfo.file_type);
			if(listinfo.file_type==DT_DIR){
				//printf("@@@@@@@\n");
				if(access(listinfo.file_path,F_OK)==0)
					mnavi_cd_child_dir(filelist->fsel,listinfo.file_path);
				else
					return 0 ;
			}
	}
	return 1;
}



/**************************************************************************
* name : filelist_esc_dir
* descp: 返回到上一级目录
* intput:  
*           
* output: 0-
* by:       zd
* note:   
**************************************************************************/
EXPORT_SYMBOL
int ui_filelist_esc_dir(UI_FILELIST * filelist)
{
	if(check_filelist_vaild(filelist)==0)
		return 0;	
	flist_info("filepath=%s\n",filelist->fsel->fileinfo_cache.cur_dir_path);
	mnavi_cd_parent_dir(filelist->fsel);
	flist_info("filepath=%s\n",filelist->fsel->fileinfo_cache.cur_dir_path);
	return 1;
}



/**
@brief set the filelist using the newmode
@param[in] filelist		: 
@param[in] showdir	: the mode to be switched to 
@param[in] pre_activeidx	: the activeidx under the current mode
@return always return 0, the active idx in the new mode will be stored in where the pre_activeidx stored
**/
EXPORT_SYMBOL
int ui_filelist_set_mode(UI_FILELIST * filelist,int showdir,int* pre_activeidx)
{

	filelist_mode_e browse_mode = showdir;
	return mnavi_set_lsmode(filelist->fsel,browse_mode);

}


/**
@brief get the file total num in a specified list
@param[in] filelist		: 
@return the file total num in a specified list
@see ui_filelist_init()
**/
EXPORT_SYMBOL
int  ui_filelist_get_filetotal_num(UI_FILELIST * filelist)
{

	if(check_filelist_vaild(filelist)==0)
		return 0;
	mnavi_fs_e file_sort = MNAVI_FS_ALL;
	
	//printf("fsel == %d filesort == %d\n", filelist->fsel,file_sort);
	filelist->total = mnavi_get_file_total(filelist->fsel,file_sort);
	//printf("total_files===%d\n",filelist->total);
	if(filelist->total < 0)
		return 0;
	else
		return filelist->total;
}



EXPORT_SYMBOL
int  ui_filelist_get_cur_filetype(UI_FILELIST * flist,int index)
{
	int rtn = FT_FILE;
	int p = 0;
	char* filename=NULL;
	if(check_filelist_vaild(flist)==0)
		return 0;	
	mnavi_listinfo_para_t listinfo;
	listinfo.file_sort = MNAVI_FS_ALL;
	listinfo.file_idx= mnavi_viewidx_2_fileidx(flist->fsel,index);
	//printf("listinfo.file_idx=================%d\n",listinfo.file_idx);	
	
	if(mnavi_get_file_info(flist->fsel,&listinfo)==0){
		p = listinfo.file_type;
	}
	filename = ui_filelist_get_longname(flist,index);
	//printf("filename is %s\n",filename);
	if(DT_DIR == p)
		rtn = FT_DIR;
	else{
		rtn = get_filetype(filename);
	}
	//printf("ui_filelist_get_cur_filetype = %d\n",rtn);
	return rtn;
}

/**
@brief get the filepath using the internal buffer
@param[in] filelist		: a pointer which got from ui_filelist_init()
@param[in] index		: the index of the file in the list
@return the point to the buffer where the path be stored, if failed, NULL will be returned
@see ui_filelist_init()
**/
EXPORT_SYMBOL
char * ui_filelist_get_cur_filepath(UI_FILELIST * filelist,int  index)
{
	char* filepath=NULL;
	int fililistTotal = 0;
	mnavi_listinfo_para_t listinfo;
	
	if((index<0)||(filelist == NULL)){
		return NULL;
	}

	if(check_filelist_vaild(filelist)==0)
		return NULL;
	
	listinfo.file_sort = MNAVI_FS_ALL;	
	
	//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	listinfo.file_idx = mnavi_viewidx_2_fileidx(filelist->fsel,index);
	
	//printf("function:%s,line:%d,listinfo.file_idx:%d\n",__FUNCTION__,__LINE__,listinfo.file_idx);
	fililistTotal = mnavi_get_file_total(filelist->fsel,MNAVI_FS_ALL);
	if(listinfo.file_idx > fililistTotal-1){
		
	//	printf("function:%s,line:%d,listinfo.file_idx====%d,fililistTotal ==== %d\n",__FUNCTION__,__LINE__,listinfo.file_idx,fililistTotal);
		listinfo.file_idx = 0;
	}
	if(mnavi_get_file_info(filelist->fsel,&listinfo)==0)
	{
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		filepath = listinfo.file_path;
		
		//printf("function:%s,line:%d,filepath:%s\n",__FUNCTION__,__LINE__,filepath);
		if(filepath)
			return filepath;
	}
	return NULL;
}


EXPORT_SYMBOL
char  *ui_filelist_get_cur_filepath_buf(UI_FILELIST * filelist,int  index,void * buffer)
{	
	return ui_filelist_get_longname(filelist,index);
}


EXPORT_SYMBOL
char  *ui_filelist_get_longname(UI_FILELIST * filelist,	int  index)
{
	if((index<0)||(filelist == NULL)){
		return NULL;
	}
	char* FileName=NULL;
	char *tmp = NULL;
	
	//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	FileName = ui_filelist_get_cur_filepath(filelist, index);
	
	//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	if(FileName){
		
		char *tmp=strrchr(FileName,'/');
		if(tmp)
			tmp++;
		//char *tmp=(strrchr(FileName,'/')+1);
		if(tmp){
			FileName = tmp;
			flist_info("ui_filelist_get_longname===%s",FileName);
		}
	}
	
	//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	return FileName;

}

/**************************************************************************
* name : ui_filelist_get_cur_workpath
* descp: 获取文件选择器的当前工作目录
* intput:  
*           
* output: 0-
* by:     zd
* note: 并不一定是当前显示文件所在的路径，因为filelist 每次会获取
*       一定数量的文件存在buffer中，工作目录是获取的最后一个文件
*        所在的目录。
**************************************************************************/
EXPORT_SYMBOL
char  * ui_filelist_get_cur_workpath(UI_FILELIST * filelist)
{	
	char *filepath=NULL;
	if(check_filelist_vaild(filelist)==0)
		return filepath;
	
	if(filelist->fsel!=NULL){
		filepath = filelist->fsel->fileinfo_cache.cur_dir_path;
	}
	flist_info("currentpath=%s",filepath);
	return filepath;
}


EXPORT_SYMBOL
int ui_filelist_creat_sortview(UI_FILELIST * filelist,mnavi_fs_e filesort)
{	
	fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
	if(filelist->fsel!=NULL){
		return mnavi_attach_sortview(filelist->fsel,  filesort);
	}
	else{
		return -1;	
	}
}
EXPORT_SYMBOL
int ui_filelist_creat_sortview_by_keyword(UI_FILELIST * filelist,mnavi_fs_e filesort,mnavi_sort_keyword_e keyword)
{	
	fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
	if(filelist->fsel!=NULL){
		return mnavi_attach_sortview_by_keyword(filelist->fsel,filesort,keyword);
	}
	else{
		return -1;	
	}
}

EXPORT_SYMBOL
int ui_filelist_dir_file_apart(UI_FILELIST * filelist,mnavi_fs_e filesort)
{	
	if(filelist->fsel!=NULL){
		return mnavi_file_and_dir_apart(filelist->fsel,  filesort);
	}
	else{
		return -1;	
	}
}

EXPORT_SYMBOL
int ui_filelist_delete_sortview(UI_FILELIST * filelist)
{	
	if(filelist->fsel!=NULL){
		return mnavi_detach_sortview(filelist->fsel);
	}
	else{
		return -1;	
	}
}

EXPORT_SYMBOL
void ui_filelist_search_flag(int search_flag){
	mnavi_set_search_dir_flag(search_flag);
}

EXPORT_SYMBOL
void ui_stop_search_task(UI_FILELIST * filelist){
	flist_info("ui_stop_search_task");
	stop_deamon_task(filelist->fsel);
}

#endif
