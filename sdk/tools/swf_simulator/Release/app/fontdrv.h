#ifndef FONTDRV_H
#define FONTDRV_H

#include "swf_types.h"
#include "act_plugin.h"

typedef struct {
	unsigned short First;         /* first character               */
	unsigned short Last;          /* last character                */
	unsigned long  OffAddr;		  /* 指向的是当前SECTION包含的 UFL_CHAR_INFO第一个字符信息的起始地址 */
} UFL_SECTION_INF;

typedef struct {
	unsigned char magic[4];	//'U', 'F', 'L', X---Unicode Font Library,
	//X: 表示版本号。分高低4位。如 0x10表示 Ver 1.0
	unsigned long Size;		/* File total size */
	unsigned char nSection; // 共分几段数据。
	unsigned char YSize;    /* height of font  */  
	unsigned char XMag;     /* magnification x */
	unsigned char YMag;		/* magnification y */
	unsigned char COMP;		/* compress flag */
	unsigned char reserved[3];
} UFL_head_t;

typedef struct {
	stream_input_t  *pb;
	UFL_head_t info;
	UFL_SECTION_INF *section;
	unsigned char *buf;
} UFL_handle_t;

void * open_font(char * filename);
unsigned short get_utf16(char * str, int * index);
void   set_utf16(char * str, int * index, unsigned short u16c);
void * select_font(SWF_CONTEXT * ctx, int size);
int    get_font(void * handle, unsigned short code, unsigned char * data);
int    get_font_width(void * handle, unsigned short code);
void   close_font(void * font);
int    utf8_to_utf16(char * u8str, unsigned short * u16str);
int    utf16_to_utf8(unsigned short * u16str, char * u8str);

//extern void tff_display_horz(int mode, SWF_DEC * s, unsigned int  color, int *x, int *y, unsigned short c, unsigned int height, SWF_RECT * boundary, unsigned char * z_buffer, int depth, unsigned int *previous);
//extern void tff_display_horz(int mode, short * currFrame, int output_width, unsigned int  color, int *x, int *y, unsigned short c, unsigned int height, SWF_RECT * boundary, unsigned char * z_buffer, int depth, unsigned int *previous);
extern void tff_display_horz(int mode, short * currFrame, int output_width, unsigned int  color, int *x, int *y, unsigned short c, unsigned int height, SWF_RECT * boundary, unsigned char * z_buffer, int depth, unsigned int *previous,int z_buffer_mode);

extern int ttfont_load();
extern int ttfont_exit();
extern int get_ttfont_width(unsigned short code, int height);
extern int ttfont_load_new(char *filename);
extern int ttfont_restore();
#endif
