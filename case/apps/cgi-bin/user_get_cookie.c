#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "cgic.h"
#include "am_cgi.h"
#include <time.h>
#include "am_socket.h"
//char *cgiScriptName;
char username[512]="";
char userpassword[512]="";
 void Cookies(char *name,char *psk)
{
	char **array, **arrayStep;
	
	if (cgiCookies(&array) != cgiFormSuccess) {
		return;
	}
	arrayStep = array;
	//fprintf(cgiOut, "<table border=1>\n");
	//fprintf(cgiOut, "<tr><th>Cookie<th>Value</tr>\n");
	while (*arrayStep) {
		char value[1024];
		//fprintf(cgiOut, "<tr>");
		//fprintf(cgiOut, "<td>");
		//cgiHtmlEscape(*arrayStep);
		//fprintf(cgiOut, "<td>");
		cgiCookieString(*arrayStep, value, sizeof(value));
		if(strstr(*arrayStep,name)!=NULL)
			sprintf(username,"%s",value);
		if(strstr(*arrayStep,psk)!=NULL)
			sprintf(userpassword,"%s",value);
		//fprintf(cgiOut, "value%s\n",value);
		//cgiHtmlEscape(value);
		//fprintf(cgiOut, "\n");
		arrayStep++;
	}
	//fprintf(cgiOut, "</table>\n");
	cgiStringArrayFree(array);
}

int cgiMain(int argc, char *argv[])
{
	int ret=0;
	char cmd_string0[512]={0};
	char cmd_string[512]={0};
	char *name=NULL;
       char *psk=NULL;
	cgiFormString("type", cmd_string0, 512);
	name=strtok(cmd_string0,"\n");
       psk=strtok(NULL,"\n");
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	Cookies(name,psk);
       strncpy(cmd_string,username,strlen(username));
	strcat(cmd_string,"\n");
	strcat(cmd_string,userpassword);
	strcat(cmd_string,"\n"); 
	strcat(cmd_string,"compare_user_psk");
     //  fprintf(cgiOut, "%s",cmd_string);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	websetting_client_write(cmd_string);
	char receiveline[1000]={0}; 
	ret=websetting_client_read(receiveline);
	fprintf(cgiOut,"%s",receiveline);
	return ret;
}
