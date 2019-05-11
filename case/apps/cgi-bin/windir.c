#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include <unistd.h>
#include<dirent.h>
#include <string.h> 
static int save_setting(char *dir,char *user,char *pass)
{

	FILE *fp;
	fp=fopen("/tmp/winsha_setting.bat","w+");
	if(fp!=NULL)
	{
		fputs(dir,fp);
		fputs("\r\n",fp);
		fputs(user,fp);
		fputs("\r\n",fp);
		fputs(pass,fp);
		fputs("\r\n",fp);
		fclose(fp);
	}
	return 1;

}
static int get_setting(char *set_val)
{

	FILE *fp;
	char val[3][80];
	char tmp[80];
	int i=0;
	int exist=access("/tmp/winsha_setting.bat",0);
	if(exist==0)
	{
		fp=fopen("/tmp/winsha_setting.bat","r");
		if(fp!=NULL)
		{
			for(i=0;i<3;i++)
			{
				fgets(tmp,80,fp);
				char *end = strchr(tmp, '\n');
			   	if(end != NULL){
				   char *end_r = strchr(tmp, '\r');
				   	if(end_r != NULL){
						   end = (end < end_r)?end:end_r;
				   }
				
			   	}
				int len = end - tmp;
				strncpy(val[i],tmp,len);
				val[i][len]='\0';
				memset(tmp,0,80);
			}
			fclose(fp);
			sprintf(set_val,"%s&%s&%s",val[0],val[1],val[2]);
		}
	}
	return 1;
}
int cgi_write_time(char *time)
{
	//printf("create index filein usb \n");
	FILE *fp;
	fp=fopen("/mnt/vram/time.txt","w+");
	if(fp!=NULL)
	{
		//printf("write index filein usb \n");
		fputs(time,fp);
		fputs("\r\n",fp);
		fclose(fp);
	}
	else
		{
		fclose(fp);



	}

}
int cgiMain(int argc,char*argv[])
{
	
	
	char fullname[240]="";
	char tmp[240]="";
	char sharedir[80]="";
	char username[80]="";
	char password[80]="";
	char *cmd;
	char *index;
	int len=0;
	//char *dir;
	char cmd_boot[16]="reboot";
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	char js_time[32]="";
	cgiFormString("fullname", fullname, 240);
	if(!strcmp(fullname,"pre_sets")){
		get_setting(tmp);
		fprintf(cgiOut,"%s",tmp);	
	}
	else if(strstr(fullname,"reboot")){
		fprintf(cgiOut,"%s",fullname);	
		char *p=strchr(fullname,'*');
		if(p!=NULL){
			p++;
		char y1=*(p++);
		char y2=*(p++);
		char y3=*(p++);
		char y4=*(p++);
		char m1=*(p++);
		char m2=*(p++);
		char d1=*(p++);
		char d2=*(p++);
		char h1=*(p++);
		char h2=*(p++);
		char min1=*(p++);
		char min2=*(p++);
		char s1=*(p++);
		char s2=*(p++);
		sprintf(js_time,"%c%c%c%c.%c%c.%c%c-%c%c:%c%c:%c%c",y1,y2,y3,y4,m1,m2,d1,d2,h1,h2,min1,min2,s1,s2);
		fprintf(cgiOut,"%s",js_time);
		cgi_write_time(js_time);
	}else{fprintf(cgiOut,"* not found");}
		htmlsetting_func("system_cmd", (void*)cmd_boot, strlen(cmd_boot), NULL, NULL);	
	}
	else
	{
		index=strstr(fullname,"&");
		//printf("index=%d fullname=%d",index,fullname);
		if(index!=NULL)
		{
			strncpy(sharedir,fullname,(index-fullname));
			//fprintf(cgiOut,"[sharedir=]%s",sharedir);
			
			len=index-fullname+1;
			//printf("%d",len);
			len=strlen(fullname)-len;
			strncpy(tmp,(index+1),len);
			//printf("[tmp=]%s",tmp);
		}
		index=strstr(tmp,"&");
		if(index!=NULL){
			//printf("index=%d tmp=%d",index,tmp);
			strncpy(username,tmp,(index-tmp));
			//fprintf(cgiOut,"[username=]%s",username);
			len=index-tmp+1;
			len=strlen(tmp)-len;
			//fprintf("%d",len);
			strncpy(password,(index+1),len);
			//fprintf(cgiOut,"[password=]%s",password);
		}
		sprintf(cmd,"mount -t cifs %s /mnt/user1/thttpd/html/airusb/windows -o rw,file_mode=0666,username=%s,password=%s",sharedir,username,password);
		//fprintf(cgiOut,"mount shared directory:%s",cmd);
		htmlsetting_func("system_cmd", (void*)cmd, strlen(cmd), NULL, NULL);
		save_setting(sharedir,username,password);
	}
	///system(cmd);
	cmd=NULL;
	index=NULL;
	return 1;
		
	
}



