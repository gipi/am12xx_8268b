#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain()
{
	int ret;
	char cmd_string[100]={0};
	char ssid_result[40] = {0};
	char ssid_index[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ssid_index", ssid_index, 5);
	cgiFormStringNoNewlines("ssid", ssid_result, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,ssid_index,strlen(ssid_index));
	strcat(cmd_string,"\n");
	strcat(cmd_string,ssid_result);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"delete_password");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
