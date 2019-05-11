#ifndef VVD_ENGINE_H
#define VVD_ENGINE_H

#ifdef MODULE_CONFIG_VVD

#include "swf_types.h"
#include "fui_common.h"
#include "swf_ext.h"
#include "file_list.h"

#define VVD_DEBUG
#ifdef VVD_DEBUG
	#define DBG_PRINT(x...) printf(x)
#else
	#define DBG_PRINT(x...) do{}while(0)
#endif

typedef struct _VtInfo {
	BLIST        BList;
	char         thumb[FULL_PATH_SIZE];
	char         flash[FULL_PATH_SIZE];
	SWF_DEC *    swf_inst;
} VT_INFO;


typedef struct _ItemInfo {
	BLIST     BList;
	char      path[FULL_PATH_SIZE];
} ITEM_INFO;


typedef struct _Vd2Info {
	char          themepath[FULL_PATH_SIZE];
	char          filename[FULL_PATH_SIZE];
	char          photodir[FULL_PATH_SIZE];
	char          musicdir[FULL_PATH_SIZE];
	int           state;
	int           photo_overoll;
	
	BLIST         templ_list;
	int           templ_index;
	UI_FILELIST * templ_files;
	
	BLIST         music_list;
	int           music_index;
	UI_FILELIST * music_files;
	
	BLIST         photo_list;
	UI_FILELIST * photo_files;
	int           photo_index;
} VD2_INFO;

#define BUF_LEN 240

enum
{
	VVD_PARSER_NAME  = 0,
	VVD_PARSER_VALUE = 1,
};

enum
{
	VVD_IDLE=0,
	VVD_PLAY=1,
	VVD_PAUSE=2,
};

#endif	/** MODULE_CONFIG_VVD */

#endif
