#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
int cgiMain(int argc, char *argv[])
{
	int ret;
	char output_mode_index[5]={0};
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("output_mode_index", output_mode_index, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;		
	strncpy(cmd_string,output_mode_index,strlen(output_mode_index));
	strcat(cmd_string,"set_music_output");			
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}
