#ifndef _APPS_VRAM_H
#define _APPS_VRAM_H

#include "sys_vram.h"
#include "apps_vram_default.h"
#include "sys_rtc.h"
#include "multialarm.h"


/**
* @note this file gives the definitions for application
*     vram.Please see the sys_vram.h for reference.
*/


/**
* @brief definition for system parameters.
* "SYS_PARAM_ID": id for system parameters.
* 
*/
#define    SYS_PARAM_SUBID    1  
#define    SYS_PARAM_NAME     1
#define PHOTOALBUM_MAX   16
#define PHOTOALBUMNAME_MAX   16

#define	GLOVAL_VALUE_NUMS_NUM 9	///< the length of the number array
#define 	GLOVAL_VALUE_STR_LEN 	32	///< the length of the str
#define  	GLOVAL_VALUE_STRS_NUM 1	///< the length of the str array

typedef int globalnum_type;
typedef struct global_value_s
{
	globalnum_type nums[GLOVAL_VALUE_NUMS_NUM];
	char strs[GLOVAL_VALUE_STRS_NUM][GLOVAL_VALUE_STR_LEN];
}global_value_t;

/**
* @brief save the softap ssid and password when OTA upgraded.
*/
typedef struct softap_info_s
{
	char softap_psk_setted_flag;
	char softap_ssid[32];
	char softap_psk[16];
}softap_info_t;

/**
* @brief structures for system config.
*/
struct sysconf_param
{
	/**
	* system related params
	*/
	char local_language_id;
	char active_media_type;  
	char sys_volume;
	 char audio_autoplay;
	/**
	* lcd related
	*/
	int backlight_strength;
	int brightness;
	int constrast;
	int saturation;
	int hue;
	int sharpness;

	/**
	* photo related
	*/
	char  photo_display_ratio;           // 0 - fit to screen; 1 - full screen; 2 - Crop to screen
	char photo_background_effect;  //see photo_background_effect_e
	char  photo_background_music_en;     // 0-disable; 1-enalbe
	int   photo_slideshow_interval;      // photo slideshow interval 3s/5s/...../12hour....
	char  photo_slideshow_mode;          // photo slideshow repeat mode : Once/repeat
	char  photo_slideshow_effect;        // photo slideshow effect : random...
	char  photo_clockshow_en;            // 0-do not show clock; 1-show clock
	char photo_autorotation_exif_en:1;		///< photo rotate automaticlly, 0 -disable ,1 -enable, used by exif information
	char photo_autorotation_adhere_en:1; ///<photo rotate automaticlly, 0 -disable ,1 -enable, used by adhere information

    /**
		Flash UI photo album name
	*/
	int is_udisk_dirty;///< 1:the udisk is dirty, the db should be created again, 0: nothing changed in udisk 
	int  albumSum;
	unsigned char albumStatus[PHOTOALBUM_MAX];
	char albumName[PHOTOALBUM_MAX][PHOTOALBUMNAME_MAX];
	/**
	* video related
	*/
	char video_playback_mode;            // video playback mode : Once/repeat/repeat one.
	char video_display_ratio;            // video display ration : original size / full screen.
	char video_autoplay;			//when enter in the video app, 1:auto play 0:you should choose a video file to be played
	/**
	* music related
	*/
	char music_effect;                   // music playback effect :Pop/Rock/Soft/DBB/Jazz/Normal/Classic
	char music_playback_mode;   // music playback mode : Once/repeat/repeat one.

	
	/**
	* calendar related
	*/
	char  calendar_clock_mode;           // clock mode: 0-24 hour mode; 1-12 hour mode

	/**
	*auto power on
	*/
	char auto_power_on_enable; //0 disalbe 1 enable
	rtc_time_t power_on_time;

	/**
	*auto power off
	*/
	char auto_power_off_enable; //0 disable  1 enable
	rtc_time_t power_off_time;

	char auto_power_freq;//01111111 form right to left, each bit stands for one day(monday to sunday),if the bit is 1, the day is enable 

	/**
	*printer
	**/
	char printer_enable; //0:disable priter 1:enable

	/**
	*HDMI related
	**/
	int hdmi_mode;		//Lcm_op.h DS_VIDEO_TIMING_FMT
	int hdmi_output_true;
	char output_enable;

	int cvbs_mode;		//Lcm_op.h CVBS_FORMAT
	int ypbpr_mode;		//Lcm_op.h YPbPr_FORMAT

	/**
	* check sum
	*/
	int check_sum;

	/**
	*alarms
	*/
	alarm_info_head_t alarm_info_head;

	/**
	*EBook related
	**/
	int ebook_text_color;
	int ebook_back_color;
	int ebook_text_size;

	/**
	*screen auto off or not
	**/
	char auto_screen_off_enable; //0 disable  1 enable
	int screen_off_time;		//mins

	/**
	* language auto set by APP
	**/
	char lang_auto_disable;	// 1: disable auto set    0: enable auto set

	/**global value**/
	char is_gloval_valid;	///< whether the global value is valid, 1: vaild 0: invalid
	global_value_t	 glo_val;	///< global value
	char realtek_wifi_concurrent_softap_ssid_init_flag;	///<the flag of realtek dongle wifi concurrent softap default ssid has setting
	
	char realtek_softap_ssid_init_flag;	///<the flag of realtek dongle wifi  softap default ssid has setting
	
	char ralink_softap_ssid_init_flag;	///<the flag of ralink dongle wifi  softap default ssid has setting 
	char dmr_title[256];
	char dmr_uuid[36+1];

	softap_info_t softap_info;			// save the softap ssid and password when OTA upgraded
	char psk_changed;					// 0: password have not changed by user        1: password have changed by user

	/**
	*wifi channel region
	*/
	char wifiChannelRegion[16];

	/**
	* Which UI stayed before last power down, only used in AM8252 now;
	**/
	char last_ui;	//0: init value, load help UI; 1: DLNA/EZAir UI; 2: EZMirra UI;

	//char air_view;  //0: off; 1: off;
	char play_way;	//0: internet ; 1: local  just for ezmusic ;
};

/**
* @brief initialize the application level vram data. 
* @return 0-success and others-failed. 
*/
extern int apps_vram_init();

/**
* @brief release the application level vram manager. 
*    this function will be used only in the case that
*    the application system being shut down.
* @return 0-success and others-failed. 
*/
extern int apps_vram_release();


/**
* @brief store the vram content from sdram to disk that 
*    pointed by "pvram".
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
extern int apps_vram_store(int vid);


/**
* @brief read the vram content that already in sdram.
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
extern int apps_vram_read(int vid, void *pdata, long size);


/**
* @brief store the vram content to sdram.
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
extern int apps_vram_write(int vid, void *pdata, long size);


/**
* @brief set the vram to default value.
* @return 0-success and others-failed. 
*/
extern int apps_vram_set_default();



#endif
