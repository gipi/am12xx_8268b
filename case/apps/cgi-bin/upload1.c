#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "cgic.h"
#include "am_cgi.h"
#include "htmlsetting_cgi.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif

#define F_FILENAME      "fileName1"
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
#define FILE_SUBTITLE	"ezsubtitle"

#define SUPPORT_FILES	{\
							FILE_HELPVIEW,\
							FILE_APPINFO,\
							FILE_KEY,\
							FILE_TRIALKEY,\
							FILE_SUBTITLE1,\
							FILE_SUBTITLE2,\
							FILE_SUBTITLE3 \
						}

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

int isFileInvalid(char *fileName){
	if(fileName == NULL){
		return -1;
	}

	int i = 0;
	char files[][64] = SUPPORT_FILES;

	for(i=0; i<sizeof(files)/sizeof(files[0]); i++){
		if(strcmp(fileName, files[i]) == 0)
			return 0;
		else{
			char *p = strrchr(fileName, '.');
			if(p != NULL && strcmp(p, ".pem") == 0)
				return 0;
		}
	}

	return 1;
}

static void write_log(char *str){
	if(access(LOG_PATH, F_OK) == 0){
		struct stat info;
		stat(LOG_PATH, &info);
		if(info.st_size >= 102400)
			unlink(LOG_PATH);
	}

	FILE *fp = fopen(LOG_PATH, "a");
	if(fp != NULL){
		fwrite(str, 1, strlen(str), fp);
		fclose(fp);
	}
}

static int isSubtitle(char *file_name){
	char *tmp;

	tmp = strrchr(file_name, '.');
	if((tmp-file_name)!=strlen(FILE_SUBTITLE)){
		//printf("the length of \"%s\" is: %d, the length of \"%s\" is: %d\n", file_name, (tmp-file_name), FILE_SUBTITLE, strlen(FILE_SUBTITLE));
		return -1;
	}
	if(!strncmp(file_name, FILE_SUBTITLE, strlen(FILE_SUBTITLE))){
		return 0;
	}

	return -1;
}

static int file_to_command(char *file_name, char *command){
	//char cmd[256];
	
	if(!strcmp(file_name, FILE_HELPVIEW)){
		strncpy(command, CMD_HELPVIEW, strlen(CMD_HELPVIEW));
		return 0;
	}
	if(!strcmp(file_name, FILE_APPINFO)){
		strncpy(command, CMD_APPINFO, strlen(CMD_APPINFO));
		return 0;
	}
	//if(!strcmp(file_name, FILE_SUBTITLE1) || !strcmp(file_name, FILE_SUBTITLE2) || !strcmp(file_name, FILE_SUBTITLE3)){
	//if(!isSubtitle(file_name)){
	//	snprintf(cmd, sizeof(cmd), "mv %s/%s %s/%s", PATH_TMP, file_name, PATH_EZDATA, file_name);
	//	system(cmd);
	//}
	if(!strcmp(file_name, FILE_KEY)){
		strncpy(command, CMD_KEY, strlen(CMD_KEY));
		return 0;
	}
	if(!strcmp(file_name, FILE_TRIALKEY)){
		strncpy(command, CMD_TRIALKEY, strlen(CMD_TRIALKEY));
		return 0;
	}

	return 1;
}


static int Is_CApemfile(char *filepath){
	if(strstr(filepath,".pem"))
		return 1;
	else
		return 0;
}
static int Is_Timesyncfile(char *filepath){
	if(strcmp(filepath,"time.txt"))
		return 0;
	else
		return 1;
}
int wifi_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			printf("Make Dir Failed=%s\n",dir_path);
		}
		//system("sync");
	}
	return rtn;	
}

int cgiMain(int argc, char *argv[])
{
    cgiFilePtr cfp;
    char rfile[FILENAME_LEN] /*remote file */ ,
        lfile[FILENAME_LEN] /* local file */ , buf[BUF_LEN], 
        bakfile[FILENAME_LEN],
        cmd[CMD_LEN];
    int ret, fd, fsize, got;
	int i=0;
	
    if ((ret = cgiFormFileName(F_FILENAME, rfile, sizeof(rfile))) !=
        cgiFormSuccess) {
		cgiHeaderContentType("text/html");
        fprintf(cgiOut, "<p>No file was uploaded.(%d)</p>", ret);
        EZCAST_WRITE_LOG("No file was uploaded.(%d)", ret);
		cgiHeaderContentType("text/html");
		
		fprintf(cgiOut, "<p> end </p>\n");
		sleep(3);
		//printf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
    }
	 cgiHeaderContentType("text/html");
    cgiFormFileSize(F_FILENAME, &fsize);
	fprintf(cgiOut,"<p>---- filename=%sfilesize: %d<p>",F_FILENAME, fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
   
	if(fsize > 2*1024*1024){
		printf("--fail-- The file is too large!!!\n");
        EZCAST_WRITE_LOG("--fail-- The file is too large!!!\n");
		//printf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
	}
	if(isFileInvalid(rfile) != 0){
		printf("--fail-- The file[%s] is invalid!!!\n", rfile);
        EZCAST_WRITE_LOG("--fail-- The file[%s] is invalid!!!\n", rfile);
		//fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
	}

    if ((ret = cgiFormFileOpen(F_FILENAME, &cfp)) != cgiFormSuccess) {
        fprintf(cgiOut, "<p>open cgi file failed.(%d)</p>", ret);
        EZCAST_WRITE_LOG("open cgi file failed.(%d)", ret);
		//printf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
        return -1;
    }
	if(!isSubtitle(rfile)){
    	snprintf(lfile, sizeof(lfile), "/tmp/ezdata/%s", rfile);
	}else{
    	snprintf(lfile, sizeof(lfile), "/tmp/%s", rfile);
	}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	if(Is_CApemfile(rfile)){
		wifi_create_dir("/etc/cert/");
		//sprintf(cmd,¡ê?"%s","rm /etc/ssl/ca/*.pem");
		//htmlsetting_func("system_cmd", (void*)cmd, strlen(cmd), NULL, NULL);
		memset(lfile,0,FILENAME_LEN);
		snprintf(lfile, sizeof(lfile), "/etc/cert/CA.pem");	
	}
	
#endif
    fprintf(cgiOut, "write to %s\n", lfile);
	EZCAST_WRITE_LOG("write to %s\n", lfile);
    snprintf(bakfile, sizeof(bakfile), "%s.bak", lfile);
    if (access(lfile, F_OK) == 0)
        rename(lfile, bakfile);
    if ((fd = open(lfile, O_CREAT | O_WRONLY,
              S_IRWXU | S_IRWXG | S_IRWXO)) < 0) {
        fprintf(cgiOut, "<p>create file on server failed.(%s)</p>",
                strerror(errno));
		EZCAST_WRITE_LOG("create file on server failed.(%s)", strerror(errno));
		//printf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
        rename(bakfile, lfile);
        return -1;
    }
    while (cgiFormFileRead(cfp, buf, sizeof(buf), &got) == cgiFormSuccess) {
        if ((ret = write(fd, buf, got)) < 0) {
            fprintf(cgiOut, "<p> write failed(%d/%s)</p>\n", errno,
                    strerror(errno));
			EZCAST_WRITE_LOG("write failed(%d/%s)\n", errno, strerror(errno));
        }
        fprintf(cgiOut, "<p>got(%d)</p>\n", got);
		EZCAST_WRITE_LOG("got(%d)\n", got);
    }
    close(fd);
    cgiFormFileClose(cfp);
    remove(bakfile);
	fprintf(cgiOut, "---- filesize: %d\n", fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
	sleep(3);
	
    fprintf(cgiOut, "<p> end </p>\n");
	//fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
	//fprintf(cgiOut,"<script>window.location.href='./upload.html'</script>");
    return 0;
}
