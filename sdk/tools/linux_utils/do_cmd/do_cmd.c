#include <stdio.h>
#include <stdlib.h>
#include "soket_opt.h"

int main(int argc, char *argv[])
{
	int ret = -1;
	char cmd_string[512]={0};
	if(argc != 2){
		printf("args error!!!\n");
		exit(1);
	}
	snprintf(cmd_string, sizeof(cmd_string), "%s", argv[1]);
	printf("Do command: %s\n", cmd_string);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"do_command");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

