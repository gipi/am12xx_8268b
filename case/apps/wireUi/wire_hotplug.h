#ifndef __WIRE_HOTPLUG_H
#define __WIRE_HOTPLUG_H

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include "sys_msg.h"

enum{
	HOTPLUG_STAT_IN=0,
	HOTPLUG_STAT_IN_CHECKING=1,
	HOTPLUG_STAT_OUT=2,
	HOTPLUG_STAT_OUT_CHECKING=3
};

enum _usb_storage_bus_flag{
	USB_STORAGE_BUS_DEFAULT = 0,
	USB_STORAGE_BUS_ONE,
	USB_STORAGE_BUS_TWO,
};

#define MEDIA_MSG_FIFO_LEN 64

typedef struct _media_hotplug_work
{
	sem_t  lock_out,lock_in;
	struct am_sys_msg msgfifo[MEDIA_MSG_FIFO_LEN];
	int rp, wp;
}M_HOTPLUG_WORK;

int wire_hotplug_handle_thread();
bool is_wire_usb_event(int fd, fd_set *fds);
int wire_usb_handler(int fd);
int initLibUsbHandle();
void deinitLibUsbHandle();
int get_usb_path(char *path);


#endif
