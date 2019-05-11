#include "swfext.h"

void * __as_handler = NULL;
INT32S __as_param_num   = 0;
INT32S __as_return_type = 0;

#define PREPARE_OUTPUT()\
{\
	if(__as_return_type) \
	{\
		return;\
	}\
	else\
	{\
		__as_return_type = 1;\
		for(;__as_param_num > 0;__as_param_num--)\
		{\
			SWF_GetNumber(__as_handler);\
		}\
	}\
}

#define PREPARE_INPUT()\
{\
	if(__as_param_num == 0)\
		return 0;\
	else\
		__as_param_num--;\
}

INT32S Swfext_GetParamNum()
{
	return __as_param_num;
}

INT32S Swfext_GetParamType()
{
	return SWF_GetParamType(__as_handler);
}

INT32S Swfext_GetNumber()
{
	PREPARE_INPUT();
	return FTOI(SWF_GetNumber(__as_handler));
}

FLOAT32 Swfext_GetFloat()
{
	PREPARE_INPUT();
	return SWF_GetNumber(__as_handler);
}

INT8S * Swfext_GetString()
{
	PREPARE_INPUT();
	return SWF_GetString(__as_handler);
}

void * Swfext_GetObject()
{
	PREPARE_INPUT();
	return SWF_GetObject(__as_handler);
}

void Swfext_PutNumber(AS_NUMBER n)
{
	PREPARE_OUTPUT();
	SWF_PutNumber(__as_handler, ITOF(n));
}

void Swfext_PutFloat(FLOAT32 n)
{
	PREPARE_OUTPUT();
	SWF_PutNumber(__as_handler, ITOF(n));
}

void Swfext_PutString(INT8S * str)
{
	PREPARE_OUTPUT();
	SWF_PutString(__as_handler, str);
}

void Swfext_PutObject(void * obj)
{
	PREPARE_OUTPUT();
	SWF_PutObject(__as_handler, obj);
}

void Swfext_PutNull()
{
	PREPARE_OUTPUT();
	SWF_PutNull(__as_handler);
}

INT32S Swfext_Register(void)
{
	swfext_filelist_register();
	swfext_pe_register();
	swfext_music_register();
	swfext_video_register();
	swfext_system_info_register();
	swfext_vvd_register();
	swfext_ebook_register();
	swfext_locale_register();
	swfext_cmmb_register();
	swfext_flashengine_register();
	swfext_usbui_register();
	return 1;
}

#ifdef MIPS_VERSION

#include "includes.h"
#include "elf.h"

int FUI_Start(char* swf_filaname);

static INT32S SWF_Ext_Init(void * init)
{
	return 1;
}

static INT32S SWF_Ext_Release(void)
{
	return 1;
}

static INT32U swf_start(char * swf_filename)
{
	FUI_Start(swf_filename);
}

static unsigned int swf_api[]=
{
	(unsigned int)swf_start,
};
/**
 *@brief 驱动 地址结构体
*/
extern unsigned int _dstart, _dend;
addrInfo addr_info __addrdata=	
{
	startAddr:(INT32U)&_dstart,
	endAddr:(INT32U)&_dend
};

static drv_Info drvinfo = {
	"drv",
	SWF_Ext_Init,
	SWF_Ext_Release,
	(INT32U)swf_api
};

drv_Info *GetDrvInfo(void)
{
	return &drvinfo;
}

#endif
