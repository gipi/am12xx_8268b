#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "am_cgi.h"
#include <stddef.h>
#include <pthread.h>
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("connection_mode_value", cmd_string, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_router_ctl");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
