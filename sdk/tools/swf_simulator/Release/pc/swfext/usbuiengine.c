#include "swfext.h"
#include "stdio.h"
#include "fontdrv.h"

static INT32S KeyBoard_Show(void * handle)
{
	int value;
	SWFEXT_FUNC_BEGIN(handle);
	value = Swfext_GetNumber();
	keyboard_show(value);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
static INT32S EnableChinese(void * handle)
{
	int value;
	SWFEXT_FUNC_BEGIN(handle);
	value = Swfext_GetNumber();
	Enable_Chinese(value);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
INT32S sq357_getMemoryCurrent( void * handle )
{
    INT32S mc = 0;
    SWFEXT_FUNC_BEGIN(handle);
    mc = SWF_MemCurrent();
    Swfext_PutNumber( mc );
    SWFEXT_FUNC_END();
}

INT32S sq357_getMemoryTotal( void * handle )
{
    INT32S mc = 0;
    SWFEXT_FUNC_BEGIN(handle);
    mc = SWF_MemTotal();
    Swfext_PutNumber( mc );
    SWFEXT_FUNC_END();
}
INT32S sq357_getMemoryBasicCurrent( void * handle )
{
    INT32S mc = 0;
    SWFEXT_FUNC_BEGIN(handle);
    mc = SWF_MemBasicCurrent();
    Swfext_PutNumber( mc );
    SWFEXT_FUNC_END();
}
INT32S sq357_getMemoryBasicTotal( void * handle )
{
    INT32S mc = 0;
    SWFEXT_FUNC_BEGIN(handle);
    mc = SWF_MemBasicTotal();
    Swfext_PutNumber( mc );
    SWFEXT_FUNC_END();
}
static INT32S TTFont_load_new(void * handle)
{
	char * name, filename[128];
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	name = Swfext_GetString();
	Utf8ToGb2312(filename, name, strlen(name));
	rtn = ttfont_load_new(filename);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static INT32S TTFont_restore(void * handle)
{
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = ttfont_restore();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


INT32S swfext_usbui_register(void)
{
	SWFEXT_REGISTER("usb_KeyBoard_Show", KeyBoard_Show);
	SWFEXT_REGISTER("key_EnableChinese", EnableChinese);
	SWFEXT_REGISTER("sq357_getMemoryCurrent", sq357_getMemoryCurrent);
	SWFEXT_REGISTER("sq357_getMemoryTotal", sq357_getMemoryTotal);
	SWFEXT_REGISTER("sq357_getMemoryBasicCurrent", sq357_getMemoryBasicCurrent);
	SWFEXT_REGISTER("sq357_getMemoryBasicTotal", sq357_getMemoryBasicTotal);
	SWFEXT_REGISTER("TTFont_load_new", TTFont_load_new);
	SWFEXT_REGISTER("TTFont_restore", TTFont_restore);
}

