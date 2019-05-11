#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "cgic.h"
#include "am_cgi.h"
#include <time.h>
char *cgiServerName;
#if 1
#define SERVER_NAME cgiServerName
#endif
//char *cgiScriptName;
#if 1
char username[128]="";
char userpassword[128]="";
int  CookieSet(char *cmd_string) 
 {
     char *name=NULL;
     char *psk=NULL;
     char *c_time=NULL;
     char setcmd_tmp[256];
     char domain_buf[512];
     int sec=0;
      if(cmd_string!=NULL)
		strncpy(setcmd_tmp,cmd_string,strlen(cmd_string));
	else 
		return -1;
	 name=strtok(setcmd_tmp,"\n");
        psk=strtok(NULL,"\n");      
	 c_time=strtok(NULL,"\n"); 
	 strncpy(domain_buf,c_time,strlen(c_time)-3);
	 sec=atoi(domain_buf);
	 sec+=86400;
        if (strlen(name)) {
                 /* Cookie lives for one day (or until browser chooses
                         to get rid of it, which may be immediately),
                         and applies only to this script on this site. */        
         cgiHeaderCookieSetStringforPro(name, psk,
                         sec, "/cgi-bin/", domain_buf);
	//  cgiHeaderCookieSetStringforPro("act_password3", psk,
              //          86400, cgiScriptName, SERVER_NAME);
        }
	cgiHeaderContentType("text/html");
	// Cookies();
	// fprintf(cgiOut,"username=%s c_time=%s  sec=%d\n,domain_buf=%s\n",username,c_time,sec,domain_buf); 
	 return 0;
 }
#endif
int cgiMain(int argc, char *argv[])
{
	int ret=0;
	char cmd_string[512]={0};
	cgiFormString("type", cmd_string, 512);
 	CookieSet(cmd_string);
      
	return ret;
}
