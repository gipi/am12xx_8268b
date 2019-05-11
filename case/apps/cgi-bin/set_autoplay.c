#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char set_cmd[40] = {0};
	char val[20] = {0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ezchannel_cmd", set_cmd, 40);
	cgiFormStringNoNewlines("ezchannel_open", val, 20);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,set_cmd,strlen(set_cmd));
	strcat(cmd_string,"\n");
	strcat(cmd_string,val);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"setAutoplayEnable");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();

	return 0;
}
