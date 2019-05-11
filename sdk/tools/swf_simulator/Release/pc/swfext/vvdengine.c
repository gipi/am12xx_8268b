#include "swf_types.h"
#include "fui_common.h"
#include "swfext.h"
#include "windows.h"
#include "swf_system.h"
#include "stdio.h"

#define VVD_DEBUG
#ifdef VVD_DEBUG
#define DBG_PRINT printf
#else
#define DBG_PRINT
#endif

typedef struct _FileList {
	int             total_num;
	HANDLE          handle;
	WIN32_FIND_DATA data;
	char			root[256];
	char			type[256];
	char			str[256];
} UI_FILELIST;

typedef enum{
	FILES_BROWSE=0,
		DIR_BORWSE_ALL,
		DIR_BORWSE_VALID,
		DIR_BORWSE_HIDE,
}FILST_TYPE_t;

typedef struct _VtInfo {
	BLIST        BList;
	char         thumb[128];
	char         flash[128];
	int	         pic_num;
	SWF_DEC *    swf_inst;
} VT_INFO;

typedef struct _ItemInfo {
	BLIST     BList;
	char      path[128];
} ITEM_INFO;

typedef struct _Vd2Info {
	char          themepath[128];
	char          filename[128];
	int           state;
	
	BLIST         templ_list;
	int           templ_index;
	UI_FILELIST * templ_files;
	
	BLIST         music_list;
	int           music_index;
	UI_FILELIST * music_files;

	BLIST         photo_list;
	UI_FILELIST * photo_files;
	int           photo_index;
	
	SWF_RECT      window;
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

static VD2_INFO * currVd2 = NULL;
static VT_INFO  s_vt;

static int strcasecmp(const char * cs,const char * ct)
{
	register signed char ch1, ch2;

	do
	{
		if ( ((ch1 = (unsigned char)(*(cs++))) >= 'A') &&(ch1 <= 'Z') )
			ch1 += 0x20;

		if ( ((ch2 = (unsigned char)(*(ct++))) >= 'A') &&(ch2 <= 'Z') )
			ch2 += 0x20;
	} while ( ch1 && (ch1 == ch2) );

	return(ch1 - ch2);
}

static char * strdup(const char * s)
{
	char * str = (char*)malloc(strlen(s) + 1);
	strcpy(str, s);
	return str;
}

static INT32S str2i(char * str)
{
	int num;
	for(num = 0; *str != 0; str++)
	{
		num = num * 10 + (*str - '0');
	}
	return num;
}

static int ParseValue(void * h, stream_input_t * pb, int (*set_value)(void*,char*,char*))
{
	INT32S len, i, n = 0, state = VVD_PARSER_NAME;
	char * buf, * name, * val;
	int ret = 0;
	
	buf  = (char*)malloc(BUF_LEN);
	name = (char*)malloc(128);
	val  = (char*)malloc(128);

	while((len = pb->read(pb, (INT32U)buf, BUF_LEN)) > 0)
	{
		for(i = 0; i < len; i++)
		{
			switch(buf[i])
			{
			case '\t':
			case '\r':
			case ' ' :
			case '\0':
				continue;
			case '\n':
				if(state == VVD_PARSER_VALUE)
				{
					val[n] = 0;
					if(set_value(h, name, val))
					{
						ret = 1;
					}
				}
				n = 0;
				state = VVD_PARSER_NAME;
				break;
			case '=':
				if(state == VVD_PARSER_NAME)
				{
					name[n] = 0;
					state = VVD_PARSER_VALUE;
				}
				n = 0;
				break;
			default:
				if(state == VVD_PARSER_NAME)
				{
					name[n] = buf[i];
				}
				else if(state == VVD_PARSER_VALUE)
				{
					val[n] = buf[i];
				}
				n++;
			}
		}
	}

	free(val);
	free(name);
	free(buf);
	
	return ret;
}

static VT_INFO * vvd_new_template(char * filename)
{
	char * suffix;
	VT_INFO * vt_info = (VT_INFO*)malloc(sizeof(VT_INFO));
	vt_info->swf_inst = NULL;
	strcpy(vt_info->flash, filename);
	strcpy(vt_info->thumb, vt_info->flash);
	suffix = strstr(vt_info->thumb, ".swf");
	if(suffix != NULL)
	{
		suffix[1] = 'j';
		suffix[2] = 'p';
		suffix[3] = 'g';
	}
	else
	{
		vt_info->thumb[0] = 0;
	}
	return vt_info;
}

static INT32S vvd_parser(VD2_INFO * vd2_info, char * name, char * val)
{
	if(val[0] != 0 && name[0] != 0)
	{
		if(strcasecmp(name, "theme") == 0)
		{
			strcpy(vd2_info->themepath, val);
		}
		else if(strcasecmp(name, "template") == 0)
		{
			BListAdd(&vd2_info->templ_list, &vvd_new_template(val)->BList);
		}
		else if(strcasecmp(name, "pic") == 0)
		{
			ITEM_INFO * photo = (ITEM_INFO*)malloc(sizeof(ITEM_INFO));
			strcpy(photo->path, val);
			BListAdd(&vd2_info->photo_list, &photo->BList);
		}
		else if(strcasecmp(name, "music") == 0)
		{
			ITEM_INFO * music = (ITEM_INFO*)malloc(sizeof(ITEM_INFO));
			strcpy(music->path, val);
			BListAdd(&vd2_info->music_list, &music->BList);
		}
	}
	return 0;
}

static void vvd_new_name(char * path, char * filename)
{
	char * p0 = NULL, * p1 = NULL, * p;
	char prefix[128];
	SWF_DATE d;
	SWF_TIME t;

	//remove space and tab
	for(p = &path[strlen(path) - 1]; p > path; p--)
	{
		if(*p == ' ' || *p == '\t')
		{
			*p = 0;
		}
		else
		{
			break;
		}
	}
	//search '\\'
	for(p = path; *p != 0; p++)
	{
		if(*p == '\\')
		{
			p0 = p1;
			p1 = p;
		}
	}
	//check last '\\'
	if(p > path && *(p - 1) != '\\')
	{
		p0 = p1;
		p1 = p;
	}
	//get filename
	if(p0 != NULL && p1 != NULL)
	{
		int i;
		for(i = 0, p = p0 + 1; p < p1; p++, i++)
		{
			prefix[i] = *p;
		}
		prefix[i] = 0;
	}
	else
	{
		strcpy(prefix, "user");
	}
	swf_sys_getDate(&d);
	swf_sys_getTime(&t);
	sprintf(filename, "%s%02d%02d%02d%02d%02d.vd2", prefix, d.year%100, d.month+1, d.day, t.hour, t.minute);
}

static void vvd_current_path(char * path)
{
	strcpy(path, "c:\\");
}

#include "swf_dec.h"

static VD2_INFO * vvd_new(char * filename)
{
	VD2_INFO * vd2_info;
	vd2_info = (VD2_INFO*)malloc(sizeof(VD2_INFO));
	BLIST_INIT(&vd2_info->templ_list);
	BLIST_INIT(&vd2_info->music_list);
	BLIST_INIT(&vd2_info->photo_list);
	vd2_info->state  = VVD_IDLE;
	vd2_info->templ_index = 0;
	vd2_info->photo_index = 0;
	vd2_info->music_index = 0;
	vd2_info->templ_files = NULL;
	vd2_info->photo_files = NULL;
	vd2_info->music_files = NULL;
	strcpy(vd2_info->themepath, filename);
	return vd2_info;
}

static VD2_INFO * vvd_open(char * filename, int num)
{
	VD2_INFO * vd2_info = NULL;
	stream_input_t * pb;
	if(strstr(filename, ".") == NULL)
	{
		//This is a theme directory
		vd2_info = vvd_new(filename);
		vvd_new_name(vd2_info->themepath, vd2_info->filename);
		if(num > 0)
		{
			UI_FILELIST tmpl_files;
			ui_filelist_init(&tmpl_files, filename[0], filename, "swf", 0, FILES_BROWSE);
			if(tmpl_files.total_num > 0)
			{
				int i, templ_index = -1;
				for(i = 0; i < num; i++)
				{
					char * path;
					int r = 0, j;
					for(j = 0; j < tmpl_files.total_num; j++)
					{
						r = rand() % tmpl_files.total_num;
						if(r != templ_index)
						{
							templ_index = r;
							break;
						}
					}
					path = ui_filelist_get_cur_filepath(&tmpl_files, r);
					BListAdd(&vd2_info->templ_list, &vvd_new_template(path)->BList);
				}
			}
		}
	}
	else
	{
		//This is a normal file
		pb = create_fui_input(filename);
		if(pb != NULL)
		{
			vd2_info = vvd_new(filename);
			ParseValue(vd2_info, pb, vvd_parser);
			pb->dispose(pb);
		}
	}
	return vd2_info;
}

static int vvd_save(VD2_INFO * vd2, char * path)
{
	VT_INFO * vt;
	ITEM_INFO * item;
	stream_input_t * pb = NULL;
	char buf[128];
	if(vd2 == NULL) return 0;
	if(path != NULL)
	{
		strcpy(buf, path);
		if(path[strlen(path)-1] != '\\')
		{
			strcat(buf, "\\");
		}
		strcat(buf, vd2->filename);
	}
	else
	{
		strcpy(buf, "c:\\");
	}
	pb = create_fui_output(buf);
	if(pb != NULL)
	{
		sprintf(buf, "theme=%s\n", vd2->themepath);
		pb->write(pb, (int)buf, strlen(buf));
		
		ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		{
			sprintf(buf, "template=%s\n", vt->flash);
			pb->write(pb, (int)buf, strlen(buf));
		}
		
		ITERATE_OVER_LIST(&vd2->photo_list, (BLIST*)item)
		{
			sprintf(buf, "pic=%s\n", item->path);
			pb->write(pb, (int)buf, strlen(buf));
		}
		
		ITERATE_OVER_LIST(&vd2->music_list, (BLIST*)item)
		{
			sprintf(buf, "music=%s\n", item->path);
			pb->write(pb, (int)buf, strlen(buf));
		}
		pb->dispose(pb);
		return 1;
	}
	return 0;
}

static void vvd_close(VD2_INFO * vd2)
{
	VT_INFO * vt;
	ITEM_INFO * item;
	stream_input_t * pb = NULL;
	if(vd2 != NULL)
	{
		while((vt = (VT_INFO*)BListRemoveItemFromHead(&vd2->templ_list)) != 0)
		{
			free(vt);
		}
		while((item = (ITEM_INFO*)BListRemoveItemFromHead(&vd2->music_list)) != 0)
		{
			free(item);
		}
		while((item = (ITEM_INFO*)BListRemoveItemFromHead(&vd2->photo_list)) != 0)
		{
			free(item);
		}
		free(vd2);
	}
}

static int vvd_decode_bitmap(SWF_IMAGE_INFO * info)
{
	if(currVd2 != NULL)
	{
		if((info->flag & 0xF) != SWF_BMP_FMT_ARGB && (info->width * info->height == 800 * 600))
		{
			if(BLIST_IS_EMPTY(&currVd2->photo_list))
			{
				if(currVd2->photo_files == NULL)
				{
					char path[8];
					vvd_current_path(path);
					currVd2->photo_files = (UI_FILELIST*)malloc(sizeof(UI_FILELIST));					
					ui_filelist_init(currVd2->photo_files, path[0], path, "jpg bmp", 0, FILES_BROWSE);					
				}
				if(currVd2->photo_files->total_num > 0)
				{
					currVd2->photo_index = rand() % currVd2->photo_files->total_num;
					info->jpeg_data = ui_filelist_get_cur_filepath(currVd2->photo_files, currVd2->photo_index);
					info->jpeg_size = 0;
					DBG_PRINT("vvd show %s\n", info->jpeg_data);
				}
			}
			else
			{
				ITEM_INFO * item;
				int index = currVd2->photo_index++;
				while(index > 0)
				{
					ITERATE_OVER_LIST(&currVd2->photo_list, (BLIST*)item)
					{
						index--;
						if(index == 0)
						{
							info->jpeg_data = item->path;
							info->jpeg_size = 0;
							DBG_PRINT("vvd show %s\n", info->jpeg_data);
							break;
						}
					}
				}
			}
		}
		decode_bitmap(info);
	}
	return 0;
}

static int vvd_release(SWF_DEC * s);
static int vvd_add_playlist(VT_INFO * vt)
{
	SWF_DEV_FUNC dev_func;
	SWF_INSTINFO info;
	int w = SWF_GetActiveInst()->window_rect.Xmax/16;
	int h = SWF_GetActiveInst()->window_rect.Ymax/16;
	SWF_RECT window = {0, 0, w, h};
	vt->swf_inst = SWF_AddInst(vt->flash, &window, SWF_FULLSCREEN_FLAG | SWF_NON_RELOAD | SWF_STANDALONE, &info);
	if(vt->swf_inst != NULL)
	{
		SWF_GetDeviceFunction(vt->swf_inst, &dev_func);
		dev_func.decode_bitmap = vvd_decode_bitmap;  
		dev_func.release = vvd_release;
		SWF_SetDeviceFunction(vt->swf_inst, &dev_func);
		DBG_PRINT("show theme %s\n", vt->flash);
		return 1;
	}
	DBG_PRINT("cannot find %s for vvd\n", vt->flash);
	return 0;
}
static int vvd_next(VD2_INFO * vd2)
{
	if(BLIST_IS_EMPTY(&vd2->templ_list))
	{
		if(vd2->templ_files == NULL)
		{
			vd2->templ_files = (UI_FILELIST*)malloc(sizeof(UI_FILELIST));					
			ui_filelist_init(vd2->templ_files, vd2->themepath[0], vd2->themepath, "swf", 0, FILES_BROWSE);
		}
		if(vd2->templ_files->total_num > 0)
		{
			vd2->templ_index = rand() % vd2->templ_files->total_num;
			strcpy(s_vt.flash, ui_filelist_get_cur_filepath(vd2->templ_files, vd2->templ_index));
			return vvd_add_playlist(&s_vt);
		}
	}
	else
	{
		VT_INFO * vt;
		int i = 1;
		ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		{
			if(i > vd2->templ_index)
			{
				vd2->templ_index = i;
				return vvd_add_playlist(vt);
			}
			i++;
		}
		vd2->templ_index = 0;
	}
	return 0;
}
static int vvd_release(SWF_DEC * s)
{
	if(currVd2 != NULL)
	{
		if(currVd2->state == VVD_PLAY)
		{
			if(vvd_next(currVd2))
			{
				return 1;
			}
			currVd2->state = VVD_IDLE;
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}		
	}
	return 0;
}
static void vvd_play(VD2_INFO * vd2)
{
	if(vd2 != NULL)
	{
		if(vd2->state == VVD_IDLE)
		{
			vd2->templ_index = 0;
			if(vvd_next(vd2))
			{
				vd2->state = VVD_PLAY;
				//__AP_EG_Start(vd2);
				return;
			}
			DBG_PRINT("vvd no valid template\n");
		}
		else
		{
			DBG_PRINT("vvd not idle : %d\n", vd2->state);
		}
	}
}
static void vvd_pause(VD2_INFO * vd2)
{
	VT_INFO * vt;
	if(vd2->state == VVD_PLAY)
	{
		int i = 0;
		ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		{
			if(i == vd2->templ_index)
			{
				SWF_Message(vt->swf_inst, SWF_MSG_PAUSE, NULL);
			}
			i++;
		}
		vd2->state = VVD_PAUSE;
	}
}
static void vvd_resume(VD2_INFO * vd2)
{
	VT_INFO * vt;
	if(vd2->state == VVD_PAUSE)
	{
		int i = 0;
		ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		{
			if(i == vd2->templ_index)
			{
				SWF_Message(vt->swf_inst, SWF_MSG_ACTIVE, NULL);
			}
			i++;
		}
		vd2->state = VVD_PLAY;
	}
}
static void vvd_stop(VD2_INFO * vd2)
{
	if(BLIST_IS_EMPTY(&vd2->templ_list))
	{
		if(s_vt.swf_inst)
		{
			SWF_DEV_FUNC dev_func;
			dev_func.release = NULL;
			SWF_SetDeviceFunction(s_vt.swf_inst, &dev_func);
			SWF_RemoveInst(s_vt.swf_inst);
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
	}
	else
	{
		VT_INFO * vt;
		int i = 0;
		ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		{
			if(i + 1 == vd2->templ_index)
			{
				SWF_DEV_FUNC dev_func;
				dev_func.release = NULL;
				SWF_SetDeviceFunction(vt->swf_inst, &dev_func);
				SWF_RemoveInst(vt->swf_inst);
				SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
				break;
			}
			i++;
		}
	}
	vd2->state = VVD_IDLE;
}
static BLIST * vvd_get_item(BLIST * list, int index)
{
	BLIST * item;
	ITERATE_OVER_LIST(list, (BLIST*)item)
	{
		if(index-- == 0)
		{
			return item;
		}
	}
	return NULL;
}

/************************************************************************/

static INT32S Open(void * handle)
{
	int number = 6;
	char * path = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_close(currVd2);
		currVd2 = NULL;
	}
	if(Swfext_GetParamType() == SWFDEC_AS_TYPE_STRING)
	{
		path = Swfext_GetString();
		if(Swfext_GetParamType() == SWFDEC_AS_TYPE_NUMBER)
		{
			number = Swfext_GetNumber();
		}
		currVd2 = vvd_open(path, number);
	}
	if(currVd2 != NULL)
	{
		Swfext_PutNumber(1);
	}
	else
	{
		DBG_PRINT("cannot open vvd file %d\n", path);
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();	
}

static INT32S Save(void * handle)
{
	int type;
	char * path = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		type = Swfext_GetParamType();
		if(type == SWFDEC_AS_TYPE_STRING)
		{
			path = Swfext_GetString();
		}
		if(vvd_save(currVd2, path))
		{
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_close(currVd2);
		currVd2 = NULL;
	}
	SWFEXT_FUNC_END();	
}

static INT32S Play(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_play(currVd2);
	}
	SWFEXT_FUNC_END();	
}

static INT32S Stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_stop(currVd2);
	}
	SWFEXT_FUNC_END();	
}

static INT32S Pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_pause(currVd2);
	}
	SWFEXT_FUNC_END();	
}

static INT32S Resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vvd_resume(currVd2);
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetState(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		Swfext_PutNumber(currVd2->state);
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S SetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

/************************************************************************/

static INT32S GetTemplateNum(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&currVd2->templ_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	SWFEXT_FUNC_END();	
}

static INT32S InsertTemplate(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int i = 0, index = Swfext_GetNumber();
		UI_FILELIST tmpl_files;
		ui_filelist_init(&tmpl_files, currVd2->themepath[0], currVd2->themepath, "swf", 0, FILES_BROWSE);
		if(tmpl_files.total_num > 0)
		{
			int tmpl_index = rand() % tmpl_files.total_num;
			char * path = ui_filelist_get_cur_filepath(&tmpl_files, tmpl_index);
			VT_INFO * vt = vvd_new_template(path), * vt2;
			if(vt != NULL)
			{
				if(index > 0)
				{
					BLIST * after;
					ITERATE_OVER_LIST(&currVd2->templ_list, after)
					{
						if(index-- <= 0) break;
					}
					BListInsertTail(after, &vt->BList);
				}
				else if(index == 0)
				{
					BListInsertHead(&currVd2->templ_list, &vt->BList);
				}
				else
				{
					BListAdd(&currVd2->templ_list, &vt->BList);
				}
				ITERATE_OVER_LIST(&currVd2->templ_list, (BLIST*)vt2)
				{
					if(vt2 == vt)
					{
						break;
					}
					i++;
				}
				Swfext_PutNumber(i);
				SWFEXT_FUNC_END();	
			}
		}
		
	}
	Swfext_PutNumber(-1);
	SWFEXT_FUNC_END();	
}

static INT32S DeleteTemplate(void * handle)
{
	VT_INFO * vt;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&currVd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			BListRemove(&vt->BList);
			free(vt);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static INT32S GetTemplatePath(void * handle)
{
	VT_INFO * vt;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&currVd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			Swfext_PutString(vt->flash);
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetThumbPath(void * handle)
{
	VT_INFO * vt;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&currVd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			Swfext_PutString(vt->thumb);
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetPhotoNum(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&currVd2->photo_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	SWFEXT_FUNC_END();	
}

static INT32S InsertPhoto(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int    i = 0;
		int    index = Swfext_GetNumber();
		char * path  = Swfext_GetString();
		ITEM_INFO * photo = (ITEM_INFO*)malloc(sizeof(ITEM_INFO)), * photo2;
		strcpy(photo->path, path);
		if(photo != NULL)
		{
			if(index > 0)
			{
				BLIST * after;
				ITERATE_OVER_LIST(&currVd2->photo_list, after)
				{
					if(index-- <= 0) break;
				}
				BListInsertTail(after, &photo->BList);
			}
			else if(index == 0)
			{
				BListInsertHead(&currVd2->photo_list, &photo->BList);
			}
			else
			{
				BListAdd(&currVd2->photo_list, &photo->BList);
			}
			ITERATE_OVER_LIST(&currVd2->photo_list, (BLIST*)photo2)
			{
				if(photo2 == photo)
				{
					break;
				}
				i++;
			}
			Swfext_PutNumber(i);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(-1);
	SWFEXT_FUNC_END();	
}

static INT32S DeletePhoto(void * handle)
{
	ITEM_INFO * photo;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		photo = (ITEM_INFO*)vvd_get_item(&currVd2->photo_list, Swfext_GetNumber());
		if(photo != NULL)
		{
			BListRemove(&photo->BList);
			free(photo);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static INT32S GetPhotoPath(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		ITEM_INFO * photo = (ITEM_INFO*)vvd_get_item(&currVd2->photo_list, Swfext_GetNumber());
		if(photo != NULL)
		{
			Swfext_PutString(photo->path);
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetMusicNum(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&currVd2->music_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	SWFEXT_FUNC_END();	
}

static INT32S InsertMusic(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		int    i = 0;
		int    index = Swfext_GetNumber();
		char * path  = Swfext_GetString();
		ITEM_INFO * music = (ITEM_INFO*)malloc(sizeof(ITEM_INFO)), * music2;
		strcpy(music->path, path);
		if(music != NULL)
		{
			if(index > 0)
			{
				BLIST * after;
				ITERATE_OVER_LIST(&currVd2->music_list, after)
				{
					if(index-- <= 0) break;
				}
				BListInsertTail(after, &music->BList);
			}
			else if(index == 0)
			{
				BListInsertHead(&currVd2->music_list, &music->BList);
			}
			else
			{
				BListAdd(&currVd2->music_list, &music->BList);
			}
			ITERATE_OVER_LIST(&currVd2->music_list, (BLIST*)music2)
			{
				if(music2 == music)
				{
					break;
				}
				i++;
			}
			Swfext_PutNumber(i);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(-1);
	SWFEXT_FUNC_END();	
}

static INT32S DeleteMusic(void * handle)
{
	ITEM_INFO * music;
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		music = (ITEM_INFO*)vvd_get_item(&currVd2->music_list, Swfext_GetNumber());
		if(music != NULL)
		{
			BListRemove(&music->BList);
			free(music);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static INT32S GetMusicPath(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		ITEM_INFO * music = (ITEM_INFO*)vvd_get_item(&currVd2->music_list, Swfext_GetNumber());
		if(music != NULL)
		{
			Swfext_PutString(music->path);
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetSavePath(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		Swfext_PutString(currVd2->filename);
	}
	SWFEXT_FUNC_END();	
}

static INT32S LoadMovie(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(currVd2 != NULL)
	{
		SWF_DEV_FUNC dev_func;
		VT_INFO * vt;
		ITERATE_OVER_LIST(&currVd2->templ_list, (BLIST*)vt)
		{
			void * target = Swfext_GetObject();
			int width = Swfext_GetNumber() * 20 / 16;
			int height = Swfext_GetNumber() * 20 / 16;
			dev_func.decode_bitmap = vvd_decode_bitmap;
			dev_func.play_audio = NULL;
			dev_func.release = NULL;
			dev_func.KeySound_Trigger = NULL;
			SWF_LoadMovie(target, vt->flash, width, height, &dev_func);
			break;
		}
	}
	SWFEXT_FUNC_END();	
}

static SWF_DEC * vvd_flash = NULL;

static int PlayFlash(void * handle)
{
	int w = SWF_GetActiveInst()->window_rect.Xmax/16;
	int h = SWF_GetActiveInst()->window_rect.Ymax/16;
	SWF_RECT window = {0, 0, w, h};
	SWF_INSTINFO info;
	char * path;

	SWFEXT_FUNC_BEGIN(handle);
	path = Swfext_GetString();
	if(path)
	{
		char fullpath[AS_MAX_TEXT];

		if(vvd_flash)
		{
			SWF_RemoveInst(vvd_flash);
		}
		
		//get full path
		if(path[1] != ':')
		{
			//relative path
			char * p, * q;
			strcpy(fullpath, SWF_GetFilePath(handle));
			for(p = fullpath, q = NULL; *p != 0; p++)
			{
				if(*p == '\\')
				{
					q = p + 1;
				}
			}
			if(q)
			{
				strcpy(q, path);
			}
			else
			{
				strcpy(fullpath, path);
			}
		}
		else
		{
			strcpy(fullpath, path);
		}

		//insert instance
		vvd_flash = SWF_AddInst(fullpath, &window, SWF_FULLSCREEN_FLAG | SWF_STANDALONE, &info);
	}
	SWFEXT_FUNC_END();	
}

static int StopFlash(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	vvd_flash = SWF_GetActiveInst();
	if(vvd_flash)
	{
		SWF_RemoveInst(vvd_flash);
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		vvd_flash = NULL;
	}
	SWFEXT_FUNC_END();	
}

INT32S swfext_vvd_register(void)
{
	SWFEXT_REGISTER("vvd_Open", Open);
	SWFEXT_REGISTER("vvd_Save", Save);
	SWFEXT_REGISTER("vvd_Close", Close);
	SWFEXT_REGISTER("vvd_Play", Play);
	SWFEXT_REGISTER("vvd_Stop", Stop);
	SWFEXT_REGISTER("vvd_Pause", Pause);
	SWFEXT_REGISTER("vvd_Resume", Resume);
	
	SWFEXT_REGISTER("vvd_GetState", GetState);
	SWFEXT_REGISTER("vvd_GetPlayMode", GetPlayMode);
	SWFEXT_REGISTER("vvd_SetPlayMode", SetPlayMode);

	SWFEXT_REGISTER("vvd_GetTemplateNum", GetTemplateNum);
	SWFEXT_REGISTER("vvd_InsertTemplate", InsertTemplate);
	SWFEXT_REGISTER("vvd_DeleteTemplate", DeleteTemplate);
	SWFEXT_REGISTER("vvd_GetTemplatePath", GetTemplatePath);
	SWFEXT_REGISTER("vvd_GetThumbPath", GetThumbPath);
	
	SWFEXT_REGISTER("vvd_GetPhotoNum", GetPhotoNum);
	SWFEXT_REGISTER("vvd_InsertPhoto", InsertPhoto);
	SWFEXT_REGISTER("vvd_DeletePhoto", DeletePhoto);
	SWFEXT_REGISTER("vvd_GetPhotoPath", GetPhotoPath);
	
	SWFEXT_REGISTER("vvd_GetMusicNum", GetMusicNum);
	SWFEXT_REGISTER("vvd_InsertMusic", InsertMusic);
	SWFEXT_REGISTER("vvd_DeleteMusic", DeleteMusic);
	SWFEXT_REGISTER("vvd_GetMusicPath", GetMusicPath);

	SWFEXT_REGISTER("vvd_GetSavePath", GetSavePath);
	SWFEXT_REGISTER("vvd_LoadMovie", LoadMovie);

	SWFEXT_REGISTER("vvd_PlayFlash", PlayFlash);
	SWFEXT_REGISTER("vvd_StopFlash", StopFlash);
	
	return 0;
}


