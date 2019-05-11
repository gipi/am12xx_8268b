#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include <unistd.h>
#include<dirent.h>
#include <string.h> 
#include"htmlsetting_cgi.h"

/*
translate the url data ?name=value&?.....
get name  and value separated

*/

/*
test_result format:
1 wifi 
2  chan1
3  chan2
4  chan3
5  edid
6 lan
7 version

"1= , 2=,3=,"




*/

static int get_keybuf_value(char *buf,char *key,char *value)
{
	char * locate1 = NULL;
	int key_len=0;
	locate1 = strstr(buf,key);
	if(locate1!=NULL)
	{
		char * locate2 = NULL;
		//if((locate2=strstr(locate1,"&"))==NULL)
		locate2 = strchr(locate1,',');
	
		if(locate2!=NULL)
		{
			key_len=strlen(key);
			
			memcpy(value,(locate1+key_len),locate2-locate1-key_len);

			//printf("key value=%s\n",value);
		}
		else
			return -1;
		//value=tmp;
	}
	else
		return -1;
	return 0;
}

static int show_result(char *buf)
{

	char value[64]="";
	if(!get_keybuf_value(buf,"1=",value)){
		fprintf(cgiOut,"Wifi Throuthput:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"2=",value)){
		fprintf(cgiOut,"Wifi Channel_1:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"3=",value)){
		fprintf(cgiOut,"Wifi Channel_6::%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"4=",value)){
		fprintf(cgiOut,"Wifi Channel_11:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"5=",value)){
		fprintf(cgiOut,"Edid Resolution:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"6=",value)){
		fprintf(cgiOut,"Version Test:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"7=",value)){
		fprintf(cgiOut,"Language Config:%s\n<br>",value);
		memset(value,0,64);
	}

	return 1;
}
int get_url_inputs()
{
	//char method[10];
	int post_len;
	char *inputstring=NULL;
	//char *index;
	int *Dongle_res = NULL,*Test_result=NULL,ret = -1;	
	/*
	strcpy(method,getenv("REQUEST_METHOD"));
	if(!strcmp(method,"POST"))
	{
		fprintf(cgiOut,"post!!!\n");	
		post_len=atoi(getenv("CONTENT_LENGHT"));
		if(post_len!=0)
		{
			inputstring=malloc(sizeof(char)*post_len+1);
			fread(inputstring,sizeof(char),post_len,stdin);	
		}
	}
	*/
	
		//inputstring=malloc(1024);
	inputstring=getenv("QUERY_STRING");
	post_len=strlen(inputstring);
	if(strlen(inputstring)>0)
	{
		//cmd  执行时间长 cgi会超时
		if(!strcmp(inputstring,"testresult=true")){
			htmlsetting_func("factory_test_cmd", (void*)inputstring, post_len, (void **)&Test_result, NULL);
			char *factory_test_result = (char *)Test_result;
			show_result(factory_test_result);
			del_return((void **)&Test_result);
		}
		else{
			htmlsetting_func("factory_test_cmd", (void*)inputstring, post_len, (void **)&Dongle_res, NULL);
			if(Dongle_res){
				ret = *Dongle_res;
				fprintf(cgiOut,"%d\n",ret);	
			}else{
				ret = -1;
				fprintf(cgiOut,"Fail to get Dongle Response!!!\n");	
			}
			del_return((void **)&Dongle_res);
		}
		
	}
	else 
	{
		return 0;
	}
	return 1;
}






int cgiMain(int argc,char*argv[])
{
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	int inputs=0;
	if((inputs=get_url_inputs())!=1)
		fprintf(cgiOut,"cgi Get inputs Error");
	return 0;
}






