#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic.h"
#include "am_cgi.h"

struct process_list_s{
	char name[64];
	int (*cgiMain)(int argc, char *argv[]);
};





int set_defaultmode_POST(int argc, char *argv[])
{
	int ret=0,val=0,val_old=0;
	char callbuf[24]={0};
	char default_mode_value[5]={0};
	FILE *fp = NULL;
 
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("Default_mode_value", default_mode_value, 5);
	val=atoi(default_mode_value);
	fp=fopen("/mnt/vram/ezcast/DEFAULTMODE","r");
	if(fp!=NULL)
	{
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		val_old=atoi(callbuf);
		if(val==val_old)
			return 0;
	}		
	if(val==1)
	{
		system("echo 1 > /mnt/vram/ezcast/DEFAULTMODE");
		system("sync");

	}
	else
	{
		system("echo 2 > /mnt/vram/ezcast/DEFAULTMODE");
		system("sync");
	}
	
	return 0;
}
int get_defaultmode(int argc, char *argv[])
{
	int ret=0;
	char callbuf[24]={0};
	char cmd_string[512]={0};
	int mode=2;
    	cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    	cgiHeaderPragma("no-cache");
    	cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	cgiFormString("type", cmd_string, 512);
	if(strstr(cmd_string,"get_default_mode")){
		FILE *fp = NULL;
		fp=fopen("/mnt/vram/ezcast/DEFAULTMODE","r");
		if(fp==NULL)
		{
			system("echo 2 > /mnt/vram/ezcast/DEFAULTMODE");
			system("sync");
			fprintf(cgiOut,"%d",mode);
			return ret;
		}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		fprintf(cgiOut,"%s",callbuf);		
	}
	else
	{
		mode=2;
		fprintf(cgiOut,"%d\n",mode);	
	}
	return ret;
}







int cgiMain(int argc, char *argv[])
{
	struct process_list_s process_list[]={
		
		{"set_defaultmode_POST.cgi", set_defaultmode_POST},
		{"get_defaultmode.cgi", get_defaultmode},

	};

	if(argc > 0)
	{
		char *pn = getenv("SCRIPT_NAME");
		char *process = NULL;
		int i = 0;
		process = strrchr(pn, '/');
		if(process == NULL)
			process = pn;
		else
			process++;

		if(process == NULL)
			return -1;

		for(i=0; i<sizeof(process_list)/sizeof(struct process_list_s); i++)
		{
			if(strcmp(process, process_list[i].name) == 0)
			{
				return process_list[i].cgiMain(argc, argv);
			}
		}
	}

	return -1;
}

