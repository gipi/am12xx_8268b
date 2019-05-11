
#ifndef _TTFONT_H_
#define _TTFONT_H_

#include "swf.h"
#include <ft2build.h>
#include FT_FREETYPE_H

/** Truetype字形缓冲大小 */
#if 0
#define SBIT_HASH_SIZE (1024-1)
#else
#define SBIT_HASH_SIZE (1024)
#endif
#define PATH_MAX 1024

#define SBIT_CACHE_MAX 1024*1024

#ifndef true
#define true 1
#endif

#ifndef bool
#define bool	int
#endif

#ifndef false
#define false 0
#endif
#ifndef byte
typedef unsigned char byte;
#endif
#ifndef word
typedef unsigned short word;
#endif
#ifndef dword
typedef unsigned long dword;
#endif

typedef dword pixel;

/** 缓冲字形位图结构 */
typedef struct Cache_Bitmap_
{
	int width;
	int height;
	int left;
	int top;
	char format;
	short max_grays;
	int pitch;
	unsigned char *buffer;
} Cache_Bitmap;

/** 缓冲字形结构 */
typedef struct SBit_HashItem_
{
	unsigned long ucs_code;
	int glyph_index;
	int size;
	bool anti_alias;
	bool cleartype;
	bool embolden;
	int xadvance;
	int yadvance;
	Cache_Bitmap bitmap;
} SBit_HashItem;

/** Truetype字体结构 */
typedef struct _ttf
{
	FT_Library library;
	FT_Face face;

	char *fontName;
	bool antiAlias;
	bool cleartype;
	bool embolden;
	int pixelSize;

	SBit_HashItem sbitHashRoot[SBIT_HASH_SIZE];
	int cacheSize;
	int cachePop;

	char fnpath[PATH_MAX];
	int fileSize;
	byte *fileBuffer;
} t_ttf, *p_ttf;

/**
 * 打开TTF字体
 *
 * @param filename TTF文件名
 * @param size 预设的字体大小
 * @param load2mem 是否加载到内存
 *
 * @return 描述TTF的指针
 * - NULL 失败
 */
extern p_ttf ttf_open(const char *filename, int size, bool load2mem);

/**
 * 从指定数据中打开TTF字体
 *
 * @param ttfBuf TTF字体数据
 * @param ttfLength TTF字体数据大小，以字节计
 * @param pixelSize 预设的字体大小
 * @param ttfName TTF字体名
 *
 * @return 描述TTF的指针
 * - NULL 失败
 */
extern p_ttf ttf_open_buffer(void *ttfBuf, size_t ttfLength, int pixelSize,
							 const char *ttfName);

/**
 * 释放TTF字体
 *
 * @param ttf ttf指针
 */
extern void ttf_close(p_ttf ttf);

/**
 * 设置TTF字体大小
 *
 * @param ttf ttf指针
 * @param size 字体大小
 *
 * @return 是否成功
 */
extern bool ttf_set_pixel_size(p_ttf ttf, int size);

/**
 * 设置TTF是否启用抗锯齿效果
 *
 * @param ttf ttf指针
 * @param aa 是否启用抗锯齿效果
 */
extern void ttf_set_anti_alias(p_ttf ttf, bool aa);

/**
 * 设置TTF是否启用ClearType(Sub-Pixel LCD优化效果)
 *
 * @param ttf ttf指针
 * @param cleartype 是否启用cleartype
 */
extern void ttf_set_cleartype(p_ttf ttf, bool cleartype);

/**
 * 设置TTF是否启用字体加粗
 *
 * @param ttf ttf指针
 * @param embolden 是否启用字体加粗
 */
extern void ttf_set_embolden(p_ttf ttf, bool embolden);



/** TTF加锁 */
extern void ttf_lock(void);

/** TTF解锁 */
extern void ttf_unlock(void);

/** 初始化TTF模块 */
extern void ttf_init(void);

/** 释放TTF模块 */
extern void ttf_free(void);



//int show_string_in_osd(char *gb, char *string_bitmap, int w, int h);

void Set_vars(FT_Face *face);
int set_pixel_size_ex(int size);

#endif
