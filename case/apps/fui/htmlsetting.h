#ifndef	_HTMLSETTING_H_
#define	_HTMLSETTING_H_

#include "ezcast_public.h"

#if EZCAST_ENABLE
#define HTMLSETTING_ENABLE
#endif

// the max connection number of the server  
#define MAX_CONNECTION_NUMBER 5
#define UNIX_SOCKET_PATCH	"/tmp/htmlsetting_patch"
#define	CMDLEN_MARK			"/c"
#define VALLEN_MARK			"/v"
#define SENDLEN_MARK		"/r"
#define END_MARK			"/e"

#define CLI_TIME_OUT				(60)

struct msglen_s{
	char flag[2];
	int len;
};

struct select_ctrl_s{
	struct select_ctrl_s *next;
	struct select_ctrl_s *prev;
	int fd;
	time_t priv_time;
};

struct funLink_s{
	char name[128];
	int (*func)(void *val, void *handle);
};

#endif
