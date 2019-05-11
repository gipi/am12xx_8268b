#include "swfext.h"
#include "swf.h"
#include "ebook.h"
#include "fui_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>

#define		CHARS_PAR_LINE		30
#define		LC_ALL				0


static int state = 0;
static int width = 0;
static int bookmark = 0;
static int color = 0;
static int bgcolor = 0xFFFFFF;
static int size = 16;
static ebook_info_t ebook;

static char str[] = "Circuit diagrams and other information relating to products of Actions Semiconductor Company, Ltd.";
static INT8U buf[MAX_BUF_SIZE];


static INT32S EbookInit()
{
	memset(&ebook, 0, sizeof(ebook));
	
	return 0;
}

static INT32S EBookGetFileType()
{
	int nTxtType = EBOOK_MBCS;
	if(ebook.pb != NULL)
	{		
		ebook.pb->seek(ebook.pb, 0, SEEK_SET);
		ebook.pb->read(ebook.pb, (int)buf, MAX_BUF_SIZE);

		if((buf[0] == 0xff)&&(buf[1] == 0xfe))
		{
			nTxtType = EBOOK_U16_LIT;
			ebook.nMagicLen = 2;
			printf("TXT type == U16 little\n");
		}
		else if((buf[0] == 0xfe)&&(buf[1] == 0xff))
		{
			nTxtType  = EBOOK_U16_BIG;
			ebook.nMagicLen = 2;
			printf("TXT type == U16 big\n");
		}
		else  if((buf[0] == 0xef)&&(buf[1] == 0xbb)	&& (buf[2] == 0xbf))
		{
			nTxtType  = EBOOK_UTF8;
			ebook.nMagicLen = 3;
			printf("TXT type == UTF8\n");
		}
		else
		{		
			ebook.nMagicLen = 0;
			printf("TXT type == MBCS\n");
		}	
	}
	return nTxtType;
}

static INT32S Open(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(EbookInit() == 0)
	{
		char *file = Swfext_GetString();
		ebook.TxtW = Swfext_GetNumber();
		printf("open ebook %s width = %d\n", file, ebook.TxtW);
		ebook.pb = create_fui_input(file);
		if(ebook.pb != NULL)
		{
			ebook.type = EBookGetFileType();
		}
	}
	Swfext_PutNumber(ebook.pb != NULL?1:0);

	SWFEXT_FUNC_END();	
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("ebook close\n");	
	if(ebook.pb != NULL)
	{
		ebook.pb->dispose(ebook.pb);
		ebook.pb = NULL;
	}

	SWFEXT_FUNC_END();	
}

static INT32S GetTotalRows(void * handle)
{
	int nTotolRows = 1;
	SWFEXT_FUNC_BEGIN(handle);
	if(ebook.pb != NULL)
	{
		long len = 0;
		ebook.pb->seek(ebook.pb, 0, SEEK_END);
		len = ebook.pb->tell(ebook.pb);
		ebook.pb->seek(ebook.pb, 0, SEEK_SET);	
		
		nTotolRows = (len+CHARS_PAR_LINE-1) / CHARS_PAR_LINE;
	}
	
	Swfext_PutNumber(nTotolRows);
	SWFEXT_FUNC_END();	
}

static INT32S GetRow(void * handle)
{
	int nLine;	
	SWFEXT_FUNC_BEGIN(handle);
	nLine = Swfext_GetNumber();
	if(ebook.pb != NULL)
	{
		wchar_t unibuf[MAX_BUF_SIZE];
		ebook.nCurLine = nLine;
		ebook.pb->seek(ebook.pb, nLine*CHARS_PAR_LINE+ebook.nMagicLen, SEEK_SET);
		memset(buf, 0, sizeof(buf));
		ebook.pb->read(ebook.pb, buf, CHARS_PAR_LINE);		
		switch(ebook.type)
		{
		case EBOOK_MBCS:
			{								
				char *temp=setlocale(LC_ALL,".936"); // 默认简中
				puts(temp);
				memset(unibuf, 0, MAX_BUF_SIZE);
				if(-1==mbstowcs(unibuf, buf, CHARS_PAR_LINE))
				{
					perror("mbstowcs");					
				} 
				else
				{							
					printf("uni= 0x%x, 0x%x\n",unibuf[0], unibuf[1]);
					memset(buf, 0, sizeof(buf));
					utf16_to_utf8(unibuf,buf);				
				}
			}
			break;
		case EBOOK_U16_BIG:
		case EBOOK_U16_LIT:
			{			
				memcpy(unibuf, buf, CHARS_PAR_LINE);
				utf16_to_utf8(unibuf,buf);				
			}
			break;
		case EBOOK_UTF8:	// 直接显示		
		default:			
			break;
		}		
	}		
	Swfext_PutString(buf);
	SWFEXT_FUNC_END();	
}

static INT32S GetStatus(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(state);
	SWFEXT_FUNC_END();	
}

static INT32S GetBookMark(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bookmark);
	SWFEXT_FUNC_END();	
}

static INT32S SetBookMark(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	bookmark = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

static INT32S GetColor(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(color);
	SWFEXT_FUNC_END();	
}

static INT32S SetColor(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	color = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

static INT32S GetBGColor(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bgcolor);
	SWFEXT_FUNC_END();	
}

static INT32S SetBGColor(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	bgcolor = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

static INT32S GetFontSize(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(size);
	SWFEXT_FUNC_END();	
}

static INT32S SetFontSize(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	size = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

INT32S swfext_ebook_register(void)
{
	SWFEXT_REGISTER("eb_Open", Open);
	SWFEXT_REGISTER("eb_Close", Close);
	SWFEXT_REGISTER("eb_GetTotalRows", GetTotalRows);
	SWFEXT_REGISTER("eb_GetRow", GetRow);
	SWFEXT_REGISTER("eb_GetStatus", GetStatus);
	SWFEXT_REGISTER("eb_GetBookMark", GetBookMark);
	SWFEXT_REGISTER("eb_SetBookMark", SetBookMark);
	SWFEXT_REGISTER("eb_GetColor", GetColor);
	SWFEXT_REGISTER("eb_SetColor", SetColor);
	SWFEXT_REGISTER("eb_GetBGColor", GetBGColor);
	SWFEXT_REGISTER("eb_SetBGColor", SetBGColor);
	SWFEXT_REGISTER("eb_GetFontSize", GetFontSize);
	SWFEXT_REGISTER("eb_SetFontSize", SetFontSize);
	return 0;
}
