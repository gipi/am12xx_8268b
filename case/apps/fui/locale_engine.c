#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "swf_ext.h"
#include "locale_engine.h"
#include "fui_input.h"
#include "sys_cfg.h"
#include "ezcast_public.h"
#include "sys_resource.h"

#define LOCALE_DEBUG
#ifdef LOCALE_DEBUG
#define locale_infor(fmt,arg...)		printf("INFO[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)
#define locale_error(fmt,arg...)		printf("ERROR[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)
#else
#define locale_infor(fmt,arg...) do{}while(0);
#define locale_error(fmt,arg...) do{}while(0);
#endif

#define     OFFSET_DATA_WIDTH		sizeof(unsigned short)
#define		LANGUAGE_HEBREW_INDEX			(25)
#define		LANGUAGE_ARABIC_INDEX			(23)
#define		LANGUAGE_FARSI_INDEX			(27)

static PFUI_RES_STR g_pfrs = NULL;

#if 1
static char _upper(char c)
{
	if ( c >= 'a' && c <= 'z' )
		return c - 'a' + 'A'; 
	return c;
}


static int strnocasecmp (const char *cs, const char *ct)
{	
	register int __res = 0;	
	
	while (1) {	
		if ((__res =_upper(*cs) -_upper(*ct++)) != 0 || !*cs++)
			break;			
	}
	
	return __res;
}
static void set_utf16(char * str, int * index, unsigned short u16c)
{
	int i = *index;
	
	if(u16c >= 0x800)
	{
		// 3 byte
		*index = i + 3;
		str[i] = (u16c >> 12) | 0xe0;
		str[i+1] = ((u16c >> 6) & 0x3f) | 0x80;
		str[i+2] = (u16c & 0x3f) | 0x80;
	}
	else if(u16c >= 0x80)
	{
		// 2 byte
		*index = i + 2;
		str[i] = ((u16c >> 6) & 0x1f) | 0xc0;
		str[i+1] = (u16c & 0x3f) | 0x80;
	}
	else
	{
		// 1 byte
		*index = i + 1;
		str[i] = (unsigned char)u16c;
	}
}

static int utf16_to_utf8(unsigned short * u16str, char * u8str)
{
	int i, j;
	i = 0, j = 0;
	
	do {
		set_utf16(u8str, &i, u16str[j]);
	} while (u16str[j++] != 0);
	
	return i;
}

static void *open_res(char *filename)
{	
	PFUI_RES_STR pfrs = NULL;
	stream_input_t *pb = create_fui_input(filename);
	if(pb != NULL)
	{		
		int nLen = 0;		
		
		pfrs = (PFUI_RES_STR)malloc(sizeof(FUI_RES_STR));		
		pb->read(pb, (int)&pfrs->header, sizeof(FUI_RES_HEADER) - 4);

		nLen = pfrs->header.LangCnt * MAX_LANG_ID_LEN;
		pfrs->header.pLangId = (char*)malloc(nLen);
		pb->read(pb, (int)pfrs->header.pLangId, nLen);

		nLen = sizeof(FUI_KEYS1_TAB) * pfrs->header.SheetCnt;		
		pfrs->pKeys1 = (PFUI_KEYS1_TAB)malloc(nLen);
		pb->read(pb, (unsigned int)pfrs->pKeys1, nLen);	


		pfrs->pData = (char*)malloc(MAX_TEXT_LEN); 
		pfrs->pData_len = MAX_TEXT_LEN;
		pfrs->pb = pb;	
		if(pfrs->header.encode!=2){ // 2 utf-8
			printf("%s,%d: Not Support yet!\n",__FILE__,__LINE__);
		}

	}
	g_pfrs = pfrs;
	return pfrs;
}

static int load_language(const char *AppName, const char *LangId)
{
	PFUI_RES_STR pfrs = g_pfrs;	

	if(pfrs != NULL)
	{	
		unsigned short i;	
		
		pfrs->AppIdx = -1;
		pfrs->LangIdx = -1;
		
		for(i = 0; i < pfrs->header.SheetCnt; i++)
		{
			if(strnocasecmp(pfrs->pKeys1[i].AppName, AppName) == 0)
			{
				printf("\n");
				EZCASTLOG("===================================================\n");
				EZCASTLOG("			AppName: %s\n", AppName);
				EZCASTLOG("===================================================\n\n");
				pfrs->AppIdx = i;
				break;
			}
		}			
		if(pfrs->AppIdx == -1)
		{
			printf("%s[%d]: Cann't find appname(%s)!\n", __FILE__, __LINE__, AppName);
			return 0;
		}

		for(i = 0; i < pfrs->header.LangCnt; i++)
		{
			if(strnocasecmp(LangId, pfrs->header.pLangId + i * MAX_LANG_ID_LEN) == 0)
			{
				pfrs->LangIdx = i;
				break;
			}
		}
		if(pfrs->LangIdx == -1)
		{
			printf("%s[%d]: Cann't find language(%s)!\n", __FILE__, __LINE__, LangId);
			return 0;
		}
		
		return 1;
	}
	return 0;
}

static char * load_string(const char *StrId)
{
	PFUI_RES_STR pfrs = g_pfrs;
	if(pfrs != NULL)
	{	
		if(pfrs->AppIdx != -1 && pfrs->LangIdx != -1)
		{		
			unsigned short i = 0;
			PFUI_RES_TABLE pfrt = NULL;
			PFUI_KEYS1_TAB pfk1t = &pfrs->pKeys1[pfrs->AppIdx];
			stream_input_t *pb = pfrs->pb;	
			DWORD dwKeys2BlockLen = (pfrs->header.LangCnt*OFFSET_DATA_WIDTH)+MAX_ID_NAME_LEN+sizeof(DWORD);
			DWORD dwLen = dwKeys2BlockLen*pfk1t->StrCnt;
			unsigned char *pBuf = (unsigned char*)malloc(dwLen);
			if(pBuf == NULL)
			{
				printf("%s[%d]: malloc failed!\n", __FILE__, __LINE__);
				return NULL;
			}
			pb->seek(pb, pfk1t->StartAddr, DSEEK_SET);
			pb->read(pb, (int)pBuf, dwLen);
			
			for(i = 0; i < pfk1t->StrCnt; i++)
			{
				pfrt = (PFUI_RES_TABLE)(pBuf + (dwKeys2BlockLen * i));			
				if(strnocasecmp(StrId, pfrt->StrID) == 0)
				{	
					unsigned short *pOffset =  (unsigned short*)(pBuf + (dwKeys2BlockLen * i)+MAX_ID_NAME_LEN+sizeof(DWORD));
					unsigned short *pData;
					if(pfrs->LangIdx == 0)
					{
						pb->seek(pb, pfrt->dwStartAddr, DSEEK_SET);
						//pData = (unsigned short *)malloc(pOffset[0]);
						//pb->read(pb, (int)pData, pOffset[0]);
						dwLen  = pOffset[0];						
					}
					else
					{
						dwLen = pOffset[pfrs->LangIdx-1];
						pb->seek(pb, pfrt->dwStartAddr + dwLen, DSEEK_SET);
						dwLen = pOffset[pfrs->LangIdx] - pOffset[pfrs->LangIdx-1];
						//pData = (unsigned short *)malloc(dwLen);
						//pb->read(pb, (int)pData, dwLen);
					}
					if(dwLen > pfrs->pData_len)
					{
						if(pfrs->pData!= NULL)
							free(pfrs->pData);
						//Mos: Change pData size to 4 multipliers of pData_len, cause UTF-8 longest size is 4 bytes
						//Add 4 byte space at tail for Null character
						//pfrs->pData_len = pfrs->pData_len *2;
						pfrs->pData_len = (pfrs->pData_len *4) + 4;
						pfrs->pData = malloc(pfrs->pData_len);
						
					}
					if(pfrs->pData != NULL)
						pb->read(pb, (int)pfrs->pData, dwLen);
					else
						printf("%s[%d]:  Error:  malloc fail !", __FILE__, __LINE__);

					free(pBuf);	
					//utf16_to_utf8(pData, pfrs->pData);
					//free(pData);
#if 1//!EZCAST_ENABLE
					//Mos: revert conv_arabic function call, cause this should be commit with arabic font, otherwise UI will missing character
					if(pfrs->LangIdx == LANGUAGE_ARABIC_INDEX || pfrs->LangIdx == LAN_ARABIC){
						conv_arabic(pfrs->pData, pfrs->pData_len);
					}
                    if(pfrs->LangIdx == LANGUAGE_FARSI_INDEX){
                        conv_farsi(pfrs->pData, pfrs->pData_len);
                    }
#endif

					if(pfrs->LangIdx == LANGUAGE_HEBREW_INDEX || pfrs->LangIdx == LANGUAGE_ARABIC_INDEX || LANGUAGE_FARSI_INDEX == pfrs->LangIdx|| pfrs->LangIdx == LAN_ARABIC){
						bidi_piece(pfrs->pData);
					}

					return pfrs->pData;				
				}		
			}
			free(pBuf);		
		}
	}
	return NULL;
}

static void close_res(void * res)
{
	PFUI_RES_STR pfrs = g_pfrs;
	if(pfrs != NULL)
	{
		stream_input_t * pb = pfrs->pb;
		pb->dispose(pb);
		free(pfrs->header.pLangId);
		free(pfrs->pKeys1);
		free(pfrs->pData);
		free(pfrs);
	}
}

/**
@brief open the res 
@param[in] filepath	: where the current running swf store, make sure the fui.res had been store at the same place
@return 
	- -1	: failed
	- 0	: succeed
**/
static int  __locale_open_res(char *filepath)
{
	if(g_pfrs == NULL){
		char buf[256], * p, * slice;
		strcpy(buf, filepath);
		for(p = buf, slice = buf; *p != 0; p++)
		{
			if(*p == '/')
			{
				slice = p + 1;
			}
		}
		*slice = 0;
		strcat(buf, "fui.res");
		open_res(buf);
	}	
	if(g_pfrs==NULL)
		return -1;
	else
		return 0;
}

/**
@brief get language num after calling __locale_open_res()
**/
static int __locale_get_lang_num()
{
	int num=-1;
	if(g_pfrs==NULL){
		locale_error("Crazy:The FUI RES Open Error!");
		num = -1;
	}
	else
		num = g_pfrs->header.LangCnt;
	return num;
}

#endif

#if 1

/**
@brief get the language num which the system support
@param[in] handle	: the handle of the swf context
@return 
	- -1		: failed to open fui.res
	- others	: the number of language which the system support 
**/

int locale_get_lange_num(void*handle)
{
	int num=0;
	char * filepath =NULL;
	filepath = SWF_GetFilePath(handle);
	__locale_open_res(filepath);
	num = __locale_get_lang_num();
	return num;
}

/**
@brief get the language str according to the idx
@param[in] idx	: the idx of the language
@param[in] handle	: the handle of the swf context
@return 
	- NULL	: the idx is out of range or the res opened fail
	- others	: the language str according to the idx
**/
char * locale_get_lange_str(int idx,void*handle)
{
	int numtotal=0;
	numtotal = locale_get_lange_num(handle);
	if(numtotal<0)
		return NULL;
	if(idx>=numtotal){
		locale_error("Crazy: idx=%d larger than Num total=%d",idx,numtotal);
		return NULL;
	}
	else
		return g_pfrs->header.pLangId +idx * MAX_LANG_ID_LEN;
		
}


int locale_get_default_langidx()
{
	char respath[256]="";
	sprintf(respath,"%s/%s",AM_CASE_SWF_DIR,"fui.res");
	if(__locale_open_res(respath)==-1){
		locale_error("Open Res Failed!");
		return -1;
	}
	else
		return (int)(g_pfrs->header.default_lang_id);
}

#endif

static int set_language(void * handle)
{
	char * filepath, * name, * lang;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);

	filepath = SWF_GetFilePath(handle);

	
	__locale_open_res(filepath);

	for(name = filepath; *filepath != 0; filepath++)
	{
		if(*filepath == '/')
		{
			name = filepath + 1;
		}
	}

	lang = Swfext_GetString();

	ret  = load_language(name, lang);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_set_language(char *lang)
{
	char  * name;
	int ret;

	//filepath = SWF_GetFilePath(handle);
	char * filepath="/am7x/case/data/setting";
	
	__locale_open_res(filepath);

	for(name = filepath; *filepath != 0; filepath++)
	{
		if(*filepath == '/')
		{
			name = filepath + 1;
		}
	}
	ret  = load_language(name, lang);
	return ret;
}
int ezcast_set_language_swf_name(char *lang,char *name)
{
	int ret;
	//filepath = SWF_GetFilePath(handle);
	char * filepath="/am7x/case/data/setting";
	__locale_open_res(filepath);
	ret  = load_language(name, lang);
	printf("[%s]name = %s,  ret = %d\n", __func__, name, ret);
	return ret;
}
#endif
static int set_language_swf_name(void * handle)
{
	char * filepath, * name, * lang;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);

	filepath = SWF_GetFilePath(handle);

	
	__locale_open_res(filepath);
	
	lang = Swfext_GetString();
	name = Swfext_GetString();

	ret  = load_language(name, lang);
	printf("[%s]name = %s,  ret = %d\n", __func__, name, ret);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int get_string(void * handle)
{
	char * str, * ids;
	SWFEXT_FUNC_BEGIN(handle);
	ids = Swfext_GetString();
	str = load_string(ids);
	if(str)
	{
		Swfext_PutString(str);
	}
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
char * ezcast_get_string(char * ids)
{
	static char * str=NULL;
	str = load_string(ids);
	if(str)
	{
		return str;
	}
}
#endif
int swfext_locale_register(void)
{
	SWFEXT_REGISTER("locale_loadLanguage", set_language);
	SWFEXT_REGISTER("locale_loadLanguage_swf_name", set_language_swf_name);
	SWFEXT_REGISTER("locale_loadString", get_string);
	return 0;
}
