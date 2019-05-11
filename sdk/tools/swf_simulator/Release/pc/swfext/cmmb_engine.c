#include "swfext.h"
#include "stdio.h"

typedef enum{
	/* idle: when no video played or closed */
	CMMB_IDLE=0,
	/* play: video played */
	CMMB_PLAY=1,
	/* stop: video stopped */
	CMMB_STOP=2,
} CMMB_STATE;

static int    channel = 0;
static int    channel_count = 6;
static int    esg_index = -1;
static int	  esg_count = 5;
static int    state = CMMB_IDLE;

static char * channel_name[] = {
	"BBC",
	"VOA",
	"CCTV",
	"TEST1",
	"TEST2",
	"TEST3",
};

static char * esg_name[] = {
	"news1",
	"news2",
	"news3",
	"news4",
	"news5",
};

static char * esg_time[] = {
	"10:00",
	"11:00",
	"12:00",
	"13:00",
	"14:00",
};

static INT32S Open(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	printf("cmmb open\n");
	SWFEXT_FUNC_END();	
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("cmmd close\n");
	SWFEXT_FUNC_END();	
}


static INT32S Scan_Service(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("scan service\n");
	SWFEXT_FUNC_END();
}

static INT32S Get_Scan_Status(void * handle)
{
	static int progress = 0;
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(progress);
	if(progress < 100)
	{
		progress += 10;
	}
	SWFEXT_FUNC_END();	
}

static INT32S Get_Ch_Count(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(channel_count);
	SWFEXT_FUNC_END();	
}

static INT32S Set_Channel(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	channel = Swfext_GetNumber();
	esg_index = 0;
	SWFEXT_FUNC_END();	
}

static INT32S Get_Channel_Name(void * handle)
{
	int n;
	SWFEXT_FUNC_BEGIN(handle);
	n = Swfext_GetNumber();
	Swfext_PutString(channel_name[n]);
	SWFEXT_FUNC_END();	
}

static INT32S Get_Channel_Type(void * handle)
{
	int n;
	SWFEXT_FUNC_BEGIN(handle);
	n = Swfext_GetNumber();
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static INT32S Get_Next_esg(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(esg_index < esg_count - 1)
	{
		esg_index++;
		Swfext_PutNumber(1);
	}
	else
	{
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();	
}

static INT32S Get_Esg_Program(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(esg_name[esg_index]);
	SWFEXT_FUNC_END();	
}

static INT32S Get_Esg_Time(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(esg_time[esg_index]);
	SWFEXT_FUNC_END();	
}

static INT32S Play(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	state = CMMB_PLAY;
	printf("cmmb play\n");
	SWFEXT_FUNC_END();	
}

static INT32S Stop(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	state = CMMB_STOP;
	printf("cmmb stop\n");
	SWFEXT_FUNC_END();	
}

static INT32S GetState(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(state);
	SWFEXT_FUNC_END();	
}

INT32S swfext_cmmb_register(void)
{
	SWFEXT_REGISTER("cmmb_Open", Open);
	SWFEXT_REGISTER("cmmb_Close", Close);
	SWFEXT_REGISTER("cmmb_Play", Play);
	SWFEXT_REGISTER("cmmb_Stop", Stop);
	
	SWFEXT_REGISTER("cmmb_Scan_Service", Scan_Service);
	SWFEXT_REGISTER("cmmb_Get_Scan_Status", Get_Scan_Status);
	SWFEXT_REGISTER("cmmb_Get_Ch_Count", Get_Ch_Count);
	SWFEXT_REGISTER("cmmb_Set_Channel", Set_Channel);
	SWFEXT_REGISTER("cmmb_Get_Channel_Name", Get_Channel_Name);
	SWFEXT_REGISTER("cmmb_Get_Channel_Type", Get_Channel_Type);
	SWFEXT_REGISTER("cmmb_Get_Next_esg", Get_Next_esg);
	SWFEXT_REGISTER("cmmb_Get_Esg_Program", Get_Esg_Program);
	SWFEXT_REGISTER("cmmb_Get_Esg_Time", Get_Esg_Time);
	SWFEXT_REGISTER("cmmb_State", GetState);

	return 0;
}
