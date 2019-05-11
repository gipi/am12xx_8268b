#include "swfext.h"
#include "windows.h"
#include "stdio.h"

typedef struct _FileList {
	int             total_num;
	HANDLE          handle;
	WIN32_FIND_DATA data;
	char			root[256];
	char			type[256];
	char			str[256];
} UI_FILELIST;

/************************************************************************/
/* for compatible with new file search
/************************************************************************/
typedef struct _swf_filelist{
	int free;
	UI_FILELIST filelist;
}SWF_FILELIST;

#define SWF_MAX_FILELIST 10



typedef enum{
	FILES_BROWSE=0,
	DIR_BORWSE_ALL,
	DIR_BORWSE_VALID,
	DIR_BORWSE_HIDE,
}FILST_TYPE_t;

typedef struct{
    INT8U year;
    INT8U month;
    INT8U date;
    INT8U hour;
    INT8U minute;
    INT8U second;
}TIME_INFO;

typedef enum{
	FT_DIR = 0,
	FT_FILE,
	FT_MUSIC,
	FT_VIDEO,
	FT_IMAGE,
	FT_VVD,
	FT_TXT,
}FT;

typedef struct {
	FT   type;
	char ext[8];
}FT_MAP;

static FT_MAP ft_map[] = {
	{FT_MUSIC, "mp3"},
	{FT_MUSIC, "wma"},
	{FT_VIDEO, "mp4"},
	{FT_VIDEO, "3gp"},
	{FT_VIDEO, "mov"},
	{FT_VIDEO, "avi"},
	{FT_IMAGE, "bmp"},
	{FT_IMAGE, "jpg"},
	{FT_IMAGE, "jpeg"},
	{FT_VVD, "vvd"},
	{FT_VVD, "swf"},
	{FT_TXT, "txt"},
};

static UI_FILELIST swf_ext_ui_filelist; 

INT8S * ui_filelist_get_cur_workpath(UI_FILELIST * filelist)
{
	return filelist->root;
}

static char * file_map(char * filename)
{
	static char buf[256];
	if(filename[1] == ':')
	{
		switch(filename[0])
		{
		case 'd':
		case 'D':
			strcpy(buf, "SD");
			break;
		case 'e':
		case 'E':
			strcpy(buf, "CF");
			break;
		case 'f':
		case 'F':
			strcpy(buf, "UDISK");
			break;
		default:
			strcpy(buf, "LOCAL");
			break;
		}
		strcat(buf, filename + 2);
		return buf;
	}
	else
	{
		return filename;
	}
}

static INT16U get_utf16(char * str, int * index)
{
	int i = *index;
	
	if((str[i] & 0x80) == 0)
	{
		*index = i + 1;
		return str[i];
	}
	else if((str[i] & 0x20) == 0)
	{
		*index = i + 2;
		return ((str[i] << 6) & 0x07c0) | (str[i+1] & 0x003f);
	}
	else if((str[i] & 0x10) == 0)
	{
		*index = i + 3;
		return ((str[i] << 12) & 0xf000) | ((str[i+1] << 6) & 0x0fc0) | (str[i+2] & 0x003f);
	}
	
	return 0;
}

static void set_utf16(char * str, int * index, INT16U u16c)
{
	int i = *index;
	
	if(u16c >= 0x800)
	{
		// 3 byte
		*index = i + 3;
		str[i] = (u16c >> 12) | 0xe0;
		str[i+1] = (u16c >> 6) & 0x3f | 0x80;
		str[i+2] = u16c & 0x3f | 0x80;
	}
	else if(u16c >= 0x80)
	{
		// 2 byte
		*index = i + 2;
		str[i] = (u16c >> 6) & 0x1f | 0xc0;
		str[i+1] = u16c & 0x3f | 0x80;
	}
	else
	{
		// 1 byte
		*index = i + 1;
		str[i] = (INT8U)u16c;
	}
}

static int utf8_to_utf16(char * u8str, INT16U * u16str)
{
	int i, j;
	for(i = 0, j = 0; (u16str[j] = get_utf16(u8str, &i)) != 0; j++);
	return j;
}

enum MultiByteSchemeCP
{
	SCHEME_UCS = CP_UTF8, //UTF-8
		SCHEME_GBK = 936,	//GBK
};

static int test_muliti_byte_scheme()
{
	WCHAR *name = "ÎÒ";
	if(((*name >> 8) == 0xD2) && ((*name & 0xFF) == 0xce))
		return SCHEME_GBK;
	else
		return SCHEME_UCS;
}

static char * MultiByteToUTF8(const char* psrc)
{
	static WCHAR wsz[128];
	static CHAR sz[256]; 
	
	int n; 	
	LPWSTR lpwsz = wsz;
	LPSTR lpsz = sz; 
	
	int flag;
	
	flag = test_muliti_byte_scheme();
	if (flag == SCHEME_GBK )
	{
		MultiByteToWideChar(CP_ACP, 0, psrc, -1, lpwsz, 128);  
		WideCharToMultiByte(CP_UTF8, 0, lpwsz, -1, lpsz, 256, NULL, NULL); 
	} 
	else
	{
		lpsz = psrc;
	}

	return lpsz;
}

static char * UTF8ToMultiByte(const char* psrc)
{
	static WCHAR wsz[128];
	static CHAR sz[256]; 
	
	int n;
	LPWSTR lpwsz = wsz;
	LPSTR lpsz = sz; 
	int flag;

	flag = test_muliti_byte_scheme();
	if (flag == SCHEME_GBK )
	{
		utf8_to_utf16(psrc, lpwsz);
		WideCharToMultiByte(SCHEME_GBK, 0, lpwsz, -1, lpsz, 256, NULL, NULL); 
	} 
	else
	{
		lpsz = psrc;
	}
	return lpsz;
}

INT32S ui_filelist_init(UI_FILELIST * filelist,INT8S disk,INT8S * base_path,INT8S * file_type,INT8S scan,FILST_TYPE_t  list_type)
{
	char type[16];
	int i;
	strcpy(filelist->root, base_path);
	if(filelist->root[0] != 0)
	{
		int len = strlen(filelist->root);
		if(filelist->root[len - 1] != '\\')
		{
			filelist->root[len] = '\\';
			filelist->root[len + 1] = 0;
		}
	}
	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, "*.");
	for(i = 0; file_type[i] != 0 && file_type[i] != ' '; i++)
	{
		type[i] = file_type[i];
	}
	type[i] = 0;
	strcat(filelist->str, type);
	strcpy(filelist->type, type);

	filelist->total_num = 0;
	filelist->handle = FindFirstFile(file_map(filelist->str), &filelist->data);
	if(filelist->handle != INVALID_HANDLE_VALUE)
	{
		filelist->total_num++;
		while(FindNextFile(filelist->handle, &filelist->data))
		{
			filelist->total_num++;
		}
		FindClose(filelist->handle);
	}

	return 0;
}

INT32S ui_filelist_get_cur_filetype(UI_FILELIST * filelist,	INT32S  index)
{
	char * name;

	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, "*.");
	strcat(filelist->str, filelist->type);

	filelist->handle = FindFirstFile(file_map(filelist->str), &filelist->data);
	if(filelist->handle != INVALID_HANDLE_VALUE)
	{
		index--;
		for(; index >= 0; index--)
		{
			FindNextFile(filelist->handle, &filelist->data);
		}

	}
	
	if(filelist->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(filelist->handle);
		return FT_DIR;
	}

	name = filelist->data.cFileName;
	while(*name != 0 && *name++ != '.');
	if(*name != 0)
	{
		int i;
		for(i = 0; i < sizeof(ft_map) / sizeof(FT_MAP); i++)
		{
			if(strcmp(ft_map[i].ext, name) == 0)
			{
				FindClose(filelist->handle);
				return ft_map[i].type;
			}
		}
	}
	FindClose(filelist->handle);
	return FT_FILE;
}

INT8S * ui_filelist_get_cur_shortname(UI_FILELIST * filelist, int index)
{
	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, "*.");
	strcat(filelist->str, filelist->type);

	filelist->handle = FindFirstFile(file_map(filelist->str), &filelist->data);
	if(filelist->handle != INVALID_HANDLE_VALUE)
	{
		index--;
		for(; index >= 0; index--)
		{
			FindNextFile(filelist->handle, &filelist->data);
		}
		
	}

	FindClose(filelist->handle);
	return filelist->data.cFileName;
}

INT8S * ui_filelist_get_cur_filepath(UI_FILELIST * filelist,int index)
{
	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, "*.");
	strcat(filelist->str, filelist->type);

	filelist->handle = FindFirstFile(file_map(filelist->str), &filelist->data);
	if(filelist->handle != INVALID_HANDLE_VALUE)
	{
		index--;
		for(; index >= 0; index--)
		{
			FindNextFile(filelist->handle, &filelist->data);
		}
		
	}

	FindClose(filelist->handle);
	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, filelist->data.cFileName);
	return filelist->str;
}

static INT32S getTotal(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(swf_ext_ui_filelist.total_num);
	SWFEXT_FUNC_END();
}

static INT32S getPath(void * handle)
{
	INT8S * path;
	SWFEXT_FUNC_BEGIN(handle);
	path = ui_filelist_get_cur_workpath(&swf_ext_ui_filelist);
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

static INT32S setPath(void * handle)
{
	INT8S * path, * type;
	INT32S n;
	SWFEXT_FUNC_BEGIN(handle);
	n = Swfext_GetParamNum();
	path = Swfext_GetString();
	if(n > 1)
	{
		type = Swfext_GetString();
	}
	else
	{
		type = "*";
	}
	ui_filelist_init(&swf_ext_ui_filelist, path[0], path, type, 0,DIR_BORWSE_ALL);
	SWFEXT_FUNC_END();
}

static INT32S getFileType(void * handle)
{
	INT32S index, type;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	type = ui_filelist_get_cur_filetype(&swf_ext_ui_filelist, index);
	Swfext_PutNumber(type);
	SWFEXT_FUNC_END();
}

static INT32S getFileName(void * handle)
{
	char * file;
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	file = ui_filelist_get_cur_shortname(&swf_ext_ui_filelist, index);
	file = MultiByteToUTF8(file);
	Swfext_PutString(file);
	SWFEXT_FUNC_END();	
}

static INT32S getFilePath(void * handle)
{
	char * file;
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	file = ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	Swfext_PutString(file);
	SWFEXT_FUNC_END();	
}

static INT32S isLeapYear(INT32S year)
{
	if(year % 4 == 0)
	{
		if(year % 100 == 0 && year % 400 != 0)
		{
			return 0;
		}
		return 1;
	}
	return 0;
}

static INT8U normal_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static INT8U leap_month[]   = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int getDaysOfYear(INT32S year, INT32S month, INT32S day)
{
	int i, days = 0;
	if(isLeapYear(year))
	{
		for(i = 0; i < month; i++)
		{
			days += leap_month[i];
		}
	}
	else
	{
		for(i = 0; i < month; i++)
		{
			days += normal_month[i];
		}
	}
	days += day;
	return days;
}


static FLOAT32 time2long(TIME_INFO * info)
{
	INT32S  i, days;
	FLOAT32 tm;
	for(i = 1970, days = 0; i < 1980; i++)
	{
		days += 365 + isLeapYear(i);
	}
	for(i = 0; i < info->year; i++)
	{
		days += 365 + isLeapYear(i + 1980);
	}
	days += getDaysOfYear(i + 1980, info->month, info->date);
	tm = ((FLOAT32)(((days * 24 + info->hour) * 60 + info->minute) * 60 + info->second)) * 1000;
	return tm;
}

static INT32S getFileTime(void * handle)
{
	int index;
	TIME_INFO info;
	FILETIME * ftime;
	SYSTEMTIME systime; 

	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	ftime = &swf_ext_ui_filelist.data.ftCreationTime;
	FileTimeToSystemTime(ftime, &systime);
	info.year   = (INT8U)(systime.wYear - 1980);
	info.month  = (INT8U)systime.wMonth;
	info.date   = (INT8U)systime.wDay;
	info.hour   = (INT8U)systime.wHour;
	info.minute = (INT8U)systime.wMinute;
	info.second = (INT8U)systime.wSecond;
	Swfext_PutFloat(time2long(&info));
	SWFEXT_FUNC_END();
}

static INT32S getFileSize(void * handle)
{
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	ui_filelist_get_cur_filepath(&swf_ext_ui_filelist,index);
	Swfext_PutNumber(swf_ext_ui_filelist.data.nFileSizeLow);
	SWFEXT_FUNC_END();	
}

static INT32S enterDir(void * handle)
{
	int index;
	UI_FILELIST * filelist = &swf_ext_ui_filelist;

	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, "*");
	filelist->handle = FindFirstFile(file_map(filelist->str), &filelist->data);
	FindNextFile(filelist->handle, &filelist->data);
	for(; index >= 0; index--)
	{
		FindNextFile(filelist->handle, &filelist->data);
	}
	FindClose(filelist->handle);

	strcpy(filelist->str, filelist->root);
	strcat(filelist->str, filelist->data.cFileName);
	strcat(filelist->str, "\\");
	strcpy(filelist->root, filelist->str);
	SWFEXT_FUNC_END();
}

static INT32S exitDir(void * handle)
{
	int i, j, k;
	SWFEXT_FUNC_BEGIN(handle);
	for(i = 0, j = 0, k = 0; swf_ext_ui_filelist.root[i] != 0; i++)
	{
		if(swf_ext_ui_filelist.root[i] == '\\')
		{
			k = j;
			j = i + 1;
		}
	}
	if(k != j && k != 0)
	{
		swf_ext_ui_filelist.root[k + 1] = 0;
	}
	SWFEXT_FUNC_END();
}

static INT32S copyFile(void * handle)
{
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	printf("copy!\n");
	SWFEXT_FUNC_END();
}

static INT32S deleteFile(void * handle)
{
	int index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	printf("delete!\n");
	SWFEXT_FUNC_END();
}



SWF_FILELIST mtFilelist[SWF_MAX_FILELIST]; 

static void SWF_FilelistInit()
{
	int i;
	for(i=0;i<SWF_MAX_FILELIST;i++){
		mtFilelist[i].free=1;
	}
}

static int SWF_GetFreelistHandle()
{
	int i;
	for(i=0;i<SWF_MAX_FILELIST;i++){
		if(mtFilelist[i].free){
			mtFilelist[i].free=0;
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

static INT32S setPathNew(void * handle)
{
	INT8S * path;
	INT8S * filetype;
	INT32S n;
	int fileHandle=-1;
	int mode;
	int scan;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();

	if(n!=4){
		goto _SET_PATH_OUT;
	}
	
	fileHandle=SWF_GetFreelistHandle();

	if(fileHandle==-1){
		goto _SET_PATH_OUT;
	}

	
	path=Swfext_GetString();
//	path = UTF8ToMultiByte(path);
	filetype = Swfext_GetString();
	mode=Swfext_GetNumber();
	scan=Swfext_GetNumber();
	printf("scan = %d, mode = %d\n",scan,mode);
	ui_filelist_init(&mtFilelist[fileHandle].filelist, path[0], path, filetype, scan,mode);
	
_SET_PATH_OUT:
	Swfext_PutNumber(fileHandle);
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getScanTaskStatNew(int handle)"
* always return 3
*/
static INT32S getScanTaskStatNew(void * handle)
{
	int stat=3;
	int n;

	
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	
	if(n==0){
		goto _GET_STAT_OUT;
	}
	
	stat=3;
	
_GET_STAT_OUT:
	Swfext_PutNumber(stat);
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getPathNew(int handle,int index,int buf)"
*/

static INT32S getPathNew(void * handle)
{
	static INT8S * path=NULL;
	int index,flhandle;
	int n;
	
	int bufen;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n=Swfext_GetParamNum();
	//printf("param number==%d\n",n);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_PATH_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _GET_PATH_NEW_OUT;
	}
	
	index=Swfext_GetNumber();
	
	if(n==3){
		bufen=Swfext_GetNumber();
	}

	path = ui_filelist_get_cur_filepath(&mtFilelist[flhandle].filelist,index);
//	path = MultiByteToUTF8(path);
	//printf("path=%s\n",path);
	
	
_GET_PATH_NEW_OUT:
	
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "releasePathNew(int handle)"
*/
static INT32S releasePathNew(void * handle)
{
	int flhandle;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _RELEASE_PATH_NEW_OUT;
	}
	
	if(mtFilelist[flhandle].free){
		goto _RELEASE_PATH_NEW_OUT;
	}
	else{
		mtFilelist[flhandle].free=1;
	}
	
_RELEASE_PATH_NEW_OUT:
	
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getTotalNew(int handle)"
*/

static INT32S getTotalNew(void * handle)
{
	int total=0,flhandle;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();

	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_TOTAL_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _GET_TOTAL_NEW_OUT;
	}
	
	total = mtFilelist[flhandle].filelist.total_num;
	
_GET_TOTAL_NEW_OUT:
	
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getLongnameNew(int handle,int index)"
*/

static INT32S getLongnameNew(void * handle)
{
	static INT8S * path=NULL;
	int index,flhandle;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_PATH_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _GET_PATH_NEW_OUT;
	}
	
	index=Swfext_GetNumber();

	path = ui_filelist_get_cur_shortname(&mtFilelist[flhandle].filelist,index);
	
_GET_PATH_NEW_OUT:
	
	Swfext_PutString(path);
	SWFEXT_FUNC_END();
}

/**
* prototype should be "getFileSizeNew(int handle,int index)"
*/
static INT32S getFileSizeNew(void * handle)
{
	char * file;
	int index;
	int flhandle;
	INT32U size=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _GET_FILESIZE_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _GET_FILESIZE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	file = ui_filelist_get_cur_filepath(&mtFilelist[flhandle].filelist,index);
	size = mtFilelist[flhandle].filelist.data.nFileSizeLow;
	
_GET_FILESIZE_NEW_OUT:
	Swfext_PutNumber(size);
	SWFEXT_FUNC_END();	
}

/**
* prototype should be "copyFileNew(int handle,int index[,char *dest])"
*/
static INT32S copyFileNew(void * handle)
{
	char *dest,path[256];
	int index;
	int flhandle;
	INT32S err=0;
	int n;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	n=Swfext_GetParamNum();
	/**
	if n==2 : default to "C:\\"
	if n==3 : destination path pointed by the third parameter
	*/
	if(n!=2 && n!=3){
		goto _COPY_FILE_NEW_OUT;
	}
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _COPY_FILE_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _COPY_FILE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	
	if(n==2){
		sprintf(path,"C:\\");
	}
	else if(n==3){
		dest=Swfext_GetString();
		strcpy(path,dest);
	}
	else{
		sprintf(path,"C:\\");
	}
	
	err=0;
	
_COPY_FILE_NEW_OUT:
	Swfext_PutNumber(err);
	SWFEXT_FUNC_END();	
}

/**
* prototype should be "deleteFileNew(int handle,int index)"
*/
static INT32S deleteFileNew(void * handle)
{
	int index;
	int flhandle;
	INT32S err=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	flhandle=Swfext_GetNumber();
	if(flhandle<0 || flhandle>=SWF_MAX_FILELIST){
		goto _DELETE_FILE_NEW_OUT;
	}
	if(mtFilelist[flhandle].free){
		goto _DELETE_FILE_NEW_OUT;
	}
	
	index = Swfext_GetNumber();
	
	err=0;

_DELETE_FILE_NEW_OUT:
	Swfext_PutNumber(err);
	SWFEXT_FUNC_END();	
}



INT32S swfext_filelist_register(void)
{
	SWF_FilelistInit();
	SWFEXT_REGISTER("fl_setPath", setPath);
	SWFEXT_REGISTER("fl_getPath", getPath);
	SWFEXT_REGISTER("fl_getTotal", getTotal);
	SWFEXT_REGISTER("fl_getFileName", getFileName);
	SWFEXT_REGISTER("fl_getFileType", getFileType);
	SWFEXT_REGISTER("fl_getFilePath", getFilePath);
	SWFEXT_REGISTER("fl_getFileTime", getFileTime);
	SWFEXT_REGISTER("fl_getFileSize", getFileSize);
	SWFEXT_REGISTER("fl_enterDir", enterDir);
	SWFEXT_REGISTER("fl_exitDir", exitDir);
	SWFEXT_REGISTER("fl_copyFile", copyFile);
	SWFEXT_REGISTER("fl_deleteFile", deleteFile);

	/**
	* newly added for file search
	*/
	SWFEXT_REGISTER("fl_setPathNew", setPathNew);
	SWFEXT_REGISTER("fl_getScanTaskStatNew", getScanTaskStatNew);
	SWFEXT_REGISTER("fl_getPathNew", getPathNew);
	SWFEXT_REGISTER("fl_releasePathNew", releasePathNew);
	SWFEXT_REGISTER("fl_getTotalNew", getTotalNew);
	SWFEXT_REGISTER("fl_getLongnameNew", getLongnameNew);
	SWFEXT_REGISTER("fl_getFileSizeNew", getFileSizeNew);
	SWFEXT_REGISTER("fl_copyFileNew", copyFileNew);
	SWFEXT_REGISTER("fl_deleteFileNew", deleteFileNew);
	return 1;
}

