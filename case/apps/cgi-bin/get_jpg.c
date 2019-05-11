#include <stdio.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "htmlsetting_cgi.h"

#define CGI_DEBUG
#ifdef CGI_DEBUG
FILE *fpconsole;
#define CPRINT(fmt,arg...) fprintf(fpconsole, "CGI: "fmt, ##arg)
#endif

#define SNAPSHOT_PATH	"/mnt/user1/thttpd/html/tmp/snapshot.jpg"
#define HOSTNAME_PATH	"/proc/sys/kernel/hostname"

int check_hostname_is_valid()
{
	char Buffer[512];
	
#if 1 // read hostname
	char pathname[256];
	char hostname[256];
	//int hostname_size = 0;
	FILE *fp = NULL;
	sprintf(pathname, "%s", HOSTNAME_PATH);
	fp = fopen(pathname, "r");  
	if (fp == NULL)
	{    
#ifdef CGI_DEBUG    
		CPRINT("open failed: %s\n", pathname);
#endif    
		return -1;
	}

	memset(hostname, 0 , 256);
	fgets(hostname, 256, fp);
	
#ifdef CGI_DEBUG    
	CPRINT("hostname: %s  length(%d)\n", hostname, strlen(hostname));
#endif   

	fclose(fp);
#endif // end of read hostname

#ifdef CGI_DEBUG
	CPRINT("%d\n", cgiContentLength);
	CPRINT("%s\n", cgiRequestMethod);
	CPRINT("%s\n", cgiContentType);
#endif

	if (cgiContentLength > 0)
	{
		memset(Buffer, 0, sizeof(Buffer));
		fread( Buffer, cgiContentLength, 1, cgiIn);
#ifdef CGI_DEBUG
		CPRINT("cnt: %s\n", Buffer);
#endif
		if(!strncmp(hostname, Buffer, strlen(hostname) - 1))
		{			
#ifdef CGI_DEBUG
			CPRINT("hostname check success!!\n");
#endif	
			return 1;
		}
		else
		{
#ifdef CGI_DEBUG
			CPRINT("hostname check Fail %s != %s\n", hostname, Buffer);
#endif
			return -1;
		}
	}
	else
	{
#ifdef CGI_DEBUG
		CPRINT("No content\n");
#endif		
		return -1;
	}
}


int get_file_size(FILE *fp)
{
	long cur_pos;
	int file_size;
	if (fp == NULL)
	{
		return 0;
	}

	cur_pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, cur_pos, SEEK_SET);
	return file_size;
}

int cgiMain(int argc, char *argv[])
{
#ifdef CGI_DEBUG
	fpconsole = fopen("/dev/console", "w");
	fflush(fpconsole);
#endif    

#if 1  
	if (strncmp(cgiRequestMethod,"POST",4))
	{
#ifdef CGI_DEBUG    
		CPRINT("get jpeg: Wrong request method\n");
		fclose(fpconsole);
#endif
		fprintf(cgiOut, "Status: 405 Method Not Allowed\r\n");

		return -1;
	}

	if (check_hostname_is_valid() != 1)
	{
#ifdef CGI_DEBUG    
		CPRINT("get jpeg: Check hostname fail\n");
		fclose(fpconsole);
#endif
		fprintf(cgiOut, "Status: 401 Unauthorized\r\n");

		return -1;
	}
#endif

	int read_size, content_size;  
	int ret;
	int b_get_jpg_again = 0;
	char pathname[256], content_type[16];  
	char buf[50000];  
	FILE *fp = NULL;

	sprintf(content_type, "image/jpg");
	sprintf(pathname, "%s", SNAPSHOT_PATH);

	fp = fopen(pathname, "r");  
	if (fp == NULL)
	{    
#ifdef CGI_DEBUG    
		CPRINT("open failed: %s\n", pathname);
#endif

		htmlsetting_func("htmlsetting_snapshot_save", NULL, 0, NULL, NULL);

		//return -1;
		b_get_jpg_again = 1;
	}

	if (b_get_jpg_again)
	{
#ifdef CGI_DEBUG    
		CPRINT("Get jpg again after sending htmlsetting_snapshot_save command\n");
#endif

		fp = fopen(pathname, "r");  
		if (fp == NULL)
		{
#ifdef CGI_DEBUG    
			CPRINT("Get jpg again FAIL!!!!!!\n");
#endif
			return -1;
		}
	}

	content_size = get_file_size(fp);

#ifdef CGI_DEBUG
	CPRINT("file size: %d\n", content_size);
	fflush(fpconsole);
#endif

	fprintf(cgiOut, "Content-Type: %s\r\n", content_type);
	fprintf(cgiOut, "Content-Length: %d\r\n\r\n", content_size);

	fseek(fp, 0, SEEK_SET);
	read_size = 0;
	while (content_size)
	{
		ret = fread(buf, 1, 1, fp);
		if (ret > 0)
		{
			content_size -= ret;
			read_size += ret;
			//printf("%c", buf[0]);
			fprintf(cgiOut, "%c", buf[0]);
		}
		else
		{
			break;
		}
	}
#ifdef CGI_DEBUG
	if (content_size)
	{
		CPRINT("read failed at %d\n", read_size);
	}
	else
	{
		CPRINT("send complete (%d)\n", content_size);
	}
	fflush(fpconsole);
#endif
	fclose(fp);

#ifdef CGI_DEBUG
	fclose(fpconsole);
#endif

	if (content_size)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
