#ifndef SWF_OPT_H
#define SWF_OPT_H

#define SWF_MEM_STAT 1
#define BMP_CACHE_ENABLE 0
#define ZBUFFER_ENABLE 0
#define MASK_ENABLE 1
#define PARALLEL_ENABLE 1
#define INVALIDATE_RECT_ENABLE 1
#define WIDE_LINE 0
#define YUV_COLOR_SPACE
#define EN_EDIT_EMBEDFONT 0

#define USE_FREE_TYPE

/* 1207 supports up to mid quality, 1209 up to high quality */
#define AA_HIGH_QUALITY		3
#define AA_MID_QUALITY		2
#define AA_LOW_QUALITY		1
#define AA_NONE				0

#if AM_CHIP_ID == 1207 || AM_CHIP_ID == 1209 || AM_CHIP_ID == 1211
#define ANTI_ALIASING 1
#define ANTI_ALIASING_QUALITY AA_MID_QUALITY
#else
#define ANTI_ALIASING 0
#define ANTI_ALIASING_QUALITY AA_NONE
#endif


//#define EN_SOFT_KEYBOARD
//#define INPUT_METHOD
//#define CURSOR_ENABLE
//#define MOUSE_ICON
//#define EBOOK_TOUCH
//#define USE_MULTI_TEXTFORMAT
//#define EBOOK_TOUCH_SOUND

#define USE_WXW	1

#endif
