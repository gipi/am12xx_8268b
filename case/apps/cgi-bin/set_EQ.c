#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char eq_index[5]={0};
	char custom_index[5]={0};
	char eq_value[5]={0};
	char custom_or_other[1]={0};
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("eq_index", eq_index, 5);
	cgiFormStringNoNewlines("custom_index", custom_index, 5);
	cgiFormStringNoNewlines("eq_value", eq_value, 5);
	cgiFormStringNoNewlines("custom_or_other", custom_or_other, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;		
	if(!strcmp(custom_or_other,"1"))
	{		
		strcpy(cmd_string,eq_value);
		strcat(cmd_string,"\n");
		strcat(cmd_string,custom_index);
		strcat(cmd_string,"\n");
		strcat(cmd_string,"set_EQ_Custom");		
	}
	else
	{
		strcpy(cmd_string,eq_index);
		strcat(cmd_string,"set_EQ_default");			
	}
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
