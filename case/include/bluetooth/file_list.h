#ifndef _FILE_LIST_H_
#define _FILE_LIST_H_

#include "mnavi_api.h"

#define FULL_PATH_SIZE	(1024)
#define MAX_FILELIST 	(10)
/**
* @brief type of file search
*/
typedef enum{
	FLIST_MODE_BROWSE_FILE,	///< just include file
	FLIST_MODE_BROWSE_ALL,	///<include dir and file
}filelist_mode_e;

/**
* @brief type of disk
*/
typedef enum
{
	NAND_DISK = 0,
	CARD_DISK = 1,
	HARD_DISK = 2,
	USB_DISK1 = 3,
	USB_DISK2 = 4,
	USB_DISK3 = 5,
	USB_DISK4 = 6,
	ROOT_DISK = 7,
	PRIV_DISK=8,
	DISK_NUM  = 9
}Disk_Type;

enum{
	FT_DIR=0,// 目录名
	FT_FILE,// 文件名
	FT_MUSIC,
	FT_VIDEO,
	FT_IMAGE,
	FT_VVD,
	FT_TXT
};
#define MAX_FILE_TYPE FT_TXT

/**
* @brief scan control command
* 
*/
enum{
   NO_SCAN=0,				///< the task will no be created
   SCAN_NOW,				///< create a task for scanning
   SCAN_LATER,				///< haven't be used under linux
};

/**
* @brief synchronization when delete files while search
*    is in process
*/
typedef enum{
	NONEED_TO_SYNC=0,			///< don't need to sync
	SYNC_TO_FILE,				///< sync the file
	SYNC_TO_DIR,				///< sync the dir
}FILST_SYNC_t;

/**
* @brief scan status returned when call "ui_filelist_check_scan_task()"
* 
*/

typedef enum{
    NO_SENSE=0,			///< it is a initial value, had no sense
    FIND_NOFILE,			///< no file can be found
    FIND_FIRST_FILE,		///< the first file had been
    FIND_FILE,			///< a file had been found
    FIND_COMPLETE,		///< the task is complete
}scan_status_t;


/**
* @brief structure that describes the file list.
*/
typedef struct
{
	Fsel_t * fsel;							///< file selector handle
	filelist_mode_e show_dir;				///< see filelist_mode_e
	int  total;								///< total item number
}UI_FILELIST;


int ui_filelist_init(UI_FILELIST * ui_filelist,char * base_path,char * file_type,char scan,filelist_mode_e  list_type);
int ui_filelist_exit(UI_FILELIST * ui_filelist);

scan_status_t ui_filelist_check_scan_task(UI_FILELIST * filelist);
int ui_filelist_refresh_list(UI_FILELIST * filelist,int file_idx);//change log

int  ui_filelist_enter_dir(UI_FILELIST * filelist,int dir_index);
int ui_filelist_esc_dir(UI_FILELIST * filelist);

int ui_filelist_set_mode(UI_FILELIST * filelist,int showdir,int* pre_activeidx);

int  ui_filelist_get_filetotal_num(UI_FILELIST * filelist);
int  ui_filelist_get_cur_filetype(UI_FILELIST * flist,int index);
char * ui_filelist_get_cur_filepath(UI_FILELIST * filelist,int  index);
char  *ui_filelist_get_cur_filepath_buf(UI_FILELIST * filelist,int  index,void * buffer);
char  *ui_filelist_get_longname(UI_FILELIST * filelist,	int  index);
char  * ui_filelist_get_cur_workpath(UI_FILELIST * filelist);
int ui_filelist_creat_sortview(UI_FILELIST * filelist,mnavi_fs_e filesort);
int ui_filelist_delete_sortview(UI_FILELIST * filelist);
int ui_filelist_delete_item(UI_FILELIST * filelist,int file_index);

#endif
