#include "swfext.h"
#include "stdio.h"
#include "windows.h"

typedef struct{
	INT32U	sd_in,cf_in,udisk_in;
	char	system_ver[64];
	INT32S	vol;
	INT32S	bl;
	INT32S	LanId;
}SWF_SYSTEM_CONF;

static SWF_SYSTEM_CONF sys_conf;

void update_media_status(int key)
{
	switch(key)
	{
		case '/':
			sys_conf.sd_in = !sys_conf.sd_in;
			printf("sd = %d\n", sys_conf.sd_in);
			break;
		case '*':
			sys_conf.cf_in = !sys_conf.cf_in;
			printf("cf = %d\n", sys_conf.cf_in);
			break;
		case '-':
			sys_conf.udisk_in = !sys_conf.udisk_in;
			printf("usb = %d\n", sys_conf.udisk_in);
			break;
		default:
			return;
	}
	SWF_Message(NULL, SWF_MSG_KEY|'C', NULL);
}

typedef enum {
	ROOT_NAND=0,
	ROOT_SD,
	ROOT_CF,	
	ROOT_UDISK
}source_type;

static INT32S media2Disk(void * handle)
{
	INT32S  media,disk_type;
	SWFEXT_FUNC_BEGIN(handle);
	media = Swfext_GetNumber();
	switch(media)
	{
	case ROOT_NAND:
		disk_type = 'C';
		break;
	case ROOT_SD:
		disk_type = 'D';
		break;
	case ROOT_CF:
		disk_type = 'E';
		break;
	case ROOT_UDISK:
		disk_type = 'F';
		break;
	default:
		disk_type = 'C';
		break;
	}		
	Swfext_PutNumber(disk_type);	
	SWFEXT_FUNC_END();
}

static INT32S setVolume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	sys_conf.vol = Swfext_GetNumber();
	printf("vol <- %d\n", sys_conf.vol);	
	SWFEXT_FUNC_END();
}

static INT32S getVolume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.vol);
	SWFEXT_FUNC_END();
}

static INT32S getFwVersion(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(sys_conf.system_ver);
	SWFEXT_FUNC_END();
}

static INT32S getDiskSpace(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(10000);
	SWFEXT_FUNC_END();
}

static INT32S getDiskSpaceLeft(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(5000);
	SWFEXT_FUNC_END();
}
static INT32S getCardSpace(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(10000);
	SWFEXT_FUNC_END();
}
static INT32S getCardSpaceLeft(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(5000);
	SWFEXT_FUNC_END();
}
static INT32S getCFSpace(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(10000);
	SWFEXT_FUNC_END();
}
static INT32S getCFSpaceLeft(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(5000);
	SWFEXT_FUNC_END();
}
static INT32S getUSBSpace(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(10000);
	SWFEXT_FUNC_END();
}

static INT32S getUSBSpaceLeft(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(10000);
	SWFEXT_FUNC_END();
}

static INT32S setBackLightStrength(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	sys_conf.bl = Swfext_GetNumber();
	printf("BackLight <- %d\n", sys_conf.bl);	
	SWFEXT_FUNC_END();
}

static INT32S getBackLightStrength(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.bl);
	SWFEXT_FUNC_END();
}

static INT32S getYear(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	Swfext_PutNumber(systime.wYear);
	SWFEXT_FUNC_END();
}

static INT32S setYear(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	systime.wYear = Swfext_GetNumber();
	SetLocalTime(&systime);
	SWFEXT_FUNC_END();
}

static INT32S getMonth(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	Swfext_PutNumber(systime.wMonth);
	SWFEXT_FUNC_END();
}

static INT32S setMonth(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	systime.wMonth = Swfext_GetNumber();
	SetLocalTime(&systime);
	SWFEXT_FUNC_END();
}

static INT32S getDay(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	Swfext_PutNumber(systime.wDay);
	SWFEXT_FUNC_END();
}

static INT32S setDay(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	systime.wDay = Swfext_GetNumber();
	SetLocalTime(&systime);
	SWFEXT_FUNC_END();
}

static INT32S getHour(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	Swfext_PutNumber(systime.wHour);
	SWFEXT_FUNC_END();
}

static INT32S setHour(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	systime.wHour = Swfext_GetNumber();
	SetLocalTime(&systime);
	SWFEXT_FUNC_END();
}

static INT32S getMin(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	Swfext_PutNumber(systime.wMinute);
	SWFEXT_FUNC_END();
}

static INT32S setMin(void * handle)
{
	SYSTEMTIME systime;
	SWFEXT_FUNC_BEGIN(handle);
	GetLocalTime(&systime);
	systime.wMinute = Swfext_GetNumber();
	SetLocalTime(&systime);
	SWFEXT_FUNC_END();
}

static INT32S getCurLanguage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.LanId);
	SWFEXT_FUNC_END();
}

static INT32S setCurLanguage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	sys_conf.LanId = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

static INT32S checkCardStatus(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.sd_in);	
	SWFEXT_FUNC_END();
}

static INT32S checkUsbStatus(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.udisk_in);	
	SWFEXT_FUNC_END();
}

static INT32S checkCFStatus(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sys_conf.cf_in);		
	SWFEXT_FUNC_END();
}

static INT32S restoreSysDefaultConfig(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("resotre to default\n");
	SWFEXT_FUNC_END();
}

INT32S swfext_system_info_register(void)
{
	sys_conf.vol = 5;
	sys_conf.cf_in = 0;
	sys_conf.sd_in = 0;
	sys_conf.udisk_in = 0;
	strcpy(sys_conf.system_ver, "0001");
	sys_conf.LanId = 0;
	sys_conf.bl = 2;

	SWFEXT_REGISTER("sys_setVolume", setVolume);
	SWFEXT_REGISTER("sys_getVolume", getVolume);
	SWFEXT_REGISTER("sys_getFwVersion", getFwVersion);
	SWFEXT_REGISTER("sys_getDiskSpace", getDiskSpace);
	SWFEXT_REGISTER("sys_getDiskSpaceLeft", getDiskSpaceLeft);
	SWFEXT_REGISTER("sys_getCardSpace", getCardSpace);
	SWFEXT_REGISTER("sys_getCardSpaceLeft", getCardSpaceLeft);
	SWFEXT_REGISTER("sys_getCFSpace", getCFSpace);
	SWFEXT_REGISTER("sys_getCFSpaceLeft", getCFSpaceLeft);
	SWFEXT_REGISTER("sys_getUSBSpace", getUSBSpace);
	SWFEXT_REGISTER("sys_getUSBSpaceLeft", getUSBSpaceLeft);	
	SWFEXT_REGISTER("sys_setBackLightStrength", setBackLightStrength);
	SWFEXT_REGISTER("sys_getBackLightStrength", getBackLightStrength);
	SWFEXT_REGISTER("sys_getYear", getYear);
	SWFEXT_REGISTER("sys_setYear", setYear);
	SWFEXT_REGISTER("sys_getMonth", getMonth);
	SWFEXT_REGISTER("sys_setMonth", setMonth);
	SWFEXT_REGISTER("sys_getDay", getDay);
	SWFEXT_REGISTER("sys_setDay", setDay);
	SWFEXT_REGISTER("sys_getHour", getHour);
	SWFEXT_REGISTER("sys_setHour", setHour);
	SWFEXT_REGISTER("sys_getMin", getMin);
	SWFEXT_REGISTER("sys_setMin", setMin);
	SWFEXT_REGISTER("sys_getCurLanguage", getCurLanguage);
	SWFEXT_REGISTER("sys_setCurLanguage", setCurLanguage);
	SWFEXT_REGISTER("sys_checkCardStatus", checkCardStatus);
	SWFEXT_REGISTER("sys_checkUsbStatus", checkUsbStatus);
	SWFEXT_REGISTER("sys_checkCFStatus", checkCFStatus);
	SWFEXT_REGISTER("sys_restoreSysDefaultConfig", restoreSysDefaultConfig);
	SWFEXT_REGISTER("sys_media2Disk", media2Disk);
	return 0;
}


