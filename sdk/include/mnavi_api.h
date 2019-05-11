#ifndef _MNAVI_API_H_
#define _MNAVI_API_H_

#include <am_types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h> 
#include <errno.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ipc_key.h"

#ifdef __cplusplus                     
extern "C" {
#endif






#define mnavi_printf(fmt,arg...) printf("MINFO[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

//#define _MNAVI_DEBUG_

#ifdef _MNAVI_DEBUG_
#define mnavi_debug(fmt,arg...) printf("MDEBUG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_inf(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_SDXC_debug(fmt,arg...) printf("MSDXC[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_inf_csh(fmt,arg...) printf("MINF_CSH[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define mnavi_inf(fmt,arg...) do{}while(0)
#define mnavi_SDXC_debug(fmt,arg...) do{}while(0)
#define mnavi_debug(fmt,arg...) do{}while(0)
#define mnavi_inf_csh(fmt,arg...) do{}while(0)
#endif

/////for  mnavi_prop which is the member of mnavi_attr_t 
#define 	MNAVI_ATTR_NEED_DEMON		(0x01<<3)			///< 是否需要创建后台程序进行扫描
#define	MNAVI_ATTR_SEARCH_CHILD		(0x01<<0)			///< 是否扫描子目录

#define 	MNAVI_ATTR_NUM_INCLUDE_CHILD	(0x01<<5)			///< 当前文件夹的目录和文件数是否包括子文件夹
#define 	MNAVI_ATTR_EXCLUDE_EMPTY_DIR		(0x01<<2)			///< 排除空文件夹,即文件夹下没有文件或目录
#define 	MNAVI_ATTR_EXCLUDE_INVAILD_DIR	(0x01<<4)			///< 排除无效文件夹，即该文件夹及该文件夹的子文件夹没有有效文件


//// for task_attr which is the member of mnavi_task_info_t
#define  MNAVI_TASK_ATTR_STOP	(0x01<<0)


////common config
#define MNAVI_DIR_MAX_DEPTH (13)
#define MNAVI_MAX_NAME_LEN	(768)
#define MNAVI_MAX_DIRNUM 2000 	// must be not less than  2 
#define MNAVI_FILE_INFO_BUF_LEN	(200)		///< mnavi_file_info_t array length //Charles change to 增加获取文件信息速度
#define MNAVI_MAX_file_dosort     2000   //the max num of files when dosort view

#define MNAVI_PATH_SEPARATOR '/'

///relate to the share memory and semaphore///
#define MNAVI_MAX_SCANTASK_NUM	(5)

#define MNAVI_DIRINFO_FIRST_SHMID	(MNAVI_KEY_BASE)
#define MNAVI_DIRINFO_LAST_SHMID	(MNAVI_DIRINFO_FIRST_SHMID+MNAVI_MAX_SCANTASK_NUM-1)

#define MNAVI_TASKINFO_FIRST_SHMID	(MNAVI_DIRINFO_LAST_SHMID+1)
#define MNAVI_TASKINFO_LAST_SHMID	(MNAVI_TASKINFO_FIRST_SHMID+MNAVI_MAX_SCANTASK_NUM-1)

#define MNAVI_TASKINFO_FIRST_SEMID 	(MNAVI_TASKINFO_LAST_SHMID+1)
#define MNAVI_TASKINFO_LAST_SEMID	(MNAVI_TASKINFO_FIRST_SEMID+MNAVI_MAX_SCANTASK_NUM-1)

#define MNAVI_DIRINFO_FILENUMARRAY_FIRST_SHMID  (MNAVI_TASKINFO_LAST_SEMID+1)
#define MNAVI_DIRINFO_FILENUMARRAY_LAST_SHMID  (MNAVI_DIRINFO_FILENUMARRAY_FIRST_SHMID+MNAVI_MAX_SCANTASK_NUM-1)

#define MNAVI_DIRINFO_DIRNUMARRAY_FIRST_SHMID (MNAVI_DIRINFO_FILENUMARRAY_LAST_SHMID+1)
#define MNAVI_DIRINFO_DIRNUMARRAY_LAST_SHMID (MNAVI_DIRINFO_DIRNUMARRAY_FIRST_SHMID+MNAVI_MAX_SCANTASK_NUM-1)


#define MNAVI_FILEINFO_ARRAY_FIRST_SHMID (MNAVI_DIRINFO_DIRNUMARRAY_LAST_SHMID+1)
#define MNAVI_FILEINFO_ARRAY_LAST_SHMID (MNAVI_FILEINFO_ARRAY_FIRST_SHMID+MNAVI_MAX_SCANTASK_NUM-1)
////////

#define MNAVI_HASH_TABLE_SIZE (9973)	///<mnavi hash table size, it is must be an prime

#define  	MNAVI_EXTENTION_LEN (8)			///< the max length of the extention of file

typedef enum{	///< task status
	MNAVI_TASK_EDLE=0,		///< edle
	MNAVI_TASK_RUNING,		///< running
	MNAVI_TASK_COMPLETED,	///< scan completed
}mnavi_task_status_e;

typedef enum{ 	///< file sort, tell mnavi that the extentions includes which type of file
	MNAVI_FS_PHOTO,		///< file type ==photo
	MNAVI_FS_MUSIC,		///< file type ==music
	MNAVI_FS_VIDEO,		///< file type ==video
	MNAVI_FS_TXT,			///< file type ==txt
	MNAVI_FS_ALL,			///< file type ==include all type
}mnavi_fs_e;

typedef enum{ 	///< file sort, tell mnavi that the extentions includes which type of file
	MNAVI_SORT_BY_NAME,		///dosort by filename
	MNAVI_SORT_BY_TIME,		///dosort by filedata
	
}mnavi_sort_keyword_e;


typedef enum{
	MNAVI_LM_BROWSE_FILE,	///< just include file
	MNAVI_LM_BROWSE_ALL,	///<include dir and file
}mnavi_list_mode_e;

typedef struct mnavi_filesort_info_s{	///< this is used for search multi types of files in one time
	mnavi_fs_e *filesort_array;			///< the sort type array
	unsigned char sort_num;		///< how many sort type had been include must >= 1
	mnavi_fs_e (*get_filesort)(char * ext_name);		///< func pointer to the function 
}mnavi_filesort_info_t;

typedef struct mnavi_sig_handle_s{
	__sighandler_t sig_quit;		///< signal quit handle
	__sighandler_t sig_init;		///< signal init handle
	__sighandler_t sig_child;	///< signal child handle
}mnavi_sig_handle_t;


typedef struct mnavi_task_info_s{
	int sem_key;		///< semaphore key
	int sem_id;		///< semaphore id
	int shm_key;		///< share memory key
	int shm_id;		///< share memory id
	pid_t task_pid;	///< this is the pid of the child process
	mnavi_task_status_e task_status;
	int task_attr;		///< the attr for the task 
}mnavi_task_info_t;

typedef struct dirinfo{
	int *filenum_array;                      	///< how many valid file had the current dir included
	int *dirnum_array;				///< how many dir which had the valid file had the current dir included
	struct dirinfo * children;         ///< pointer  to the firt child of the current dir
	struct dirinfo * next_brother;          	///< pointer to the next dir which has the same depth
	struct dirinfo * prev_brother;			///< prev brother
	struct dirinfo * parent;          	///< pointer to the parent dir
	int real_array_index;		///< the position of the dir in the array
	long int dir_offset;				///< the offset in the parent dir, for the purpose of getting struct entry quickly
}dir_info_t;

typedef struct mnavi_dir_ops_s{
	DIR * (*opendir)(const char *name);
	int (*closedir)(DIR * pDir);
	void (*rewinddir)(DIR *pDir);
	struct dirent * (*readdir)(DIR *pDir);
	long int (*telldir)(DIR * pDir);
	void (*seekdir)(DIR *pDir, long int pos);
	char *(*getcwd)(char *buf,int bufsize);
	int (*chdir)(char* dir_name);
}mnavi_dir_ops_t;

typedef struct mnavi_dir_info_s{
	int dirinfo_array_shm_key;				///< share memory key for dirinfo_array
	int dirinfo_array_shm_id;				///< share memory id for dirinfo_array
	int filenum_array_shm_key;	///< share memory key for filenum_array
	int filenum_array_shm_id;	///< share memory id for filenum_array
	int dirnum_array_shm_key;	///< share memory key for dirnum_array
	int dirnum_array_shm_id;	///< share memory id for dirnum_array
	dir_info_t * dirinfo_array;	///< dir_info array get from share memory
	int max_dir_index;			///< max dir index
	int malloc_dir_index;		///< to note the malloc dir index, it is the virtual of the arrayindex
	int sort_num;				///< the sort num , it is also the length of the filenum_array and dirnum_array
	mnavi_dir_ops_t dir_ops;
	unsigned char cur_dir_level;		///<  the current dir level
	unsigned int dir_search_attr;   ///< using when search files 
}mnavi_dir_info_t;

typedef struct mnavi_hash_s{ ///< this is used for ext lookup
	int item;    //记录传近来要保存的数值，通常是一个索引号
	int full;    //记录哈希表是否已经被分配，如果大于等于0该值其实是计算得到的哈希值
	int base;
}mnavi_hash_t;

typedef struct mnavi_ops_s{///< memory operations
	void * (*m_malloc)(unsigned int size);
	void * (*m_realloc)(void *ptr,unsigned int newsize);
	void  (*m_free)(void* ptr);
}mnavi_ops_t;

typedef struct mnavi_attr_s{
	int mnavi_prop;		///< see MNAVI_ATTR_NEED_FILENUM etc
	mnavi_ops_t mnavi_opts;		///<mnavi memory operations 
	mnavi_filesort_info_t fs_info;
	mnavi_list_mode_e mnvai_lsmode;	///< mnavi list mode
}mnavi_attr_t;

typedef struct mnavi_exts_info_s{
	unsigned char ext_num;				///< how many valid ext had been included, such as the user specified the exts is "AVI MP4 3GP" ,the ext_num=3
	unsigned char each_ext_len;			///< the length of each ext 
	char *ext_array;					///< the ext array
	mnavi_hash_t mnavi_hash[MNAVI_HASH_TABLE_SIZE];	///< hash table, used for search
	unsigned char is_ext_all;	///< whether check all file, no ext had been specified
}mnavi_exts_info_t;


typedef struct manvi_file_info_s{
	int file_index;		///< 文件实际对应的index
	int file_type;		///< 文件的类型
	char full_path[MNAVI_MAX_NAME_LEN];
}mnavi_file_info_t;

typedef struct mnavi_fileinfo_cache_s
{
	mnavi_file_info_t *fileinfo_array;
	unsigned short int array_len;
	dir_info_t * cur_dir;
	char cur_dir_path[MNAVI_MAX_NAME_LEN];
}mnavi_fileinfo_cache_t;

typedef struct mnavi_sortview_s{
	unsigned short int *view;
	int view_len;
	struct mnavi_sortview_s *next_view;
	struct mnavi_sortview_s *prev_view;
	dir_info_t * view_dir;
}mnavi_sortview_t;

/** 
 * @brief 文件选择器
*
* 详细说明…
*/
typedef struct {
	char * root_path;
	char * cur_search_path;			///< the current searching dir full path, can used it as temp buffer
	int cur_search_path_len;		///< the length of the cur_search_path
	mnavi_dir_info_t dir_info;
	mnavi_attr_t *mnavi_attr;
	mnavi_exts_info_t *exts_info;
	mnavi_task_info_t *task_info;	///< get from share memory
	mnavi_fileinfo_cache_t fileinfo_cache;
	mnavi_sortview_t *sortview;
}Fsel_t;

typedef struct mnavi_listinfo_para_s{
	int file_idx;			///<  上层传入的file_idx
	mnavi_fs_e file_sort;	///< 上层传入的file_sort,指定是哪种类型的文件
	char *file_path;		///< 底层填充的file_path
	int file_type;			///< 底层填充的filt_type DT_REG表明是个普通文件，DT_DIR 表明是个文件夹
	long int dir_offset;
}mnavi_listinfo_para_t;

typedef struct mnavi_filepos_info_s{
	char path[MNAVI_MAX_NAME_LEN];///< 指定的idx在哪个路径下面
	dir_info_t * dirinfo;	///<指定的idx在哪个dir下面，
	int idx_offset;	///< 指定的idx在获取的dirinfo路径下是第几个，从0开始
}mnavi_filepos_info_t;

/**
get a file selector pointer
**/
Fsel_t * mnavi_fsel_open( char *  rootpath, char  *extentions,mnavi_attr_t* mnavi_attr);


/**
release a file selector
**/
void mnavi_fsel_exit(Fsel_t* fsel);


/**
start scan the file
**/
int  mnavi_start_scan_dir(Fsel_t *pFsel);


/**
if it create a demon, get the task status
**/
int mnavi_get_task_staus(Fsel_t *pFsel);


/**
set the list mode
**/
int mnavi_set_lsmode(Fsel_t *pFsel,mnavi_list_mode_e lsmode);


/**
get file total num
**/
int mnavi_get_file_total(Fsel_t *pFsel,mnavi_fs_e file_sort);


/**
get file info== file full path
**/
int mnavi_get_file_info(Fsel_t *pFsel,mnavi_listinfo_para_t *listinfo_para);


/**
cd to the child dir
**/
int mnavi_cd_child_dir(Fsel_t *pFsel,char *child_dirname_fullpath);


/**
cd to parent dir
**/
int mnavi_cd_parent_dir(Fsel_t *pFsel);


/**
attach a sort view to the selector
**/
int mnavi_attach_sortview(Fsel_t *pFsel,mnavi_fs_e file_sort);


/**
detach a last attached sort view from the selector
**/
int mnavi_detach_sortview(Fsel_t *pFsel);


/**
transfrom the view idx to the file idx, the file idx can be used when getting file info
**/
int mnavi_viewidx_2_fileidx(Fsel_t *pFsel,int viewidx);


/**
change the dir tree's data, when the user delete a file outside
**/
int mnavi_delete_file(Fsel_t *pFsel,mnavi_listinfo_para_t *listinfo_para);

/**
change the dir tree's data, when the user delete a file outside
**/
int fsel_get_ext_name(char * file_name,char* ext_name,int ext_name_size);

#ifdef __cplusplus                     
}
#endif


#endif

