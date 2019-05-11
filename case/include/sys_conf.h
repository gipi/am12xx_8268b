#ifndef SYS_CONFIG__H
#define SYS_CONFIG__H

enum{
	CHIP_7211=0,
	CHIP_7213,
	CHIP_7511,
	CHIP_7212,
	CHIP_7531,
	CHIP_7331,
	CHIP_7333,
	CHIP_7338
};


#define    IMAGE_EXT                 "JPG JPEG BMP TIF TIFF"
#define    IMAGE_EXT_ONLYJPEG        "JPG JPEG"


#if AM_CHIP_ID == 1205
#define    MUSIC_FILE_TYPE        "MP3 WMA"
#elif AM_CHIP_ID == 1203
#define    MUSIC_FILE_TYPE        "MP3 WMA OGG WAV AAC"
#elif AM_CHIP_ID == 1211
#define    MUSIC_FILE_TYPE        "MP3 WMA OGG WAV AAC"
#elif AM_CHIP_ID == 1220
#define    MUSIC_FILE_TYPE        "MP3 WMA OGG WAV AAC"
#elif AM_CHIP_ID == 1213
#define    MUSIC_FILE_TYPE        "MP3 WMA OGG WAV AAC"
#else
#define    MUSIC_FILE_TYPE        "MP3 WMA OGG WAV"
#endif

#define    VIDEO_FILE_TYPE        "AVI MP4 MOV 3GP"
#define    EBOOK_FILE_TYPE        "TXT"
#define    VVD_FILE_TYPE          "VVD"

#if AM_CHIP_ID == 1205
#define    FILEMGR_FILE_TYPE        "TXT MP3 WMA AVI MP4 MOV 3GP JPG JPEG BMP TIF TIFF"
#else
#define    FILEMGR_FILE_TYPE        "TXT MP3 WMA OGG WAV AVI MP4 MOV 3GP JPG JPEG BMP TIF TIFF"
#endif

/**
* face detection enable
*/
//#define _FACE_DETECTION_EN

//#define ROTATE_SENSOR_EN

#endif
