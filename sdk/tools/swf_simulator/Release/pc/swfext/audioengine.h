#ifndef SWF_AUDIO_ENGINE
#define	SWF_AUDIO_ENGINE

#ifndef MIPS_VERSION
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#endif

enum 
{
	AE_IDLE=0,
	AE_PLAY=1,
	AE_PAUSE=2,
	AE_FF=3,
	AE_FB=4,
	AE_STOP,
};


typedef struct 
{
	INT8U  *file;
	MCIDEVICEID id;
	INT32S totalTime;   // ms
	INT32S curTime;
	INT32S state;
	INT32S loop;

	/*INT32S frame_count;
	INT8U* audio_data;
	INT32S audio_size;
	INT32S loop;
	INT32S flag;*/
}SWF_MCI;


#endif //SWF_AUDIO_ENGINE