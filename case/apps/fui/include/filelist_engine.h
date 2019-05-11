#ifndef FILELIST_ENGINE_H
#define FILELIST_ENGINE_H

#include "swf_ext.h"
#include "file_list.h"


typedef struct _swf_filelist{
	int free;
	UI_FILELIST filelist;
}SWF_FILELIST;

#define SWF_MAX_FILELIST 10

enum{
	ASFS_ERR_OK = 0,           
	ASFS_ERR_EOF = 1,            
	ASFS_ERR_Path = 2,           
	ASFS_ERR_FileExist = 3,        
	ASFS_ERR_Unformat = 4,        
	ASFS_ERR_DiskFull = 5,         
	ASFS_ERR_DiskErr = 6,         
	ASFS_ERR_WriteOnly = 7,       
	ASFS_ERR_ReadOnly = 8,         
	ASFS_ERR_FileInuse = 9,         
	ASFS_ERR_NoRes = 10,               
	ASFS_ERR_COPYFILE_STOP = 11,         
	ASFS_ERR_COPYDIR_STOP = 12,         
	ASFS_ERR_DELFILE_STOP = 13,          
	ASFS_ERR_DELDIR_STOP = 14,
	ASFS_ERR_UNKOWN = 15,    // unknown error
};

#endif
