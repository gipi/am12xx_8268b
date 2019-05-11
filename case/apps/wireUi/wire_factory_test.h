#ifndef __WIRE_FACTORY_TEST
#define __WIRE_FACTORY_TEST


#define TEST_CONFIG_FILE	"miraline_test.conf"

typedef struct _frame_head_info{
	int w;
	int h;
	int len;
}FRAME_HEAD_INFO;

typedef int (*INITAUDIO)(int , int , int );
typedef int (*SENDAUDIOBUF)(const void*, unsigned int);
typedef void (*UNUINTAUDIO)();
typedef void (*SETAUDIOCALLBACK)(int (*)(int, void *));

int do_factory_test();


#endif
