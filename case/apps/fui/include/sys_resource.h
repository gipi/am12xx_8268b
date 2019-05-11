#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include "iconv.h"


#define LAN_GB2312		0
#define LAN_UTF8			1
#define LAN_UTF16LE 		2
#define LAN_UTF16BE		3
#define LAN_ASCII		4
#define LAN_CP874		5
#define LAN_CP932		6
#define LAN_CP949		7
#define LAN_CP950		8
#define LAN_CP1250		9
#define LAN_CP1251		10
#define LAN_CP1252		11
#define LAN_CP1253		12
#define LAN_CP1254		13
#define LAN_CP1255		14
#define LAN_CP1256		15
#define LAN_CP1257		16
#define LAN_CP1258		17

typedef struct charsetchg_info_s{
	int src_charset;
	int dest_charset;
	char *inputbuf;
	size_t *inputbuflen;
	char *outputbuf;
	size_t *outputbuflen;
}charsetchg_info_t;

typedef enum lang_enum{
	LAN_ENGLISH,
	LAN_FRENCH,
	LAN_GERMAN,
	LAN_SPANISH,
	LAN_PORTUGUESE,
	LAN_CHS,
	LAN_CHT,
	LAN_ITALIAN,
	LAN_NORWEGIAN,
	LAN_SWEDISH,
	LAN_DUTCH,
	LAN_RUSSIAN,
	LAN_POLISH,
	LAN_FINNISH,
	LAN_GREEK,
	LAN_KOREAN,
	LAN_HUNGARIAN,
	LAN_CZECH,
	LAN_ARABIC,
	LAN_TURKISH,
}sys_lang;

#endif