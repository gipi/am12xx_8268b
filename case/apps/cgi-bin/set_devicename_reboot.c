#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("devicename", cmd_string, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_devicename_reboot");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
