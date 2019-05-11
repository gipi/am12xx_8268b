/**
* @brief all kinds of key mapping.
*
* @author Simon Lee
* @date: 2010.05.20
* @version: draft version 
*/

#ifndef _KEY_MAP_H
#define _KEY_MAP_H

#include "sys_msg.h"

/**
* board key (ADC and GPIO) mapping space
*/

/** ADC key */
#define    BOARD_KEY_MAPPING_BASE     0
#define    BOARD_KEY_ENTER            (BOARD_KEY_MAPPING_BASE+0x0)              
#define    BOARD_KEY_UP               (BOARD_KEY_MAPPING_BASE+0x1)
#define    BOARD_KEY_DOWN             (BOARD_KEY_MAPPING_BASE+0x2)
#define    BOARD_KEY_LEFT             (BOARD_KEY_MAPPING_BASE+0x3)
#define    BOARD_KEY_RIGHT            (BOARD_KEY_MAPPING_BASE+0x4)
#define    BOARD_KEY_ESC              (BOARD_KEY_MAPPING_BASE+0x5)

/** GPIO key */
#define    BOARD_KEY_GPIO1            (BOARD_KEY_MAPPING_BASE+0x10)
#define    BOARD_KEY_GPIO2            (BOARD_KEY_MAPPING_BASE+0x11)
#define    BOARD_KEY_GPIO3            (BOARD_KEY_MAPPING_BASE+0x12)
#define    BOARD_KEY_GPIO4            (BOARD_KEY_MAPPING_BASE+0x13)
#define    BOARD_KEY_GPIO5            (BOARD_KEY_MAPPING_BASE+0x14)
#define    BOARD_KEY_GPIO6            (BOARD_KEY_MAPPING_BASE+0x15)


/**
* IR key mapping space
* @note ir key range is 32-132
*/
#define    IR_KEY_MAPPING_BASE       0x100
#define    IR_KEY_NUMBER             100
#define    IR_KEY_MAPPING_MAX        (IR_KEY_MAPPING_BASE+IR_KEY_NUMBER)

#define    IR_KEY_COPY			(IR_KEY_MAPPING_BASE+0x1)   
#define    IR_KEY_MUTE			(IR_KEY_MAPPING_BASE+0x2)  
#define    IR_KEY_VOLPLUS		(IR_KEY_MAPPING_BASE+0x3)  
#define    IR_KEY_ZOOMIN			(IR_KEY_MAPPING_BASE+0x4)  
#define    IR_KEY_DELETE			(IR_KEY_MAPPING_BASE+0x5)  
#define    IR_KEY_HOT_MENU		(IR_KEY_MAPPING_BASE+0x6)  
#define    IR_KEY_VOLMINUS		(IR_KEY_MAPPING_BASE+0x7) 
#define    IR_KEY_HOT_FILE		(IR_KEY_MAPPING_BASE+0x8)
#define    IR_KEY_POWER			(IR_KEY_MAPPING_BASE+0x9)  
#define    IR_KEY_RIGHT_ROTATE	(IR_KEY_MAPPING_BASE+0xa)   
#define    IR_KEY_HOT_VIDEO		(IR_KEY_MAPPING_BASE+0xb)  
#define    IR_KEY_VIEW			(IR_KEY_MAPPING_BASE+0xc)  
#define    IR_KEY_ESC				(IR_KEY_MAPPING_BASE+0xd)
#define    IR_KEY_SOURCE			(IR_KEY_MAPPING_BASE+0xe)  
#define    IR_KEY_UP				(IR_KEY_MAPPING_BASE+0xf)  
#define    IR_KEY_PREV			(IR_KEY_MAPPING_BASE+0x10)  
#define    IR_KEY_DOWN			(IR_KEY_MAPPING_BASE+0x11)  
#define    IR_KEY_LEFT			(IR_KEY_MAPPING_BASE+0x12)  
#define    IR_KEY_ENTER			(IR_KEY_MAPPING_BASE+0x13)  
#define    IR_KEY_RIGHT			(IR_KEY_MAPPING_BASE+0x14)  
#define    IR_KEY_PLAY			(IR_KEY_MAPPING_BASE+0x15)
#define    IR_KEY_NEXT			(IR_KEY_MAPPING_BASE+0x16)  
#define 	 IR_KEY_HOT_VVD		(IR_KEY_MAPPING_BASE+0x17)
#define 	 IR_KEY_HOT_PLAYER	(IR_KEY_MAPPING_BASE+0x18)
#define    IR_KEY_HOT_SETUP		(IR_KEY_MAPPING_BASE+0x19) 
#define  	 IR_KEY_HOT_CALENDAR	(IR_KEY_MAPPING_BASE+0x1a)
#define    IR_KEY_HOT_PHOTO		(IR_KEY_MAPPING_BASE+0x1b) 
#define    IR_KEY_HOT_MUSIC		(IR_KEY_MAPPING_BASE+0x1c)  
#define    IR_KEY_HOT_TEXT		(IR_KEY_MAPPING_BASE+0x1d)
#define 	 IR_KEY_REV				(IR_KEY_MAPPING_BASE+0x1e)
/*
*@add by readboy keyboard
*@ shift and capslock key map
*/
#define   VK_KEY_SHIFT  	   		 0x43
#define   VK_KEY_CAPSLOCK  		 0x85
#define   VK_SWF_SHIFT			 16
#define   VK_SWF_CAPSLOCK		 20
#endif

