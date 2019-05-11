#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char res_value[10]={0};
	char current_res_value[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("res_value", res_value, 10);
	cgiFormStringNoNewlines("current_res_value", current_res_value, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	if(strstr(res_value,"OK")!= 0)
	{
		char temp[8];
		strncpy(temp,res_value,(strlen(res_value)-2));
		strncpy(cmd_string,"set_resolution",strlen("set_resolution"));
		strcat(cmd_string,"\n");
		strcat(cmd_string,temp);
		strcat(cmd_string,"\n");
		strcat(cmd_string,"OK");
	}
	else
	{
		strncpy(cmd_string,"set_resolution",strlen("set_resolution"));	
		strcat(cmd_string,"\n");
		strcat(cmd_string,res_value);
		strcat(cmd_string,"\n");	
		strcat(cmd_string,current_res_value);
		strcat(cmd_string,"\n");	
	}
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
