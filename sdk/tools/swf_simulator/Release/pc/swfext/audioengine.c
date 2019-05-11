#include "swfext.h"
#include "stdio.h"
#include "audioengine.h"

#define  AUDIO_STEP		5000

static int counter = 0;
static int state = 0;
static INT32U * decode_buffer = NULL;

char filename[256];
SWF_MCI		swfMci;


static INT32S getinfo(MCIDEVICEID mciid, DWORD item)
{
	if(mciid != 0)
	{
		MCI_STATUS_PARMS mcistatusparms;
		mcistatusparms.dwCallback=0;
		mcistatusparms.dwItem=item;
		mcistatusparms.dwReturn=0;
		mciSendCommand(mciid, MCI_STATUS,MCI_STATUS_ITEM,(DWORD)&mcistatusparms);
		return mcistatusparms.dwReturn;
	}

	return 0;
}

static INT32S getstate()
{
	return getinfo(swfMci.id, MCI_STATUS_MODE);
}

static INT32S MCISeekTo(INT32S dwTo)
{
	DWORD dwReturn = 0;
	
	if(swfMci.id !=0)
	{	
		if (dwTo>0 && dwTo< swfMci.totalTime)
		{   
			MCI_SEEK_PARMS mciSP;
			mciSP.dwTo=dwTo;
			dwReturn = mciSendCommand(swfMci.id, MCI_SEEK, MCI_WAIT|MCI_NOTIFY|MCI_TO, (DWORD)(LPVOID)&mciSP);
		}else if (dwTo>=swfMci.totalTime)
			dwReturn = mciSendCommand(swfMci.id, MCI_SEEK, MCI_SEEK_TO_END, 0);			
		else
			dwReturn = mciSendCommand(swfMci.id, MCI_SEEK, MCI_SEEK_TO_START, 0);			
	}
	return dwReturn;
}

static INT32S Open(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio open\n");

	memset(&swfMci, 0, sizeof(swfMci));

	SWFEXT_FUNC_END();	
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio close\n");
	if(swfMci.id != 0)
	{
		mciSendCommand(swfMci.id,MCI_CLOSE,0,0);
		swfMci.id = 0;
		swfMci.state = AE_IDLE;
	}
	SWFEXT_FUNC_END();	
}

static INT32S SetFile(void * handle)
{
	int type;
	char * file;
	SWFEXT_FUNC_BEGIN(handle);
	type = Swfext_GetParamType();
	if(type == SWFDEC_AS_TYPE_STRING)
	{
		file = Swfext_GetString();
	}
	else
	{
		file = "c:\\*";
	}
	printf("music open file %s\n", file);
	
	strcpy(filename, file);
	swfMci.file = filename;
	swfMci.totalTime = 0;	
	swfMci.state = AE_IDLE;

	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();	
}

static char * file_map(char * filename)
{
	static char buf[256];
	if(filename[1] == ':')
	{
		switch(filename[0])
		{
		case 'd':
		case 'D':
			strcpy(buf, "SD");
			break;
		case 'e':
		case 'E':
			strcpy(buf, "CF");
			break;
		case 'f':
		case 'F':
			strcpy(buf, "UDISK");
			break;
		default:
			strcpy(buf, "LOCAL");
			break;
		}
		strcat(buf, filename + 2);
		return buf;
	}
	else
	{
		return filename;
	}
}

static INT32S Play(void * handle)
{
	MCI_OPEN_PARMS mciopenparms;
	MCI_PLAY_PARMS mciplayparms;
	short int m_count = 0;

	SWFEXT_FUNC_BEGIN(handle);
	if(swfMci.state != AE_PAUSE)
		Stop(handle);

	printf("audio play\n");	
	
	mciopenparms.lpstrElementName=file_map(swfMci.file);
	printf("lpstrElementName=%s\n", mciopenparms.lpstrElementName);
	mciopenparms.lpstrDeviceType=NULL;
	if(!mciSendCommand(0,MCI_OPEN,MCI_DEVTYPE_WAVEFORM_AUDIO,(DWORD)(LPVOID)&mciopenparms))
	{
		swfMci.id=mciopenparms.wDeviceID;
		mciplayparms.dwCallback=0;	
		swfMci.totalTime=getinfo(swfMci.id, MCI_STATUS_LENGTH);    	
		mciplayparms.dwFrom=MCI_MAKE_HMS(0,0,0);
		mciplayparms.dwTo=MCI_MAKE_HMS(MCI_HMS_HOUR(swfMci.totalTime),MCI_HMS_MINUTE(swfMci.totalTime),MCI_HMS_SECOND(swfMci.totalTime));  
		if(mciSendCommand(swfMci.id,MCI_PLAY,MCI_TO|MCI_FROM,(DWORD)(LPVOID)& mciplayparms))
			printf("play fail!\n");
		
		swfMci.state = AE_PLAY;
	}
	else
		printf("open fail!\n");
	
	
	SWFEXT_FUNC_END();	
}

static INT32S Stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio stop\n");
	
	if(MCI_MODE_PLAY == getinfo(swfMci.id, MCI_STATUS_MODE))
	{
		mciSendCommand(swfMci.id,MCI_CLOSE, 0, 0);		
		swfMci.id = 0;
		swfMci.state = AE_STOP;
	}	
	SWFEXT_FUNC_END();	
}

static INT32S Pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio pause\n");			
	if(MCI_MODE_PLAY == getstate())
	{
		mciSendCommand(swfMci.id, MCI_PAUSE, 0, 0);	
		swfMci.state = AE_PAUSE;
	}	
	SWFEXT_FUNC_END();	
}


static INT32S Resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio resume\n");
	if(MCI_MODE_PAUSE == getstate())
	{		
		mciSendCommand(swfMci.id, MCI_RESUME, 0, 0);
		swfMci.state = AE_PLAY;
	}		
	SWFEXT_FUNC_END();	
}

static INT32S FF(void * handle)
{	
	MCI_PLAY_PARMS mciplayparms;
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio ff\n");
	swfMci.state = AE_FF;

	if (MCI_MODE_PAUSE != getstate()) 
		Pause(handle); 

	swfMci.curTime = getinfo(swfMci.id, MCI_STATUS_POSITION);
	swfMci.curTime += AUDIO_STEP;
	MCISeekTo(swfMci.curTime);
	mciSendCommand(swfMci.id, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID) &mciplayparms); // 第四个参数不能为空，不然会没有声音。
	swfMci.state = AE_PLAY;

	SWFEXT_FUNC_END();	
}

static INT32S FB(void * handle)
{
	MCI_PLAY_PARMS mciplayparms;
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio fb\n");	
	swfMci.state = AE_FB;

	if (MCI_MODE_PAUSE != getstate()) 
		Pause(handle); 
	
	swfMci.curTime = getinfo(swfMci.id, MCI_STATUS_POSITION);
	swfMci.curTime -= AUDIO_STEP;
	MCISeekTo(swfMci.curTime);
	mciSendCommand(swfMci.id, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID) &mciplayparms);
	swfMci.state = AE_PLAY;

	SWFEXT_FUNC_END();	
}


static INT32S State(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);	
	Swfext_PutNumber(swfMci.state);
	SWFEXT_FUNC_END();	
}

static INT32S TotalTime(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("audio totaltime\n");
	Swfext_PutNumber(swfMci.totalTime);
	SWFEXT_FUNC_END();	
}


static INT32S CurTime(void * handle)
{	
	INT32S dwPos = 0;
	SWFEXT_FUNC_BEGIN(handle);

	dwPos = getinfo(swfMci.id, MCI_STATUS_POSITION);
	if(dwPos >= swfMci.totalTime)
	{
		mciSendCommand(swfMci.id, MCI_CLOSE, 0, 0);	
		swfMci.id = 0;
		dwPos = 0;
	}
	Swfext_PutNumber(dwPos);

	SWFEXT_FUNC_END();	
}

static INT32S Singer(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString("Michal Jackson");
	SWFEXT_FUNC_END();	
}

static INT32S Picture(void * handle)
{
	void * target;
	INT32S decode_w, decode_h;
	SWFEXT_FUNC_BEGIN(handle);
	target   = Swfext_GetObject();
	decode_w = Swfext_GetNumber();
	decode_h = Swfext_GetNumber();
	if(decode_buffer != NULL) 
		swf_free(decode_buffer);
	decode_buffer = (INT32U*)swf_malloc_share(decode_w * decode_h * 4);
	memset(decode_buffer, 0x80, decode_w * decode_h * 4);
	SWF_AttachBitmap(target,decode_buffer,decode_w,decode_h,decode_w,decode_h,decode_w,SWF_BMP_FMT_ARGB,NULL);
	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();	
}

static INT32S Album(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString("World Crying");
	SWFEXT_FUNC_END();	
}

static INT32S Extention(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString("mp3");
	SWFEXT_FUNC_END();	
}

static INT32S SampleRate(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(44100);
	SWFEXT_FUNC_END();	
}

static INT32S Lyrics(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString("You and me");
	SWFEXT_FUNC_END();	
}

static INT32S GetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

static INT32S GetEffect(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

INT32S swfext_music_register(void)
{
	SWFEXT_REGISTER("ae_Open", Open);
	SWFEXT_REGISTER("ae_Close", Close);
	SWFEXT_REGISTER("ae_SetFile", SetFile);
	SWFEXT_REGISTER("ae_Play", Play);
	SWFEXT_REGISTER("ae_Stop", Stop);
	SWFEXT_REGISTER("ae_Pause", Pause);
	SWFEXT_REGISTER("ae_Resume", Resume);
	SWFEXT_REGISTER("ae_FF", FF);
	SWFEXT_REGISTER("ae_FB", FB);
	SWFEXT_REGISTER("ae_State", State);
	SWFEXT_REGISTER("ae_TotalTime", TotalTime);
	SWFEXT_REGISTER("ae_CurTime", CurTime);
	SWFEXT_REGISTER("ae_Singer", Singer);
	SWFEXT_REGISTER("ae_Album", Album);
	SWFEXT_REGISTER("ae_Picture", Picture);
	SWFEXT_REGISTER("ae_Extention", Extention);
	SWFEXT_REGISTER("ae_SampleRate", SampleRate);
	SWFEXT_REGISTER("ae_Lyrics", Lyrics);
	SWFEXT_REGISTER("ae_GetPlayMode", GetPlayMode);
	SWFEXT_REGISTER("ae_GetEffect", GetEffect);
	return 0;
}
