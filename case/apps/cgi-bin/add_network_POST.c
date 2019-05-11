#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char ssid_result[40] = {0};
	char psk_result[40] = {0};
	char AuthenType[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ssid", ssid_result, 40);
	cgiFormStringNoNewlines("psk", psk_result, 40);
	cgiFormStringNoNewlines("AuthenType", AuthenType, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,ssid_result,strlen(ssid_result));
	strcat(cmd_string,"\n");
	strcat(cmd_string,psk_result);
	strcat(cmd_string,"\n");
	strcat(cmd_string,AuthenType);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"add_network");//add network
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
