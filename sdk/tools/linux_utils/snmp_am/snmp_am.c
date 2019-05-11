#include <stdio.h>
#include <stdlib.h>
#include "am_socket.h"
//snmp_am -c  cmd   //format get data
//snmp_am -c  cmd -v argv[1]  argv[2] ..//format set data   argv[2] is value usually
int main(int argc, char *argv[])
{
	int ret = -1;
	int argCount = 1;
	char cmd_string[512]={0};
	//printf(" argc:%d %s %s \n",argc,argv[1],argv[2]);
	if(argc<2){
		printf("args error!!!\n");
		exit(1);
	}
	//if(argc==3&&strncmp(argv[argCount], "-c", 2) == 0 && strlen(argv[argCount]) == 2 ) //-r
	if(strncmp(argv[argCount], "-c", 2) == 0 && strlen(argv[argCount]) == 2 ) //-r
	{
		if((argc == 3) ||(strncmp(argv[argCount + 2], "-v", 2) != 0)){
			sprintf(cmd_string, "%s", argv[++argCount]);
			while(argCount < argc -1){
		                    argCount++;
		                    strcat(cmd_string,"\n");
		                    strcat(cmd_string, argv[argCount]);
		                    if(argCount == argc - 1)
		                            strcat(cmd_string, "\n");
		            }
			//snprintf(cmd_string,sizeof(cmd_string),"%s",argv[argCount+1]);
			printf("set cmd:%s\n",cmd_string);
			ret=create_websetting_client(cmd_string);
			if(ret<0)
				return -1;
			websetting_client_write(cmd_string);
			char receiveline[1000]={0};
			ret=websetting_client_read(receiveline);
			printf("return_value:%s\n",receiveline);
			return 0;
		}
	}
	if( argc>4&&strncmp(argv[argCount], "-c", 2) == 0&&strlen(argv[argCount]) ==2&&strncmp(argv[argCount+2], "-v", 2) == 0 &&strlen(argv[argCount+2] )== 2) //-w
	{
		argCount=4;
		strncpy(cmd_string,argv[argCount],strlen(argv[argCount]));
		while(argCount<argc-1)
		{
			argCount++;
			strcat(cmd_string, "\n");
			strcat(cmd_string, argv[argCount]);	
			if(argCount==argc-1)
				strcat(cmd_string, "\n");
				
		}
		strcat(cmd_string, argv[2]);
		printf("set cmd:%s\n",cmd_string);
		ret=create_websetting_client("cgi_setting_server");
		if(ret<0)
			return -1;
		websetting_client_write(cmd_string);
		close_websetting_socket_cli_fd();
		return 0;


	}
	printf("format er!\n");
	return -1;
	
}

