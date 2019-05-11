#ifndef _COMVAL_H
#define _COMVAL_H
#define MAGIC_COMVAL            0xdead

//language id
#define LAN_ID_SCHINESE       0
#define LAN_ID_ENGLISH        1
#define LAN_ID_TCHINESE       2

///////////////////////// LFIHead_t ///////////////////////////////////
//定义logical firmware image信息头的数据结构
typedef struct
{
  unsigned short magic;                /* MAGIC_COMVAL */
  
  // system time 
  int  year;
  char month;
  char day;
  char hour;
  char minute;
  char time_format;
  char is_display_time_on_heard_bar; /* 1 */
  
  //encrypt config
  char is_file_protected;       /* 1 */
  
  //player config;
  char sleep_time;              /* 0 */
  char idle_time;               /* 30 */
  
  //display config
  char backlight_duration;      /* 5 */
  char backlight_brightness;      /* 3 */
  
  //language config
  char language_id;             /* 2 simple chinese */
  
  //volume
  char voice_volume;             /* 15 */
  
  //misclleous
  char ReplayMode;
  char BatteryType;
  char Onlinedev;
  char SuppCard;
  char MTPFormatType;
  int screensaveTime;
  char Reserve[36];   //保留

}__attribute__ ((packed)) comval_t ;              //64字节

#endif

