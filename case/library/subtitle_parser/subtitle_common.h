#ifndef _SUBTITLE_COMMON_H_
#define _SUBTITLE_COMMON_H_
//#define WINDOW_PC

#include "subtitle_parser.h"

#define CONVERT_UTF_V1 0 // test
#define CONVERT_UTF_V0 (1-CONVERT_UTF_V1)

#if CONVERT_UTF_V1
//
// http://llvm.org/svn/llvm-project/llvm/trunk/include/llvm/Support/ConvertUTF.h
// http://llvm.org/svn/llvm-project/llvm/trunk/include/llvm/Support/ConvertUTF.c
//

#include "ConvertUTF.h"

#endif

#define IS_ARROW(pos) ((*(pos))==0x2D&&(*((pos)+1))==0x2D&&(*((pos)+2))==0x3E)
#define IS_NEW_LINE(pos) ((*(pos))==0x0D&&(*((pos)+1))==0x0A)
#define IS_COLON(pos) (*(pos)==0x3A)
#define IS_COMMA(pos) (*(pos)==0x2C)
#define IS_SPACE(pos) (*(pos)==0x20)
#define IS_DIGITAL(pos) (*(pos)>=0x30 && *(pos)<=0x39)

#define SUB_MAX_TEXT 12

typedef struct {
	
    int lines;
	
    unsigned long start;
    unsigned long end;
    
    char *text[SUB_MAX_TEXT];
    double endpts[SUB_MAX_TEXT];
    unsigned char alignment;
} subtitle;

unsigned char av_clip_uint8(int a);

#ifdef WINDOW_PC
char strncasecmp(const char*strsrc,const char* strdes,int size);
#endif

int get_char(parser_info_t *parser_info,void*fp,char *c);
int get_filename(char*filename,char*namebuf,int size);

int TransformLineUtf16beToUtf16le(char *lineBuf,const int lineLen);
int TransformLineUtf16ToUtf8(short int * utf16StrBuf, const int utf16StrSize);
int utf16_to_utf8(short int * u16str, char * u8str, const int u8str_size);
int hunsec2time(struct time_line *time,unsigned long hunsec);
unsigned long time2hunsec(struct time_line *time);
int get_line_info(subtitle *cursub,line_info_t *line_info,char*strbuf,int len);
unsigned int get_file_charset(parser_info_t *parser_info);

///////////////////////////////////////////////////////////////////////

typedef enum
{
	SUBTITLE_TEST = -1,
	
	SUBTITLE_MESSAGE = 0,
	SUBTITLE_ERROR = 1,
}SUBTITLE_MSG_CODE;

void PrintMsg(const char *function,const int line,SUBTITLE_MSG_CODE MsgCode, char *Msg);
#define MSG_P(MsgCode,Msg) PrintMsg(__FUNCTION__,__LINE__,MsgCode,Msg)

#endif
