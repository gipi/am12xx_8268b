/**
@file: boot_display_api.h

@abstract: boot lcm module config relative definations head file.

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

#ifndef BOOT_DISPLAY_API_H
#define BOOT_DISPLAY_API_H


struct boot_device_info{
	unsigned int buffer_width;
	unsigned int buffer_height;
	unsigned int color_space;
};

void ms_delay(int n);//charles
void boot_set_gpio(int num,int value);
unsigned char boot_get_gpio(int num);





/**
*@brief API for boot mfp config 
*
*@param[in] config_data	: config file data 
*@retval 0		: success
*@retval	!0		: error
*/
void boot_mfp_config(void* config_data);

/**
*@brief API for boot read config 
*
*@param[in] filename	: config file name
*@param[in] fentry : flash driver api entry
*@param[in] data : buffer to be filled 
*@param[in] temp : temp buffer flash driver used
*@retval 0		: success
*@retval	!0		: error
*/
int boot_read_data(unsigned char* filename,void* fentry,unsigned char* data,unsigned char* temp);

/**
*@brief API for backlight boot init
*
*@param[in] config_data	: backlight config file data in sdram 
*@retval 0		: success
*@retval	!0		: error
*/
int boot_backlight_init(void* config_data);

/**
*@brief API for open backlight 
*
*@param[in] brightness : brightness value to be set
*@retval 0		: success
*@retval	!0		: error
*/
int boot_backlight_open(int brightness);

/**
*@brief API for lcm boot init
*
*@param[in] config_data	: lcm config file data in sdram 
*@retval 0		: success
*@retval	!0		: error
*/
int boot_lcm_init(void* config_data,void* color_data);


/**
*@brief API for display picture 
*
*@param[in] data	: raw picture data
*@param[in] format : data format ,RGB,YUV etc
*@retval 0		: success
*@retval	!0		: error
*/
int boot_logo_display(void* data); // FIXME may pass data format as an argument
//#define boot_lcm_display(data)  boot_logo_display(data,PIX_FMT_YCBCR_4_2_2_INTERLEAVED)

/**
*@brief API for lcm boot init
*
*@param[in] info	: device info to store  
*@retval 0		: success
*@retval	!0		: error
*/
int boot_set_device_info(void* info);


enum{
	DDR_CAP_64M=0,
	DDR_CAP_128M,
	DDR_CAP_256M,
};

int boot_get_ddr_cap();


/**
*@brief API for serial uart init
*
*@param[in] : none
*@retval 0		: success
*@retval	!0		: error
*/
int serial_init (void);

/**
*@brief API for serial uart printf
*
*@param[in]  format : printf format
*@retval 0		: success
*@retval	!0		: error
*/
int printf(const char *format, ...); //boot printf ,don't use gcc builtin printf(-fno-builtin-printf) 

int snprintf(char *str, unsigned int size, const char *format, ...);


#endif
