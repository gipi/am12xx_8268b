#ifndef SWF_TIME_H
#define SWF_TIME_H

typedef enum {
	CLOCK_TOTAL=0,
	CLOCK_SHAPE_DEFINE,
	CLOCK_BUILD_DISPLAYLIST,
	CLOCK_SCANLINE,
	CLOCK_BACKGROUND_FILL,
	CLOCK_RENDER,
	CLOCK_2D,
	CLOCK_IDLE,
	CLOCK_ZBUF,
	CLOCK_MERGE,
	CLOCK_TEST1,
	CLOCK_TEST2,
	CLOCK_TYPE_NUM
}CLOCKTYPE;

typedef struct _swf_clock
{
	unsigned int time_start;
	unsigned int time_span;
	unsigned int time_total;
}SWFCLOCK;

void clear_clock(void);
void start_clock(int i);
unsigned int stop_clock(int i);
unsigned int total_clock(int i);

extern char * clock_type[CLOCK_TYPE_NUM];

#endif

