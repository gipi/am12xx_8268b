#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <dirent.h>

#include "cgic.h"
#include "am_cgi.h"
#include "htmlsetting_cgi.h"
#include "am_socket.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif


#define F_FILENAME      "fileName4"
#define FILENAME_LEN    1024
#ifdef BUF_LEN
	#undef BUF_LEN
#endif
#define BUF_LEN         10240
#define CMD_LEN			64
#define PATH "/tmp/domainpath"
#define LOG_PATH	"/tmp/upload_log"

#define FILE_HELPVIEW	"helpview.jpg"
#define FILE_APPINFO	"appInfo.json"
#define FILE_KEY		"key"
#define FILE_TRIALKEY	"trialkey"
#define FILE_SUBTITLE1	"ezsubtitle.srt"
#define FILE_SUBTITLE2	"ezsubtitle.smi"
#define FILE_SUBTITLE3	"ezsubtitle.ssa"
#define FILE_SUBTITLE4	"ezsubtitle.cdg"
#define FILE_SUBTITLE5	"time.txt"
#define FILE_SWF_BG		"main_customize.swf"
#define FILE_SWF_LOGO	"main_logo.swf"

#define FILE_SUBTITLE	"ezsubtitle"

#define SUPPORT_FILES	{\
							FILE_HELPVIEW,\
							FILE_APPINFO,\
							FILE_KEY,\
							FILE_TRIALKEY,\
							FILE_SUBTITLE1,\
							FILE_SUBTITLE2,\
							FILE_SUBTITLE3,\
							FILE_SUBTITLE4,\
							FILE_SUBTITLE5,\
							FILE_SWF_BG,\
							FILE_SWF_LOGO}

#define PATH_TMP		"/tmp"
#define PATH_EZDATA		"/tmp/ezdata"

#define WRITE_LOG	0

#if WRITE_LOG
#define EZCAST_WRITE_LOG(fmt, args...) do { struct timeval tv; gettimeofday(&tv,NULL); char msg[1024]; snprintf(msg, 1024, "-%06lu.%06lu- "fmt,tv.tv_sec,tv.tv_usec,##args); write_log(msg); } while (0)
#else
#define EZCAST_WRITE_LOG(fmt, args...)
#endif

#if 0
static int init_send_socket(struct sockaddr_un *addr, char *path){
	int sockfd;  
	sockfd=socket(AF_UNIX,SOCK_DGRAM,0);  
	if(sockfd<0)  
	{  
		return -1;  
	}  
	bzero(addr,sizeof(struct sockaddr_un));  
	addr->sun_family=AF_UNIX;  
	strcpy(addr->sun_path,path);  
	return sockfd;
}

static int send_to_socket(int sockfd, char msg[], const struct sockaddr_un * addr)  
{  
	int len;   
	len = strlen(addr->sun_path)+sizeof(addr->sun_family);  
	sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)addr,len);   
	return 1;  
}

static int upload_send_command(char *msg){
	int fd;  
    struct sockaddr_un addr;  

    if( (fd = init_send_socket(&addr, PATH)) > 0){
		send_to_socket(fd, msg, &addr); 
		close(fd);
		return 0;
	}

	return -1;
}
#endif

// **********************************************************************************
// For user swf ui
#define BUFSIZE 1024*16
#define SWF_PATH	"/am7x/case/data"
#define CKSUM_PATH	"/mnt/vram/swf_cksum"
#define CKSUM_FTYPE	"cksum"
unsigned int cal_checksum(unsigned char *ptr, unsigned int len, unsigned int oldCheckSum)
{
	unsigned int CheckSum,i;
	unsigned int *D_ptr;
	D_ptr=(unsigned int *)ptr;
	CheckSum=oldCheckSum;

	for(i=0;i<len/sizeof(unsigned int);i++)
	{
		CheckSum += *D_ptr;
		D_ptr++;
	}

	if(len%sizeof(unsigned int) > 0)
	{
		CheckSum += *D_ptr;
	}
	
	return CheckSum;
}

int dirCheck(char *dir)
{
	if(dir == NULL)
		return -1;
	if(access(dir, F_OK) == 0){
		DIR *dp = opendir(dir);
		if(dp != NULL){
			closedir(dp);
			return 0;
		}
		unlink(dir);
	}

	int ret = mkdir(dir, 0755);
	if(ret < 0){
		return -1;
	}
	return 0;
}

char *getSwfName(char *name)
{
	char *tmp = NULL;

	if(name == NULL)
		return NULL;

	tmp = strrchr(name, '.');
	if(tmp != NULL && strcmp(tmp+1, "ui") == 0)
	{
		*tmp = '\0';
		tmp = strrchr(name, '.');
		if(tmp != NULL)
		{
			tmp = strrchr(tmp, '_');
			if(tmp != NULL)
			{
				*tmp = '\0';
			}

			return name;
		}
	}

	return NULL;
}

int userSwfUiFile(cgiFilePtr cfp, char *file)
{
	char outPath[128];
	char cksumPath[128];
	char cmdbuf[128];
	int i = 0, ret = -1;
	FILE *fp1 = NULL, *fp2 = NULL;
	unsigned int checksum = 0, cksum = 0;
	unsigned int type = 0;
	unsigned char buff[BUFSIZE];
	if(file == NULL)
	{
		//printf("NULL pointer\n");
		return -1;
	}

	snprintf(outPath, sizeof(outPath), "%s/ctm_%s", CKSUM_PATH, file);
	snprintf(cksumPath, sizeof(cksumPath), "%s/ctm_%s.%s", CKSUM_PATH, file, CKSUM_FTYPE);
	snprintf(cmdbuf, sizeof(cmdbuf), "rm %s/ctm_%s", SWF_PATH, file);
       dirCheck(CKSUM_PATH);
	fp1 = fopen(cksumPath, "w");
	if(fp1 == NULL)
	{
		printf("Open %s  file fail\n",cksumPath);
		goto __END__;
	}
	fp2 = fopen(outPath, "w");
	if(fp2 == NULL)
	{
		printf("Open outPath file faile=%s\n",outPath);
		goto __END__;
	}

	if(cgiFormFileRead(cfp, (void *)&type, 4, &i) != cgiFormSuccess)
	{
              printf("Read  fail\n");
		goto __END__;
	}

	if(type == 1){
		if(cgiFormFileRead(cfp, (void *)&cksum, 4, &i) != cgiFormSuccess)
		{
			 printf("Read checksum in file fail\n");
			goto __END__;
		}
		
		memset(buff, 0, sizeof(buff));
		while(cgiFormFileRead(cfp, buff, sizeof(buff), &i) == cgiFormSuccess)
		{
			checksum = cal_checksum(buff, i, checksum);
			i=fwrite(buff, 1, i, fp2);
			if(i <= 0)
			{
				printf("Write  %s file fail\n",outPath);
				goto __END__;
			}
			//printf("Read size: %d\n", i);
			memset(buff, 0, sizeof(buff));
		}

		if(checksum != cksum)
		{
			fclose(fp2);
			fp2 = NULL;
			unlink(outPath);
			goto __END__;
		}

		

		ret = fwrite((void *)&checksum, 1, 4, fp1);
		if(ret <= 0)
		{
			fclose(fp2);
			fp2 = NULL;
			unlink(outPath);
			goto __END__;
		}
		fsync(fileno(fp1));
		fsync(fileno(fp2));
		system(cmdbuf);
		ret = 0;
		int ret0=create_websetting_client("reset_default");
		if(ret0<0)
			goto __END__;
		websetting_client_write("reset_default");
		close_websetting_socket_cli_fd();
	}

__END__:
	if(fp1 != NULL) fclose(fp1);
	if(fp2 != NULL) fclose(fp2);
	sync();
	return ret;
}

int isUserSwfUiFile(char *file)
{
	if(file == NULL)
	{
		return -1;
	}
     //  printf("get current swf name 1=%s\n",file);
	if(getSwfName(file) != NULL)
	{
	   //   printf("get current swf name=%s\n",file);
		if(strcmp(file, FILE_SWF_BG) == 0 || strcmp(file, FILE_SWF_LOGO) == 0)
			return 1;
	}

	return 0;
}




int cgiMain(int argc, char *argv[])
{
	cgiFilePtr cfp;
	char rfile[FILENAME_LEN] /*remote file */ ;
       int ret, fsize;
	if ((ret = cgiFormFileName(F_FILENAME, rfile, sizeof(rfile))) != cgiFormSuccess)
	{
		cgiHeaderContentType("text/html");
        	fprintf(cgiOut, "<p>No file was uploaded.(%d)</p>", ret);
       	 EZCAST_WRITE_LOG("No file was uploaded.(%d)", ret);
		fprintf(cgiOut, "<p> end </p>\n");
		sleep(3);
		return -1;
   	 }
	cgiHeaderContentType("text/html");
    	cgiFormFileSize(F_FILENAME, &fsize);
	fprintf(cgiOut,"<p>---- filename=%sfilesize: %d<p>",F_FILENAME, fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
	if(fsize >200*1024){
		printf("--fail-- The file is too large!!!\n");
        	EZCAST_WRITE_LOG("--fail-- The file is too large!!!\n");
		return -1;
	}

      if ((ret = cgiFormFileOpen(F_FILENAME, &cfp)) != cgiFormSuccess) {
       	 fprintf(cgiOut, "<p>open cgi file failed.(%d)</p>", ret);
       	 EZCAST_WRITE_LOG("open cgi file failed.(%d)", ret);
		//printf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
      		 return -1;
      }
	if(isUserSwfUiFile(rfile))
	{
		ret=userSwfUiFile(cfp, rfile);
		//return -1;
	}

	cgiFormFileClose(cfp);
	fprintf(cgiOut, "---- filesize: %d\n", fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
	sleep(3);
       fprintf(cgiOut, "<p> end </p>\n");
	fprintf(cgiOut,"</body>");
       return ret;
}
