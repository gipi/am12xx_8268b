#ifndef _APPS_VRAM_DEFAULT_H
#define _APPS_VRAM_DEFAULT_H

/**
* system related default values.
*/
#define    SYSDEFAULT_LOCAL_LANGUAGE    (-1)
#define    SYSDEFAULT_VOLUME    8

/**
albums related
**/
#define SYSDEFAULT_IS_UDISK_DIRTY (0)


/**
* lcd related default values.
*/
#define    SYSDEFAULT_BACKLIGHT_STRENGTH    21


/**
*color
*/
#define  SYSDEFAULT_COLOR_BIRGHTNESS  0     //Adjust the brightness level of the output picture [-128,127]
#define  SYSDEFAULT_COLOR_CONSTRAST   0   	//Adjust the contrast of the output picture [-64,64]
#define SYSDEFAULT_COLOR_SATURATION 0   	//Adjust the color saturation of the output picture [-64,128]
#define SYSDEFAULT_COLOR_HUE 0			//Adjust the color hue of the output picture [-60,60]
#define SYSDEFAULT_COLOR_SHARPNESS 0		//Adjust the color sharpness of the output picture [-31,31]

/**
* photo related default values.
*/
#define    SYSDEFAULT_PHOTO_DISP_RATIO        0
#define    SYSDEFAULT_PHOTO_BKMUSIC_EN        1
#define    SYSDEFAULT_PHOTO_BACKGROUND_EFFECT        0
#define    SYSDEFAULT_PHOTO_SLIDE_INTERVAL    10
#define    SYSDEFAULT_PHOTO_SLIDE_MODE        0
#define    SYSDEFAULT_PHOTO_SLIDE_EFFECT      0
#define    SYSDEFAULT_PHOTO_CLOCKSHOW_EN      0
#define    SYSDEFAULT_PHOTO_AUTOROTATION_EN      0

/**
* music related default values.
*/
#define    SYSDEFAULT_MUSIC_EFFECT    0
#define    SYSDEFAULT_MUSIC_PLAY_MODE        0
#define 	 SYSDEFAULT_AUDIO_AUTOPLAY_MODE 0

/**
* video related default values.
*/
#define    SYSDEFAULT_VIDEO_DISP_RATIO       0
#define    SYSDEFAULT_VIDEO_PLAY_MODE        3
#define 	 SYSDEFAULT_VIDEO_AUTOPLAY_MODE 0

/**
* calendar related default values.
*/
#define    SYSDEFAULT_CALENDAR_CLOCK_MODE    0

/**
*auto power on and off
**/
#define SYSDEFAULT_AUTO_POWER_ON_ENABLE 0
#define SYSDEFAULT_AUTO_POWER_OFF_ENABLE 0
#define SYSDEFAULT_AUTO_POWER_FREQ_ONCE 128
/**
*printer
**/
#define SYSDEFAULT_PRINTER_ENABLE 0

/**
*HDMI output
**/
#define SYSDEFAULT_SCREEN_OUTPUT_MODE 16

#define SYSDEFAULT_OUTPUT_MODE 1

#define SYSDEFAULT_EBOOK_TEXT_COLOR		0xFFFFFF
#define SYSDEFAULT_EBOOK_TEXT_SIZE		16

#define SYSDEFAULT_AUTO_SCREEN_ON_ENABLE 0

#endif
