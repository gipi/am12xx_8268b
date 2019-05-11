#include "taskmanager_message.h"
#include "taskmanager_debug.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "sys_msg.h"
#include <sys/msg.h>
#include "key_map.h"

/**
* @brief ADC and GPIO key mapping.
*    note that macro with EZ_... will be connected to
*    future easy tools.
*/
#define    EZ_ADCKEY_0      BOARD_KEY_ENTER
#define    EZ_ADCKEY_1      BOARD_KEY_ENTER
#define    EZ_ADCKEY_2      BOARD_KEY_ENTER
#define    EZ_ADCKEY_3      BOARD_KEY_DOWN
#define    EZ_ADCKEY_4      BOARD_KEY_DOWN
#define    EZ_ADCKEY_5      BOARD_KEY_DOWN
#define    EZ_ADCKEY_6      BOARD_KEY_UP
#define    EZ_ADCKEY_7      BOARD_KEY_UP
#define    EZ_ADCKEY_8      BOARD_KEY_UP
#define    EZ_ADCKEY_9      BOARD_KEY_RIGHT
#define    EZ_ADCKEY_10     BOARD_KEY_RIGHT
#define    EZ_ADCKEY_11     BOARD_KEY_LEFT
#define    EZ_ADCKEY_12     BOARD_KEY_LEFT
#define    EZ_ADCKEY_13     BOARD_KEY_ESC
#define    EZ_ADCKEY_14     BOARD_KEY_ESC

#define    ADC_KEY_LEVEL    15

static int adckey_map_table[ADC_KEY_LEVEL]= \
{
	EZ_ADCKEY_0,  
	EZ_ADCKEY_1,  
	EZ_ADCKEY_2,  
	EZ_ADCKEY_3,  
	EZ_ADCKEY_4,  
	EZ_ADCKEY_5,  
	EZ_ADCKEY_6,  
	EZ_ADCKEY_7,  
	EZ_ADCKEY_8,  
	EZ_ADCKEY_9,  
	EZ_ADCKEY_10,  
	EZ_ADCKEY_11,  
	EZ_ADCKEY_12,  
	EZ_ADCKEY_13,  
	EZ_ADCKEY_14
};

    
/** 
* @brief map raw key to standard system key.
*/  
int taskmgr_syskey_map(struct am_sys_msg *rawkey)
{   
	int subtype;
	int keyval,keymap;
	int keyaction;
	int i;
    
	
	if(rawkey == NULL){
		return -1;
	}
    
	subtype = rawkey->subtype & KEY_SUBTYPE_MASK;
	keyval = rawkey->dataload & KEY_VALUE_MASK;
	keyaction = rawkey->dataload & KEY_ACTION_MASK;

	if(subtype == KEY_SUBTYPE_BOARD){
		
		/**
		* for ADC and GPIO key, mapping has been done
		* in the input system. So we do nothing here.
		*/
		return 0;
	}

	if(subtype == KEY_SUBTYPE_IR){
		/*
		for(i=0;i<IR_KEY_MAX;i++){
			if(keyval == irkey_map_table[i][0]){
				keymap = irkey_map_table[i][1];
				rawkey->dataload = keymap | keyaction;
				break;
			}
		}
		*/
		return 0;
	}

	return -1;
}


