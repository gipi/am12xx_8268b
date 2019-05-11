#include <stdio.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int cgiMain(int argc, char *argv[])
{
    char *ip_address;
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
    cgiHeaderContentType("text/html");

	ip_address = getenv("REMOTE_ADDR");
	if(ip_address == NULL){
		//fprintf(cgiOut, "get ip address error!!\n");
		return -1;
	}
	fprintf(cgiOut, "%s", ip_address);
	
   // return 0;
}
