#include "ft2build.h"
//#include <stdio.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PATH_NAME_LEN 32
#define RES_NUM		12
#define PELS_BYTES	3
#define CHAR_NUM_MAX 5
#define SOFT_KEY_MAX 128
#define BOTTON_WIDTH 36
#define BOTTON_HEIGHT 46
#define KEY_NUM		61
#define KEY_LINE1	14
#define KEY_LINE2	14
#define KEY_LINE3	13
#define KEY_LINE4	12
#define KEY_LINE5	8
#define FRONT_SIZE_STR	(14+2)
#define FRONT_SIZE_ASC	(18+2)
#define FRONT_SIZE_SPC	(11+2)
#define FRONT_COLOR  0xdadada //0xff0000
//#define MARGIN		5
typedef struct _RECTANGLE
{
	int Xmin;
	int Ymin;
	int Xmax;
	int Ymax;
}RECTANGLE;
struct Resource{
	unsigned char *up_buf;
	unsigned char *down_buf;
};
struct keyboard_config{
	int x;
	int y;
	int width;
	int height;
};
struct key_char{
	int  dx; //relative to key_decribe.dx
	int  dy; //relative to key_decribe.dy
	char value;
	int assic;
	//int front_size;
	//unsigned int front_color;
};

struct key_describe{
	int dx; //relative to keyboard_config.x
	int dy; //relative to keyboard_config.y
	int width;
	int height;
	unsigned int keyvalue;
	int	is_pressed;
	int char_num;	//°´¼üµÄ×Ö·ûÊý
	int front_size;	//×ÖºÅ
	unsigned int front_color;
	struct key_char ch[CHAR_NUM_MAX];
	struct Resource *res;
};
struct keyboard_layout{
	int num;
	struct key_describe key[SOFT_KEY_MAX];
};
struct frame_info{
	int	x; //keyboard x coordinate
	int y; //keyboard y corrdinate
	int stride; //framebuffer stride
	int width;  //real width of keyboard to display
	int height; //real height of keyboard to display
	enum KEYBOARD_FORMAT format;
	unsigned char alpha;
	unsigned char *buf1;
	unsigned char *buf2;
	unsigned char *buf;
	int osd; //1:use osd //0:don't usb osd
	int scale; //0~100, 50 means displaying 50%,100 means displaying 100% 
};
struct keyboard_context{

	struct keyboard_rect rect;
	struct keyboard_layout layout;
	FT_Library library; 
	FT_Face face;
	int front_size_used;
	struct frame_info frame;
	unsigned char *keyboard_buf;
	unsigned char *keyboard_front;
	unsigned char *keyboard_back;
	unsigned char *copy_buf;
	struct key_describe *key_valid;
	struct Resource res[RES_NUM];
	struct key_describe *k_shift;
	struct key_describe *k_caps;
	struct key_describe *k_lang;
	
	int disable_output;
	unsigned int front_color;
	unsigned int back_color;
	int chinese_enable;
	int init;
	RECTANGLE drag_rect;
};

#define ARGB_DEMUX(v,a,r,g,b)\
{\
	a = (v) >> 24;\
	r = (v) >> 16;\
	g = (v) >> 8;\
	b = (v);\
}

#define RGB_DEMUX(v,r,g,b)\
{\
	r = (v) >> 16;\
	g = (v) >> 8;\
	b = (v);\
}

#define RGB2YUV(y,u,v,r,g,b)\
{\
	y = (((66 * r + 129 * g + 25 * b) + 128) >> 8) + 16;\
	u = (((-38 * r - 74 * g + 112 * b) + 128) >> 8) + 128;\
	v = (((112 * r - 94 * g - 18 * b) + 128) >> 8) + 128;\
}
#define YUV2RGB(r, g, b, y, u, v)\
{\
	int c,d,e;\
	c = y - 16;\
	d = u - 128;\
	e = v - 128;\
	r = MAX(0,MIN(255,(298 * c + 409 * e + 128) >> 8));\
	g = MAX(0,MIN(255,(298 * c - 100 * d - 208 * e + 128) >> 8));\
	b = MAX(0,MIN(255,(298 * c + 516 * d + 128) >> 8));\
}
#define ARGB_MUX(r,g,b) ((r << 16) | (g << 8) | (b))
#define RGB565_DEMUX(v,r,g,b) {(r) = (v) >> 11 << 3; (g) = (v) >> 5 << 2; (b) = (v) << 3;}
#define RGB565_MUX(r,g,b) (((r) >> 3 << 11) | ((g) >> 2 << 5) | ((b) >> 3))
#define STATIC_BUF 0