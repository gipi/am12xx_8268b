#include "swfext.h"
#include "locale.h"
#include "fui_input.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#define OFFSET_DATA_WIDTH		sizeof(unsigned short)

static PFUI_RES_STR g_pfrs = NULL;

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

static int utf16_to_utf8(INT16U * u16str, char * u8str)
{
	int i, j;
	i = 0, j = 0;
	do 
	{
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
		pb->read(pb, (int)pfrs->pKeys1, nLen);	

		pfrs->pData = (char*)malloc(MAX_TEXT_LEN); 
		pfrs->pb = pb;		
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
			if(strcmp(pfrs->pKeys1[i].AppName, AppName) == 0)
			{				
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
			if(strcmp(LangId, pfrs->header.pLangId + i * MAX_LANG_ID_LEN) == 0)
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
				if(strcmp(StrId, pfrt->StrID) == 0)
				{	
					unsigned short *pOffset =  (unsigned short*)(pBuf + (dwKeys2BlockLen * i)+MAX_ID_NAME_LEN+sizeof(DWORD));
					unsigned short *pData;
					if(pfrs->LangIdx == 0)
					{
						pb->seek(pb, pfrt->dwStartAddr, DSEEK_SET);
						pData = (unsigned short *)malloc(pOffset[0]);
						pb->read(pb, (int)pData, pOffset[0]);
					}
					else
					{
						dwLen = pOffset[pfrs->LangIdx-1];
						pb->seek(pb, pfrt->dwStartAddr + dwLen, DSEEK_SET);
						dwLen = pOffset[pfrs->LangIdx] - pOffset[pfrs->LangIdx-1];
						pData = (unsigned short *)malloc(dwLen);
						pb->read(pb, (int)pData, dwLen);
					}
					free(pBuf);	
					utf16_to_utf8(pData, pfrs->pData);
					free(pData);
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

static INT32S loadLanguage(void * handle)
{
	char * filepath, * name, * lang;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);

	filepath = SWF_GetFilePath(handle);
	if(g_pfrs == NULL)
	{
		char buf[256], * p, * slice;
		strcpy(buf, filepath);
		for(p = buf, slice = buf; *p != 0; p++)
		{
			if(*p == '\\')
			{
				slice = p + 1;
			}
		}
		*slice = 0;
		strcat(buf, "fui.res");
		open_res(buf);
	}

	for(name = filepath; *filepath != 0; filepath++)
	{
		if(*filepath == '\\')
		{
			name = filepath + 1;
		}
	}

	lang = Swfext_GetString();
	ret  = load_language(name, lang);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static INT32S loadString(void * handle)
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

INT32S swfext_locale_register(void)
{
	SWFEXT_REGISTER("locale_loadLanguage", loadLanguage);
	SWFEXT_REGISTER("locale_loadString", loadString);
	return 0;
}
