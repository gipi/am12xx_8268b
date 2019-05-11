#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#define    COLOR_HUE           1 
#define    COLOR_SATURATION    2 
#define    COLOR_BRIGHTNESS    3 
#define    COLOR_CONTRAST      4 
#define    COLOR_SHARPNESS     5

#define    MODE_READ         0
#define    MODE_WRITE        1


#define		LCD_ENABLE		0
#define		HDMI_ENABLE		1
#define		CVBS_ENABLE		2
#define		YPBPR_ENABLE	4
#define		VGA_ENABLE		8
#define    DEFAULT_OUTPUT_RESOLUTION  64
#define 	EDID_CONFIG_PATH_IN_CASE  "/mnt/vram/edid.bin"

#define		YUV_BLACK		0x108080
#define		RGB_BLACK		0x0
enum{
	GET_SPACE_TOTAL=0,
	GET_SPACE_FREE=1
};

enum cmd_set_env_para_e{
	CMD_SET_BACKLIGHT_LENGTH,
	CMD_SET_HUE,
	CMD_SET_SATURATION,
	CMD_SET_BRIGHTNESS,
	CMD_SET_CONTRAST,
	CMD_SET_VIDEO_PALY_MODE,
	CMD_SET_LOCAL_LANGUAGE,
	CMD_SET_DEFAULT_ENV,  
	CMD_SET_HDMI_MODE,
	CMD_SET_CVBS_MODE,
	CMD_SET_YPBPR_MODE,
	CMD_SET_VGA_MODE,
	CMD_SET_SHARPNESS,
};

enum media_repeat_mode_e{
	MODE_REPEAT_SEQUENCE,	
	MODE_REPEAT_ONE,
	MODE_REPEAT_RANDOM,	
	MODE_ONCE,
	MODE_INVAID,
};

enum video_dsp_ratio_e{
	DSP_LETTER_BOX,	
	DSP_CUT_TO_SCREEN,
	DSP_FULL_SCREEN,
	DSP_ORIGINAL,
	
}
;
enum photo_dsp_ratio_e{
	DSP_FIT_TO_SCREEN,
	DSP_FULLSCREEN,
	DSP_CROP_TO_FIT,
};

typedef enum{
	CMD_GET_SCREEN_WIDTH = 1,
	CMD_GET_SCREEN_HEIGHT	= 2,
}system_screen_para_e;

typedef enum{
	CMD_GET_GLONUM,
	CMD_GET_GLOSTR,
}system_globalvalue_cmd_e;

typedef enum{
	CARD_TYPE_SD =1,			///< sd card
	CARD_TYPE_MMC =2,			///< mmc card
	CARD_TYPE_MS =3,			///< ms card
	CARD_TYPE_MS_PRO =4,		///< ms pro card
	CARD_TYPE_XD =5,			///< xd card
	CARD_TYPE_CF=6				 ///< cf card
}system_card_type_e;
typedef enum{
	CONNECT_INIT =-1,			///< sd card
	CONNECT_GFILESTORAGE =0,			///< mmc card
	CONNECT_EZUSB =1,			///< ms card
	
}connectpc_flag;


typedef struct edid_config{
	unsigned int vga_valid;
	unsigned int hdmi_valid;
	int vga_format;
	int hdmi_format;
	
}edid_config_info;


typedef enum{
	GET_FILEINFO_OK ,
	CANNOT_FIND_INFO_FILE ,
	CANNOT_FIND_BEGIN_KEYWORD ,
	CANNOT_FIND_END_KEYWORD ,
}get_info_err;

typedef enum{
	MAINMENU_SWF_ACTIVE = 1,
	CONNECTPC_SWF_ACTIVE = 2,

	PHOTO_SWF_ACTIVE = 3,
	VIDEO_SWF_ACTIVE = 4,
	AUDIO_SWF_ACTIVE = 5,
	OFFICE_SWF_ACTIVE = 6,

	USBDISPLAY_SWF_ACTIVE = 7,
	WIFIDISPLAY_SWF_ACTIVE = 8,
	LANDISPLAY_SWF_ACTIVE = 9,
	DLNA_SWF_ACTIVE = 10,
	MIRACAST_SWF_ACTIVE = 11,

	SET_SYS_SWF_ACTIVE = 12,
	SET_VIDEO_SWF_ACTIVE = 13,
	SET_PHOTO_SWF_ACTIVE = 14,
	SET_AUDIO_SWF_ACTIVE = 15,
	SET_WIFI_SWF_ACTIVE = 16,
	SET_LAN_SWF_ACTIVE = 17

}active_swf_flag_s;
#endif




