#ifndef SWF_LOCALE_20090902_H
#define SWF_LOCALE_20090902_H

#include "swf_types.h"
#include "act_plugin.h"

#define		MAX_ID_NAME_LEN				60
#define		MAX_TEXT_LEN				256
#define		MAX_APP_NAME_LEN			16
#define		MAX_LANG_ID_LEN				6

#define DWORD 	unsigned long

#pragma pack(1)
// 文件头结构 (16)Byte
typedef struct tagFUI_RES_HEADER
{
	char Magic[2];
	char Version[2];
	DWORD dwFileLen;
	unsigned short SheetCnt;    // 字符串总数  (2byte)	
	unsigned short LangCnt;
	char Reserved[4];
	char *pLangId;    // 记录所有的语言id （ 字符信息描述）
}FUI_RES_HEADER, *PFUI_RES_HEADER;


// 一级检索表结构 ((16+2+4)* SheetCnt) Byte
typedef struct tagFUI_KEYS1_TAB
{
	char AppName[MAX_APP_NAME_LEN];    //AP名（表名）， 属于主检索关键字
	unsigned short StrCnt;  // 字符串数（表示该AP共有多少个字符串）。	
	DWORD StartAddr;        // 指向二级索引的起始地址（相对于文件头开始）
}FUI_KEYS1_TAB, *PFUI_KEYS1_TAB;

// 二级检索表结构
typedef struct tagFUI_RES_TABLE
{
	char   StrID[MAX_ID_NAME_LEN];    
	DWORD  dwStartAddr;	   // 数据其实地址
	unsigned short *pOffset;    // 各语言的相对偏移值
}FUI_RES_TABLE, *PFUI_RES_TABLE;

typedef struct tagFUI_RES_HASH_TAB
{
	int    hash;
	DWORD  OffAddr;	
}FUI_RES_HASH_TAB, *PFUI_RES_HASH_TAB;

typedef struct tagFUI_RES_STR{
	stream_input_t  *pb;
	FUI_RES_HEADER	header;
	PFUI_KEYS1_TAB  pKeys1;   // 记录一级检索的所有数据。
	int		LangIdx;   //记录当前语言的索引值，即表示第几种语言.
	int     AppIdx;    //记录当前APP的索引值，即表示第几个APP.
	char *  pData;
}FUI_RES_STR, *PFUI_RES_STR;

#pragma pack()

void * open_res(char *filename);
void close_res();

#endif