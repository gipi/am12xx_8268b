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
	char lan_index[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("lan_index", lan_index, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,"set_language",strlen("set_language"));
	strcat(cmd_string,"\n");
	strcat(cmd_string,lan_index);
	strcat(cmd_string,"\n");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
