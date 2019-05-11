#ifndef __WIRE_KEY_H
#define __WIRE_KEY_H

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>


#define WIRE_SWITCHKEY_DOWN	0x2000001
#define WIRE_SWITCHKEY_UP	0x1000001
#define WIRE_SWITCHKEY_HOLD	0x3000001


bool is_wire_key_event(int fd, fd_set *fds);

int wire_key_handler(int fd);



#endif
