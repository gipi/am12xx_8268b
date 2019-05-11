#include "swf_ext.h"
#include "sys_cfg.h"
#include "fui_common.h"

void * __as_handler = NULL;
int __as_param_num   = 0;
int __as_return_type = 0;

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

int Swfext_GetParamNum()
{
	return __as_param_num;
}

int Swfext_GetParamType()
{
	return SWF_GetParamType(__as_handler);
}

int Swfext_GetNumber()
{
	PREPARE_INPUT();
	return FTOI(SWF_GetNumber(__as_handler));
}

float Swfext_GetFloat()
{
	PREPARE_INPUT();
	return SWF_GetNumber(__as_handler);
}

char * Swfext_GetString()
{
	PREPARE_INPUT();
	return SWF_GetString(__as_handler);
}

void * Swfext_GetObject()
{
	PREPARE_INPUT();
	return SWF_GetObject(__as_handler);
}

void Swfext_PutNumber(int n)
{
	PREPARE_OUTPUT();
	SWF_PutNumber(__as_handler, ITOF(n));
}

void Swfext_PutFloat(float n)
{
	PREPARE_OUTPUT();
	SWF_PutNumber(__as_handler, ITOF(n));
}

void Swfext_PutString(char * str)
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


static int SWF_Ext_Init(void * init)
{
	return 1;
}

static int SWF_Ext_Release(void)
{
	return 0;
}

int Swfext_Register(void)
{
	swfext_locale_register();
	swfext_flashengine_register();
	swfext_alarm_register();
	swfext_music_register();
	swfext_pe_register();
	swfext_video_register();
	swfext_system_info_register();
	swfext_filelist_register();
	swfext_ebook_register();
	swfext_album_register();
	swfext_vvd_register();
	swfext_fs_register();
	swfext_subtitle_parser_register();
	swfext_ftpclient_register();
	swfext_calibrate_register();
	swfext_audio_record_register();
	swfext_sound_register();
	swfext_wifi_register();
	swfext_webalbum_register();
	swfext_lan_register();
	swfext_widi_register();
	swfext_hid_register();//shane
	swfext_customization_register(); //chavi 141028
	
#ifdef MODULE_CONFIG_VIDEO_OSD_NEW
	/// for new video osd engine
	swfext_osdengine_register();
#endif


#ifdef MODULE_CONFIG_WEBMAIL
	swfext_webmail_register();
#endif

#ifdef MODULE_CONFIG_DLNA
	swfext_dlna_engine_register();
	swfext_dlna_dmr_register();
#endif

	/// register the bluetooth interface.
	swfext_bt_register();

#ifdef MODULE_CONFIG_QRCODE_GENERATOR
	swfext_qrcode_register();
#endif

#ifdef MODULE_CONFIG_SQLITE
	swfext_audio_db_register();
	swfext_video_db_register();
	swfext_photo_db_register();
	swfext_office_db_register();

#endif

	swfext_miracast_engine_register();

	return 0;
}


