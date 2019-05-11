#ifndef MEDIA_HOTPLUG_H
#define MEDIA_HOTPLUG_H

#include "sys_msg.h"
#include <semaphore.h>

/**
* @brief the state machine state definition
*/
enum{
	HOTPLUG_STAT_IN=0,
	HOTPLUG_STAT_IN_CHECKING=1,
	HOTPLUG_STAT_OUT=2,
	HOTPLUG_STAT_OUT_CHECKING=3
};

/**
* @brief media type definition for hotplug.
*/
enum{
	HOTPLUG_MEDIA_CARD=0,
	HOTPLUG_MEDIA_USB=1,
	HOTPLUG_MEDIA_CF=2
};

/**
* @brief which hostcontroller usb wifi driver.
*/
enum{
	HCD_WIFI_DEFAULT=0,
	HCD_USE_WIFI=1,
	HCD_NEXT_USE_WIFI=2
};
/**
* @brief which usb device controller is working.
*/
enum{
	UDC_DEFAULT=0,
	UDC_WORK=1,     
	UDC_NEXT_WORK=2
};
/**
* @brief identy cdrom autorun working or not.
*/
enum{
	CDROM_CLOSE=0,
	CDROM_USE=1,
};
/**
* @brief the hotplug thread use this structure to 
*    manages the media plugging events.
*/

#define MEDIA_MSG_FIFO_LEN 64


struct media_hotplug_work
{
	sem_t  lockget,lockput;
	struct am_sys_msg msgfifo[MEDIA_MSG_FIFO_LEN];
	int rp, wp;
};


/**
* @brief check connect pc or not
*/
extern int check_connect_pc();

/**
* @brief open the hotplug work thread.
*/
extern int hotplug_open_work();

/**
* @brief close the hotplug work thread
*/
extern int hotplug_close_work();


/**
* @brief put msg to hotplug work fifo
* @param msg: msg to put
*/
extern int hotplug_put_msg(struct am_sys_msg msg);

/**
* @brief enable driver uninstall when media has been
*    plugged out.
*/
extern int hotplug_enable_plugout_process(int media_type);
extern int iphone_out();
#endif

