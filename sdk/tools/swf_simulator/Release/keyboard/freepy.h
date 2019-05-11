#ifndef _FREEPY_H_
#define _FREEPY_H_
#include "freepy_func.h"

#define KEY_A_UP 0x41
#define KEY_Z_UP 0x5c

#define KEY_A_LOW 0x61
#define KEY_Z_LOW 0x7c

#define KEY_SPACEBAR 0x20
#define KEY_ENTER 0xA
#define KEY_BACKSPACE 0x8
#define KEY_TABLE 0x9
#define KEY_SINGLE_QUOTE 0x27

#define KEY_CLEAR			  	0xC

#define KEY_SHIFT					0x10
#define KEY_CTRL					0x11
#define KEY_MENU					0x12
#define KEY_PAUSE					0x13
#define KEY_CAPS_LOCK		 	0x14
#define KEY_ESC					 	0x1B


#define K_PRIVATE 0x100	

#define KEY_PAGE_UP				(K_PRIVATE|0x21)
#define KEY_PAGE_DOWN			(K_PRIVATE|0x22)
#define KEY_END						(K_PRIVATE|0x23)
#define KEY_HOME			  	(K_PRIVATE|0x24)
#define KEY_LEFT_ARROW	 	(K_PRIVATE|0x25)
#define KEY_UP_ARROW			(K_PRIVATE|0x26)
#define KEY_RIGHT_ARROW		(K_PRIVATE|0x27)
#define KEY_DOWN_ARROW		(K_PRIVATE|0x28)
#define KEY_SELECT				(K_PRIVATE|0x29)
#define KEY_PRINT_SCREEN 	(K_PRIVATE|0x2A)
#define KEY_EXECUTE			 	(K_PRIVATE|0x2B)
#define KEY_SNAPSHOT			(K_PRIVATE|0x2C)
#define KEY_INSERT				(K_PRIVATE|0x2D)
#define KEY_DELETE				(K_PRIVATE|0x2E)
#define KEY_HELP					(K_PRIVATE|0x2F)



//#define KEY_PGDN 0xe051
//#define KEY_PGUP 0xe049


#define KEY_NUMBER_0 0x30
#define KEY_NUMBER_1 0x31
#define KEY_NUMBER_2 0x32
#define KEY_NUMBER_3 0x33
#define KEY_NUMBER_4 0x34
#define KEY_NUMBER_5 0x35
#define KEY_NUMBER_6 0x36
#define KEY_NUMBER_7 0x37
#define KEY_NUMBER_8 0x38
#define KEY_NUMBER_9 0x39

#define MAX_LINE_LENGTH 81



#define global_font_size 20

#define MAX_HZ_SELECT_NUM 128

#define global_screen_w 800
#define global_screen_h 600

#define LINE_HEIGHT (global_font_size+4)
#define bitmap_buf_size ((LINE_HEIGHT+4)*global_screen_w)


typedef enum ARROW_STATE
{
	LEFT_VALID,
	LEFT_INVALID,
	RIGHT_VALID,
	RIGHT_INVALID
}ARROW_STATE;



typedef enum FREEPY_STATE
{
	WAITTING=0x0,
	NO_FIND,
	HZ_SELECT,
}FREEPY_STATE;

typedef struct FREEPY_RTN
{
	char output_str[MAX_INPUT_BUF_SIZE];
	int w;
	int h;
	unsigned short *buf;
	int stride;
	int update_flag;
	int word_off[16];
	int py_off[16];
	int arrow_off_x[8];
	int arrow_off_y[8];
}FREEPY_RTN;

typedef struct SEARCH_RTN
{
	int h;
	int w;
	void *addr;
	int word_off[16];
	int py_off[16];
	int arrow_off_x[8];
	int arrow_off_y[8];
}SEARCH_RTN;

int Freepy_Open(int font_size);
int Freepy_Close(void);

FREEPY_RTN freepy_process(unsigned int input, int eng);

extern FREEPY_RTN freepy_rtn;


#endif
