/**
@file: welcome.h

@abstract: welcome app relative definations head file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Boot Display;
 *  include code recover, upgrade, welcome logo	 
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
 */

#ifndef WELCOME_H
#define WELCOME_H

/** config file internal offset **/
#define  CONFIG_DATA_OFFSET		48	
/*** relative config file name ***/
#define  LCM_DATA_FILE			"LCM     BIN" 
#define  COLOR_DATA_FILE		"GAMMA   BIN"
#define  BACKLIHT_DATA_FILE 	"BACKLIGHBIN"
#define  PLATFORM_DATA_FILE		"GPIO    BIN"	
/*** for display data simulate **/
//#define  PIC_SIMULATE


/*** memory used by welcome **/
#define  BASE_ADDR		0xa0000000+32*1024*1024  //set to 32MB
/**   memory gap between config file ,asume per file is less then 2MB in size  ***/
#define  MEMGAP_SIZE	2*1024*1024			

/** show pic type **/
#define  SHOW_TYPE_RECOVER		0
#define  SHOW_TYPE_CARD			1
#define  SHOW_TYPE_UPGRAGD		2
#define  SHOW_TYPE_WELCOME		3
#define  SHOW_TYPE_COVER		4
#define  SHOW_TYPE_FAILED		5
#define  SHOW_TYPE_SUCCESSFUL	6

struct welcome_global_data{
	/*** boot address  allocation ****/
	unsigned int lcm_config_addr;
	unsigned int lcm_color_addr;
	unsigned int backlight_config_addr;
	unsigned int pic_data_addr ;
	unsigned int platform_data_addr ;
	/*** default backlight brightness **/
	unsigned int default_brightness;
	/** flag for if lcm is init ? **/
	int  lcm_init_flag ;
};
/*
typedef enum
{
 	S_INIT,
    S_WAIT,
    S_RUNNING,
    S_FINISHED,
    S_ERROR
}Fwu_state_t;   //表示upgrade.drv的运行状态

typedef struct
{
    int prg;    //进度值，0～100，由upgrade.drv写入
    Fwu_state_t state;
}Fwu_status_t;      //upgrade.drv返回的结构体
*/



extern void init_global_buffer(void);

extern void load_resource(unsigned int fentry,unsigned int file_name);



#endif
