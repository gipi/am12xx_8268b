#include <stdio.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h> 
#include <stdlib.h>


#include "htmlsetting_cgi.h"
#include "am_socket.h"

int cgiMain()
{
	int ret;
	char cmd_string[512]={0};
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	cgiFormString("type", cmd_string, 512);
	
	ret=create_websetting_client(cmd_string);
	if(ret<0)
		return -1;
	websetting_client_write(cmd_string);
	 char receiveline[1000]={0}; 
	ret=websetting_client_read(receiveline);
	fprintf(cgiOut,"%s",receiveline);

if(ret)
	{
	htmlsetting_func("htmlsetting_snapshot_save", NULL, 0, NULL, NULL);
}
return ret;

	/* Send the content type, letting the browser know this is HTML */
#if 0	
	cgiHeaderContentType("text/html");
	//fprintf(cgiOut, "OK");
	//refresh every second
	fprintf(cgiOut, "<head><title> Actions-Micro | CBV </title><meta http-equiv=\"refresh\" content=\"3; URL=htmlsetting_snapshot.cgi\"></head>");
	fprintf(cgiOut, "<img src=\"../tmp/snapshot.jpg\" width=\"100%\"/>");
#else
	puts("use javascript instead...");
#endif	
	return 0;
}


